# ArcWave by Qua$$a

**Install (Windows, no Git needed):** download the latest `ArcWave-v0.3.0-win64.zip`
from the [Releases page](../../releases/latest), extract it, and double-click
**`install-vst3.bat`**. That copies `ArcWave.vst3` to
`C:\Program Files\Common Files\VST3`. Prefer no admin prompt? Skip the script and
add the extracted folder to FL Studio's VST3 search paths instead. Then open FL
Studio and use **Options → Manage plugins → Find plugins** to rescan. ArcWave
works standalone with its own wavetables; if FL Studio's GMS wavetables are
installed, it uses those automatically.

ArcWave is a free (AGPLv3) 64-bit Windows VST3 wavetable synthesizer: three
wavetable oscillators, unison, FM/Ring/AM cross-modulation, pitch envelope,
multimode filter, envelopes, LFO, and an FX chain, wrapped in a compact custom
UI with 68 categorized factory presets.

![ArcWave UI](docs/arcwave-ui.png)

## Features

- **3 wavetable oscillators** with per-osc wave, pitch, detune and mix
- **Unison** up to 8 voices with detune, stereo spread and phase
- **FM / Ring / AM** cross-modulation (OSC 1 modulates OSC 2 and 3)
- **Pitch envelope** for kicks, zaps and percussion
- **TPT state-variable filter** (low/high/band-pass) with cutoff, resonance,
  key tracking and filter envelope
- **Amp ADSR**, filter envelope, LFO
- **FX chain**: drive, crusher, stereo delay, reverb, master gain
- **Soft-clip safety** and a -12 dB default master for headroom
- **Custom UI**: embedded Chakra Petch / Share Tech Mono, holographic dials,
  live scope, per-oscillator waveform previews, output meter
- **Compact 960×600** default window, resizable down to 860×520

## Presets

The preset browser is grouped by category and shows `CATEGORY // NAME`:

- **Init**
- **Bass** — Velvet Sub, Reese Reactor, Acid Filament, Wobble Engine, Growl Bass, Sub Punch, Pluck Bass
- **Leads** — Prism, Neon Wire, Vocal Beacon, Glass Anthem, Screamer, Pure Whistle, Trance, Sync
- **Saws** — Supersaw Anthem, Detuned Saw, Retro Saw, Saw Stack, Saw Pluck, Wide Saw
- **Pads - Ambient** — Aurora, Deep Space, Celestial Choir, Ambient Drift, Frozen Horizon
- **Pads - Bloom** — Bloom Cathedral, Bloom Rising, Bloom Glass, Bloom Choir, Bloom Sub
- **Pads - Textured** — Textured Ice, Grain, Vocal, Metal, Dust
- **Percussion** — Kick 909, Kick Sub, Snare Noise, Hat Closed/Open, Tom Low, Clave, Metallic Hit, Laser Zap
- **Electronic** — EDM Stab, Rave Chord, Techno Pluck, Future Bass, Digital Arp, Trance Pluck
- **FM** — Bell, Bass, Lead, Keys, Growl, Drone
- **Keys** — Velvet Rhodes, Neon Organ, Digital EP, Glass Keys
- **FX** — Riser Engine, Impact Bloom, Downlifter, Noise Sweep, Alien Transmission

## Wavetables

ArcWave loads single-cycle WAV tables in this order:

1. The folder in the `ARCWAVE_WAVETABLES` environment variable, if set.
2. The newest installed FL Studio's GMS `Synth Waves` folder, if found.
3. Otherwise it generates its own original 29-table set at runtime, so the
   plugin still works on machines without FL Studio.

The generated set is also shipped in `assets/wavetables-original/` for reference.
No Image-Line assets are included or redistributed. If you own GMS, ArcWave reads
your local copy only.

## Requirements

- Windows 10/11 x64
- A VST3 host (tested with FL Studio)
- Optional: an FL Studio install with GMS for the GMS wavetables

## Install without Git

1. Open the [Releases page](../../releases/latest) and download
   `ArcWave-v0.3.0-win64.zip`.
2. Extract the ZIP anywhere.
3. Run `install-vst3.bat` (asks for admin), **or** add the extracted folder to
   your host's VST3 search paths (no admin needed).
4. In FL Studio: **Options → Manage plugins → Find plugins**.

## Install from the repo

The built plugin is committed at `dist/ArcWave.vst3`. Clone the repo and either
run `install-vst3.bat` from the repo root, or copy `dist\ArcWave.vst3` to
`C:\Program Files\Common Files\VST3\` manually. You can also add `dist` to your
host's VST3 search paths.

## Build from source

Prerequisites: CMake ≥ 3.22, Visual Studio 2022 Build Tools (MSVC), and Git.

```powershell
git clone https://github.com/<owner>/arcwave.git
cd arcwave
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
```

`scripts/build.ps1` fetches JUCE 9.0.4 into `third_party/JUCE/` if missing, builds
the VST3, runs the headless render test, and stages the bundle in `dist/`.

Manual build:

```powershell
git clone --depth 1 --branch 9.0.4 https://github.com/juce-framework/JUCE.git third_party/JUCE
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target ArcWave_VST3 --parallel
```

JUCE is fetched at build time and is not committed to this repository.

## Reverse-engineering background

ArcWave grew out of a clean-room study of the GMS generator that ships with FL
Studio. The static analysis, recovered parameter vocabulary, wavetable inventory
and `.gmsynth` format notes are in [`analysis/REPORT.md`](analysis/REPORT.md).
ArcWave is an independent implementation: it is not a copy of GMS, does not
include Image-Line code or assets, and is not affiliated with Image-Line.

## License

ArcWave is licensed under the **GNU Affero General Public License v3.0**
(`LICENSE`). Because it links JUCE under JUCE's AGPLv3 option, the source is
provided under AGPLv3. Third-party notices are in
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

Fonts: Chakra Petch and Share Tech Mono, SIL Open Font License 1.1.

"GMS", "Groove Machine Synth", "FL Studio" and "Image-Line" are trademarks of
Image-Line. ArcWave is not affiliated with or endorsed by Image-Line.
