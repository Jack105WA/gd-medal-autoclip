# GD Medal AutoClip

A Windows Geode mod for Geometry Dash 2.2081. It detects new bests and level completions, then presses Medal's configured clip hotkey automatically.

## Features

- Detects new best percentages during normal gameplay.
- Detects completed levels.
- Triggers Medal's regular clip hotkey (F8 by default).
- No Developer Portal, API key, event IDs, or custom integration required.
- Separate toggles for new-best and completion clips.
- Optional Safe Mode and rated-level filters.
- Configurable Medal hotkey and capture delay.
- Builds a Windows x64 `.geode` package through GitHub Actions.

## Requirements

- Geometry Dash 2.2081
- Geode for Geometry Dash 2.2081
- Medal for Windows, running and recording
- Medal's clip hotkey configured and working

## Setup

1. Install and launch Medal.
2. Open Medal Settings and find the hotkey used to save a clip. Medal uses F8 by default; change it in Medal if you prefer another key.
3. Make sure Medal is recording and that its clip hotkey saves a clip when pressed.
4. Install **GD Medal AutoClip** through Geode.
5. Open the mod settings in Geode.
6. Enable **Clip New Bests** and/or **Clip Level Completions**.
7. Configure the filters if desired.
8. Set **Medal Clip Hotkey** to the same key configured in Medal. The default is `F8`.
9. Optionally configure **Capture Delay**.

## Settings

| Setting | Default | Description |
|---|---:|---|
| Clip New Bests | On | Triggers when normal-mode progress exceeds the saved normal record |
| Clip Level Completions | On | Triggers when a level is completed in normal mode |
| Only Clip When Safe Mode Is Off | On | Blocks clip triggers when Geode's `safe-mode` launch flag is enabled |
| Rated Levels Only | Off | Only triggers for levels with a star rating |
| Medal Clip Hotkey | F8 | Key sent to Medal to save the clip |
| Capture Delay | 0 ms | Delay between detecting the event and sending the hotkey |

The rated-level filter checks the level's star count. Unrated levels are skipped when enabled.

**Safe Mode note:** Geode Safe Mode disables third-party mods, so this mod normally will not be running in Safe Mode in the first place. The filter additionally checks Geode's public `safe-mode` launch flag when the mod is active.

Supported hotkey names include `F1`–`F24`, letters, digits, `SPACE`, `TAB`, `ENTER`, `INSERT`, `DELETE`, `HOME`, `END`, `PAGEUP`, and `PAGEDOWN`. An unrecognized value falls back to F8.

## Important limitations

This version uses Medal's normal clip hotkey instead of Medal's Auto Clipping event API. Medal's documented local event endpoint requires a Developer Portal public key and registered event ID, so it cannot be used as a portal-free direct-save endpoint.

Because this mod triggers the ordinary hotkey:

- Clip duration is controlled by Medal's own recording/clip settings, not by this mod.
- Medal chooses the saved clip's title. The mod cannot set a custom title through the hotkey.
- New Best and Level Complete events both invoke the same Medal hotkey.
- Medal must be running, recording, and able to receive the configured global hotkey.
- The game must not be running elevated while Medal is not, or Windows may block simulated input.

Practice mode is ignored.

## Build

Every push to `main`, pull request, or manual workflow run builds a Windows x64 package. Open the repository's **Actions** tab and download the `gd-medal-autoclip-windows` artifact.

To publish a GitHub Release, push a version tag such as `v1.2.0`. The workflow attaches the built `.geode` file to the release.
