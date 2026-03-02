# Granny for Move Anything

`granny-grain` is a granular sampler for [Move Anything](https://github.com/charlesvestal/move-anything).

It plays grains from a WAV file you choose in Shadow UI.

## Quick Start

1. Build and install this module.
2. Load `Granny` in a chain.
3. Open `Main` and select `Sample File`.
4. Pick a `.wav` from:
   - `/data/UserData/UserLibrary/Samples`
5. Play notes/pads.

By default, `sample_path` is empty, so Granny is silent until a file is selected.

## UI Layout

Root pages:
- `Main`
- `Scan`
- `Window / Tone`
- `Pitch / Voice`

Main root knobs:
- `Position`, `Size`, `Density`, `Spray`, `Jitter`, `Scan`, `Freeze`, `Quality`

## Parameter Overview

### Main

- `sample_path`: file browser for `.wav` files
- `position`: base point in the file (0..1)
- `size_ms`: grain length in ms
- `density`: grains per second
- `spray`: random position spread around `position`
- `jitter`: random timing offset of grain starts
- `freeze`: hold grain position behavior

### Scan

- `scan_enable`: turns scan motion on/off
- `scan`: scan speed and direction while a note is held
- `scan_end_mode`: behavior at file edges (`wrap`, `pingpong`, `clamp`, `stop`)

### Window / Tone

- `window_type`: grain window shape (`hann`, `triangle`, `blackman`)
- `window_shape`: window curve amount
- `grain_gain`: grain level
- `quality`: `eco`, `normal`, `high`
- `trigger_mode`: `per_voice` or `global_cloud`

### Pitch / Voice

- `pitch_semi`: semitone transpose
- `fine_cents`: fine tune
- `keytrack`: note-to-pitch amount
- `play_mode`: `mono`, `portamento`, `poly`
- `polyphony`: voice count in poly mode
- `portamento_ms`: glide time in portamento mode
- `spread`: stereo spread

## File Parameter (`sample_path`)

`sample_path` is exposed as a `filepath` chain parameter in `module.json`.

Fields used:
- `type`: must be `filepath`
- `root`: start folder for browser
- `filter`: extension filter (for example `.wav`)
- `default`: default path (empty by default in Granny)

Current config:

```json
{
  "key": "sample_path",
  "name": "Sample File",
  "type": "filepath",
  "root": "/data/UserData/UserLibrary/Samples",
  "filter": ".wav",
  "default": ""
}
```

## WAV Support

Supported WAV formats:
- PCM 8/16/24/32-bit
- float32 WAV

Input is converted to mono float internally.

## Build

```bash
./scripts/build.sh
```

Output:
- `dist/granny-grain/`
- `dist/granny-grain-module.tar.gz`

## Install

```bash
./scripts/install.sh
```

Install target:

```text
/data/UserData/move-anything/modules/sound_generators/granny-grain/
```

Restart Move Anything after install.

## Porting Note

For reusable Shadow UI filepath browser integration details, see:
- `docs/filepath-browser-porting.md`
