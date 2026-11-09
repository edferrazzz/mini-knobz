# Mini Knobz (by Audio Makers)

Five one-knob effects with the same window, each with its own colour:

| Plugin | What it does | Options | Extra row |
|---|---|---|---|
| **Knobz Bite** | Saturation, loudness-matched | TAPE, TUBE, DRIVE, CLIP | MIX |
| **Knobz Phat** | Fattener + exciter (3 bands + glue) | WARM, PUNCH, CRUSH, AIR | MIX |
| **Knobz Kickside** | Tempo-synced sidechain pump | 5 curves (CLASSIC, SMOOTH, HARD, LONG, CHOP) | Tempo 1/1 to 1/16 |
| **Knobz Wide** | Stereo widener, side-only (mono sum never changes) | CLEAN, DOUBLE, SPREAD | MONO BASS |
| **Knobz Gasp** | Reverse reverb from a dropped clip, exported as WAV | SMOOTH, STUTTER, TAPE, RISE | Waveform + drag WAV |

JUCE 8, C++20, CMake. One target per plugin (`KNOBZ_PLUGIN` picks it); all share `Source/Common` (processor, editor) and `Source/Dsp` (audio code, `Spec.cpp` = names, colours, help texts).

- Build: `cmake -S . -B build && cmake --build build --config Release`
- Tests: `knobz_tests` checks every plugin x option x knob position (no NaN, peak <= 1.0, loudness match, dry path alignment, Wide mono sum, Gasp length and swell). `KNOBZ_SNAPSHOT_DIR=dir` writes PNGs of the windows (Linux: run under `xvfb-run`), `KNOBZ_DEMO_DIR=dir` writes before/after WAVs.
- CI (`.github/workflows/build.yml`): on push to main builds Windows (MSVC + Inno Setup, `installer/MiniKnobz.iss`, one checkbox per plugin) and macOS (universal VST3 + AU + Gasp app, `.pkg` with one choice per plugin) and commits the installers to `Release/`.
- Update check: the gear reads `https://updates.audiomakers.club/mini-knobz.json` (`product: "mini-knobz"`, `latestVersion`, `downloadUrl`).

Fonts: Space Grotesk and Inter (SIL Open Font License).
