#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/web.hpp>

#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace {
    constexpr auto MEDAL_EVENT_URL = "http://localhost:12665/api/v1/event/invoke";

    std::string replaceToken(std::string value, std::string const& token, std::string const& replacement) {
        size_t position = 0;
        while ((position = value.find(token, position)) != std::string::npos) {
            value.replace(position, token.size(), replacement);
            position += replacement.size();
        }
        return value;
    }

    std::string getClipName(std::string value, std::string const& token, std::string const& replacement) {
        return replaceToken(std::move(value), token, replacement);
    }

    void sendMedalEvent(std::string const& eventID, std::string const& eventName) {
        auto mod = Mod::get();
        auto publicKey = mod->getSettingValue<std::string>("public-key");

        if (publicKey.empty() || eventID.empty()) {
            log::warn("Medal AutoClip: configure a Medal public key and event ID in the mod settings.");
            return;
        }

        auto duration = static_cast<int>(mod->getSettingValue<int64_t>("clip-duration"));
        auto delay = static_cast<int>(mod->getSettingValue<int64_t>("capture-delay"));
        duration = std::clamp(duration, 5, 120);
        delay = std::clamp(delay, 0, 60000);

        matjson::Value payload = matjson::Value::object();
        payload["eventId"] = eventID;
        payload["eventName"] = eventName;
        payload["triggerActions"] = matjson::Value::array();
        payload["triggerActions"].push("SaveClip");
        payload["clipOptions"] = matjson::Value::object();
        payload["clipOptions"]["duration"] = duration;
        payload["clipOptions"]["captureDelayMs"] = delay;

        log::info("Medal AutoClip: requesting '{}' (event {}).", eventName, eventID);

        auto request = web::WebRequest()
            .header("Content-Type", "application/json; charset=utf-8")
            .header("publicKey", publicKey)
            .bodyJSON(payload)
            .timeout(asp::Duration::fromSecs(8))
            .post(MEDAL_EVENT_URL);

        async::spawn(std::move(request), [eventName](web::WebResponse response) {
            auto body = response.string();
            if (!response.ok()) {
                log::error("Medal AutoClip: request failed with HTTP {}: {}", response.code(), body ? body.unwrap() : response.errorMessage());
                return;
            }

            auto json = response.json();
            if (!json) {
                log::warn("Medal AutoClip: Medal returned an unreadable response (HTTP {}).", response.code());
                return;
            }

            auto errorCode = json.unwrap().get("errorCode");
            if (errorCode && errorCode.unwrap().isString()) {
                log::warn("Medal AutoClip: Medal rejected '{}': {}", eventName, errorCode.unwrap().asString().unwrapOr("unknown error"));
                return;
            }

            log::info("Medal AutoClip: Medal accepted event '{}'.", eventName);
        });
    }

    std::string levelName(PlayLayer* layer) {
        if (!layer || !layer->m_level) {
            return "Unknown Level";
        }
        auto name = layer->m_level->m_levelName;
        if (name.empty()) {
            return "Unknown Level";
        }
        return name;
    }
}

class $modify(MedalAutoClipPlayLayer, PlayLayer) {
    struct Fields {
        int highestPercent = 0;
        bool newBestSent = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        m_fields->highestPercent = 0;
        m_fields->newBestSent = false;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_level || m_fields->newBestSent) {
            return;
        }

        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enable-new-bests")) {
            return;
        }

        int percent = this->getCurrentPercent();
        m_fields->highestPercent = std::max(m_fields->highestPercent, percent);

        // m_normalPercent is the saved normal-mode record. Trigger once when
        // the current attempt passes that record.
        if (percent > m_level->m_normalPercent) {
            m_fields->newBestSent = true;

            auto name = getClipName(
                mod->getSettingValue<std::string>("new-best-name"),
                "{percent}",
                std::to_string(percent)
            );

            auto eventID = mod->getSettingValue<std::string>("new-best-event-id");
            sendMedalEvent(eventID, name);
        }
    }

    void levelComplete() {
        auto mod = Mod::get();

        if (mod->getSettingValue<bool>("enable-completions")) {
            auto name = getClipName(
                mod->getSettingValue<std::string>("completion-name"),
                "{level}",
                levelName(this)
            );

            auto eventID = mod->getSettingValue<std::string>("completion-event-id");
            sendMedalEvent(eventID, name);
        }

        PlayLayer::levelComplete();
    }
};
