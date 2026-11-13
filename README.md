# GD Medal AutoClip

A Windows Geode mod for Geometry Dash 2.2081 that sends gameplay events to Medal's local Auto Clipping API.

## Features

- Detects new best percentages during normal gameplay.
- Detects completed levels.
- Requests Medal clips through `http://localhost:12665/api/v1/event/invoke`.
- Configurable event IDs, clip names, duration, and capture delay.
- Separate toggles for new-best and completion clips.
- Builds a Windows x64 `.geode` package through GitHub Actions.

## Requirements

- Geometry Dash 2.2081
- Geode built for Geometry Dash 2.2081
- Medal running on the same PC
- A Medal Developer Portal public key and registered event IDs

## Medal setup

1. Open the [Medal Developer Portal](https://medal.tv/portal).
2. Create an Auto Clipping integration and register two events: one for new bests and one for level completions.
3. Copy the integration's public key.
4. In Geometry Dash, open Geode, select **GD Medal AutoClip**, and open its settings.
5. Paste the public key and enter the corresponding event IDs.
6. Enable the event types you want to clip.
7. Configure the clip name, duration, and capture delay.

Medal must be running and Auto Clipping must be enabled for the registered events. Medal may accept an event without actually scheduling a clip if the game/event is inactive or disabled.

## Clip naming

The default names are:

- `New Best - {percent}%`
- `Level Complete - {level}`

The placeholders are replaced by the mod before the request is sent.

## Build

Every push to `main`, pull request, or manual workflow run builds a Windows x64 package. Open the repository's **Actions** tab and download the `gd-medal-autoclip-windows` artifact.

To publish a GitHub Release, push a version tag such as `v1.0.0`. The workflow attaches the built `.geode` file to the release.

The project uses the Geode SDK version declared in `mod.json` and targets Geometry Dash 2.2081.

## API

The mod uses Medal's documented local endpoint:

`POST http://localhost:12665/api/v1/event/invoke`

See the [Medal Auto Clipping API reference](https://medal.tv/portal/docs/auto-clipping/api) for request fields and response errors.
