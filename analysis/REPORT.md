# GMS (Groove Machine Synth) `GMS_x64.dll` — Reverse-Engineering Report

Static, non-executing analysis performed with **REA 6.7.0** plus direct
inspection of bundled assets and presets. No native decompiler provider
(Hopper/Ghidra/IDA/Kuna) is installed on this host, so this report contains
**no pseudocode or disassembly**; every claim below is tagged observed,
inferred, or unknown.

## 1. Target identity

| Field | Value | Source |
| --- | --- | --- |
| File | `C:\Program Files\Image-Line\FL Studio 2025\Plugins\Fruity\Generators\GMS\GMS_x64.dll` | filesystem |
| SHA-256 | `a4ab525a1a425e396b69f82c83d1d491c1a8338c8de6a7bae8616f5ab33f2e1f` | REA `inspect_artifact` |
| Size | 1,552,384 bytes | REA |
| Format / arch | PE32+ (PE32-plus), x64 / amd64 | REA |
| Imphash (pefile) | `2bc0e462ad86f739ee764a021dba2859` | REA `inspect_pe_imports` |
| Version resource | `GMS 1.1.0 Rev2594  (x64)` | REA `inspect_pe_resources`, RT_VERSION |
| PDB path (leaked) | `c:\build\output\_temp\CompiledUnits-release\GMS\FL Release x64\GMS_x64.dll.pdb` | strings |
| Vendor | Image-Line (`www.image-line.com`, `LegalCopyright Image-Line`) | strings / version |

REA evidence for this report:

| Operation | Evidence ID |
| --- | --- |
| `inspect_artifact` | `ev_a56abe4e5ef927a64285585fb42b1123bbb7a3d2fed07aae56ee5218c0adbdd9` |
| `inspect_native_exports` | `ev_d244b542382b74aa2d966d439bcf9af0ee0862f142b534d6b5faad4b66bb82e4` |
| `inspect_pe_imports` | `ev_40b43b6bba6f6a165fadb046f1a0c4ac75d747e8d2929725b6ca1a59435fc83c` |
| `inspect_pe_resources` | `ev_ce9ed20004a705d6d35b88db0e00e0edee6fadeccfa5b27af336699e1a4ac903` |

Raw evidence is saved as `analysis/artifact.json`, `exports.json`,
`pe-imports.json`, and `resources.json`.

## 2. Plugin ABI (observed)

- Export table contains exactly **one** export: `CreatePlugInstance`
  (ordinal 1, RVA/VA `0x1800c76a0`). This is Image-Line's native plug-in entry
  point, not a VST3/CLAP entry point.
- Import table: **7 DLLs / 147 functions**, all Windows system libraries:
  `KERNEL32.dll` (95), `USER32.dll` (31), `GDI32.dll` (11), `COMDLG32.dll` (2),
  `ADVAPI32.dll` (3), `SHELL32.dll` (2), `ole32.dll` (3).
- **No** audio/DSP library and **no** VST SDK symbols are imported. The DSP is
  self-contained; the host drives it through `CreatePlugInstance`.
- Registry strings: `SOFTWARE\Image-Line\Shared\General`,
  `Software\Image-Line\Registrations\`, `SOFTWARE\Image-Line\`.
- Dynamically referenced decoders (strings, not imports): `oggio_x64.dll`
  (Ogg Vorbis) and `wavpackdll_x64.dll` (WavPack), used for wavetable import.

**Inferred:** GMS is a "Fruity" native generator built on Image-Line's private
`TFruityPlug` framework and made VST/AU-compatible by Image-Line's wrapper,
not a from-scratch VST3.

## 3. Recovered class model (observed RTTI)

MSVC RTTI type descriptors (`.?AV...@@`) expose these classes:

**Plugin core**
- `GMSInstance`, `GMSynth`, `GMSynthVoice`, `GMSynthEditor`
- `CVoiceGenerator`, `CSpectrumChunk`, `CAudioBuffer`, `CWaveSample`,
  `CEventBuffer`, `CWindowPort`

**Image-Line framework**
- `TFruityPlug`, `TCPPFruityPlug`, `CPluginWrapper`, `CPluginInstance`,
  `CPluginEditor`, `CPluginRenderer`, `CFLWrapper`, `CVSTWrapper` (implied)

**UI widgets**
- `CControl`, `CControlButton`, `CControlDisplay`, `CControlKnob`,
  `CControlNumber`, `CControlStateSelector`, `CControlGroup`, `CControlList`,
  `CControlMenu`, `CControlSlider`, `CFileSelector`, `CFileList`, `CImage`,
  `CGraphicBuffer`, `CPresetList`, `CPresetListEditor`, `CPresetListMsgBox`,
  `CPresetListAboutBox`

**Utility**
- `CRijndael` (AES implementation; not observed applied to `.gmsynth` presets,
  which are plaintext)

**Inferred object graph:** `GMSInstance` wraps one `GMSynth` and one
`GMSynthEditor`; `GMSynth` owns N `GMSynthVoice` voices, each driven by a
`CVoiceGenerator`. UI uses the `CControl*` family over `CPluginEditor`.

## 4. Signal architecture (inferred from class names + parameter vocabulary)

Three wavetable oscillators (Wave 1/2/3) with per-oscillator pitch and detune,
mixable into a unison/noise stage, then a state-variable/ multimode filter,
modulated by two envelopes, two LFOs, and an amp EG, followed by a 10-slot
serial/parallel FX rack with a global "glue" stage. Specifics:

- **Oscillators:** 3 × wavetable, per-osc Pitch + Detune; Osc 2 has Invert,
  Phase Offset, Phase Retrig; Osc 1/2/3 mix levels; separate Noise Mix.
- **Voicing:** mono/poly switch, unison voices + detune + stereo + phase +
  phase-retrig, octave shift.
- **Modulation:** `Mod Kind` + `Modulation` (cross-mod between oscillators —
  exact modulation type not established; AM/FM/RM/PM are candidates).
- **Filter:** type switch (`Low Pass`, `Hi Pass` observed; others unknown),
  Cutoff, Resonance, Keyboard Tracking, envelope amount/invert/attack/decay.
- **Envelopes:** Envelope 1 and 2 (`EG1/EG2`: Attack, Decay, Amount) plus a
  dedicated Amplitude EG (`AMP Attack`, `AMP Decay`).
- **LFOs:** LFO 1 and 2 (Rate, Amount, Shape, Sync, Retrig, Invert, Destination).
- **FX rack:** slots `FX 1`..`FX 10`, each with `FX Activate`; effect types
  observed: Distortion, Crusher/Crush, Flanger, Phaser, Reverb, Panner, Delay,
  Filter, Trancer. Per-effect controls observed: Input, Time, Frequency,
  Cutoff, Feedback, Depth, Decay, Release, Bandwidth, LowCut, Predelay,
  HighDamping, MuteDry, MuteInput, MuteReverb, MuteER, Panning.
- **FX modulation:** `FX Glue`, `FX LFO Rate/Amount/Shape/Sync`, switch
  variants, and `FX Param X` / `FX Param Y` (macro targets).
- **Channel section:** `CHN:`-prefixed controls (Mix Level, Note Length,
  Panning, Pitch, Amp Attack/Decay/Sustain/Release/Level, Slide Enable/Time,
  Shuffle Enable). These are FL wrapper/channel parameters, not core DSP.

> Caveat: exact numeric ranges, filter topology, envelope curves, and the
> sense of `Mod Kind` are **unknown** without decompilation. Many preset values
> are normalized floats in `[0,1]`, suggesting normalized control storage.

## 5. Wavetables (observed)

`Plugins\Fruity\Generators\GMS\Synth Waves\` contains **29** single-cycle WAV
tables. Most are 1,024 samples at 44.1 kHz; formats vary (16-bit PCM or 32-bit
float; mono, with two stereo exceptions; one 672-sample table). Full catalogue:
`analysis/wavetables.json`; locally extracted float32 copies: `assets/wavetables/`.

The loader demonstrably accepts 16/32-bit, mono/stereo, and non-1024 lengths,
so downstream oscillators should resample/interpolate arbitrary tables.

## 6. Preset format (observed + inferred)

See `analysis/preset-format.md`. Summary: `.gmsynth` files are a fixed 4,948
bytes: magic `3nyS`, three 512-byte wavetable slots (4-byte field + 256-byte
absolute path), zero padding, then a little-endian float32 state block from
`0x0E00` with the preset name embedded around `0x1150`.

## 7. Registration / DRM

Strings mention demo/registration states and `CRijndael`, but `.gmsynth`
presets are plaintext and readable. **No DRM analysis or bypass was performed
and none should be.** The reimplementation must not include or defeat licensing.

## 8. Observed vs inferred vs unknown

| Claim | Status |
| --- | --- |
| Export `CreatePlugInstance`, version 1.1.0 Rev2594 x64 | observed |
| 147 imports across 7 Win32 DLLs; no VST/audio libs | observed |
| Class names and UI framework | observed (RTTI strings) |
| Parameter/control labels and FX type names | observed (strings) |
| Wavetable file inventory and formats | observed |
| `.gmsynth` container layout | observed layout, inferred field meanings |
| Object graph and DSP routing | inferred |
| Exact parameter ranges, curves, filter topology, mod math | unknown |

## 9. Clean-room reimplementation plan

1. Keep an independent codebase and a new product name (no Image-Line marks).
2. Load the user's local wavetables at runtime (do not bundle them).
3. Implement: 3 wavetable oscillators → unison/noise → multimode filter →
   amp/filter envelopes + 2 LFOs → 10-slot FX rack → glue.
4. Match the recovered **parameter vocabulary** so presets/automation map
   cleanly, using observed labels and normalized [0,1] storage.
5. Verify against the original by rendering identical note/automation cases and
   comparing spectra; treat differences as unknowns to resolve, not guesses.

See `../src/` for the scaffold and `../README.md` for build status.
