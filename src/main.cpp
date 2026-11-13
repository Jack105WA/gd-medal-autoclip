#define NOMINMAX
#include <Windows.h>

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <string>
#include <thread>

using namespace geode::prelude;

namespace {
    WORD parseHotkey(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });

        if (value.size() >= 2 && value[0] == 'F') {
            try {
                auto number = std::stoi(value.substr(1));
                if (number >= 1 && number <= 24) {
                    return static_cast<WORD>(0x70 + number - 1);
                }
            } catch (...) {}
        }

        if (value.size() == 1) {
            const char c = value[0];
            if (c >= 'A' && c <= 'Z') return static_cast<WORD>(c);
            if (c >= '0' && c <= '9') return static_cast<WORD>(c);
        }

        if (value == "SPACE") return VK_SPACE;
        if (value == "TAB") return VK_TAB;
        if (value == "ENTER" || value == "RETURN") return VK_RETURN;
        if (value == "PAUSE") return VK_PAUSE;
        if (value == "INSERT") return VK_INSERT;
        if (value == "DELETE") return VK_DELETE;
        if (value == "HOME") return VK_HOME;
        if (value == "END") return VK_END;
        if (value == "PAGEUP") return VK_PRIOR;
        if (value == "PAGEDOWN") return VK_NEXT;

        return VK_F8;
    }

    bool eventFiltersAllowClip(GJGameLevel* level) {
        auto mod = Mod::get();

        // Geode exposes the safe-mode launch flag through its public Loader API.
        if (mod->getSettingValue<bool>("only-when-safe-mode-off") &&
            Loader::get()->getLaunchFlag("safe-mode")) {
            return false;
        }

        if (mod->getSettingValue<bool>("rated-levels-only") &&
            (!level || level->m_stars <= 0)) {
            return false;
        }

        return true;
    }

    void pressMedalHotkey(std::string const& eventName) {
        auto mod = Mod::get();
        auto hotkeyName = mod->getSettingValue<std::string>("clip-hotkey");
        auto delay = static_cast<int>(mod->getSettingValue<int64_t>("capture-delay"));
        const WORD virtualKey = parseHotkey(hotkeyName);
        delay = std::clamp(delay, 0, 60000);

        log::info("Medal AutoClip: '{}' detected. Sending {} hotkey in {} ms.",
            eventName, hotkeyName, delay);

        std::thread([virtualKey, delay, eventName]() {
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }

            INPUT inputs[2]{};
            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wVk = virtualKey;

            inputs[1].type = INPUT_KEYBOARD;
            inputs[1].ki.wVk = virtualKey;
            inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

            const auto sent = SendInput(2, inputs, sizeof(INPUT));
            if (sent != 2) {
                log::error(
                    "Medal AutoClip: Windows could not send the clip hotkey for '{}'. "
                    "Check that Medal is running at the same privilege level.",
                    eventName
                );
                return;
            }

            log::info("Medal AutoClip: sent clip hotkey for '{}'.", eventName);
        }).detach();
    }
}

class $modify(MedalAutoClipPlayLayer, PlayLayer) {
    struct Fields {
        bool newBestSent = false;
        bool completionSent = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        m_fields->newBestSent = false;
        m_fields->completionSent = false;
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->newBestSent = false;
        m_fields->completionSent = false;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_level || m_isPracticeMode || m_fields->newBestSent) {
            return;
        }

        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enable-new-bests") ||
            !eventFiltersAllowClip(m_level)) {
            return;
        }

        const int percent = this->getCurrentPercent();
        if (percent > m_level->m_normalPercent) {
            m_fields->newBestSent = true;
            pressMedalHotkey("New Best - " + std::to_string(percent) + "%");
        }
    }

    void levelComplete() {
        auto mod = Mod::get();

        if (!m_isPracticeMode &&
            !m_fields->completionSent &&
            mod->getSettingValue<bool>("enable-completions") &&
            eventFiltersAllowClip(m_level)) {
            m_fields->completionSent = true;
            auto name = m_level && !m_level->m_levelName.empty()
                ? m_level->m_levelName
                : std::string("Unknown Level");
            pressMedalHotkey("Level Complete - " + name);
        }

        PlayLayer::levelComplete();
    }
};
