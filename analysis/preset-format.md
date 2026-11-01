# `.gmsynth` Preset Format

Observed from `Data\Patches\Plugin presets\Generators\GMS\**\*.gmsynth`
(e.g. `Basses\Agressive TE.gmsynth`, 4,948 bytes). Layout below is observed;
field meanings marked inferred.

## Summary

Fixed-size little-endian binary file, 4,948 bytes in every sampled preset.

| Offset | Size | Contents |
| --- | --- | --- |
| `0x0000` | 4 | Magic ASCII `3nyS` |
| `0x0004` | 256 | Oscillator 1 wavetable path (absolute, NUL-terminated) |
| `0x0104` | 252 | Padding (slot 1 to 512 bytes) |
| `0x0200` | 4 | Slot 2 header (always `00 00 00 00` in samples) |
| `0x0204` | 256 | Oscillator 2 wavetable path |
| `0x0304` | 252 | Padding |
| `0x0400` | 4 | Slot 3 header (`00 00 00 00`) |
| `0x0404` | 256 | Oscillator 3 wavetable path |
| `0x0504` | 252 | Padding |
| `0x0604` | ~0x07FC | Zero padding |
| `0x0E00` | ... | Float32 parameter state (little-endian) |
| `~0x1150` | <=256 | Preset display name (ASCII, e.g. `Agressive TE`) |
| `0x1354` | — | End of file (4,948 bytes) |

## Evidence

- Magic `33 6E 79 53` = `3nyS` at offset 0.
- Three absolute paths at `0x004`, `0x204`, `0x404`, each beginning 4 bytes
  after a 512-byte slot boundary. Example (`Agressive TE`):

  ```
  0x004  C:\Program Files\Image-Line\FL Studio 11\Plugins\Fruity\Generators\GMS\Synth Waves\Beast.wav
  0x204  C:\...\Synth Waves\Sine.wav
  0x404  C:\...\Synth Waves\Saw.wav
  ```

- Region `0x0E00` onward is dense little-endian `float32` data. Common values:
  `0x3F800000` = 1.0, `0x3F000000` = 0.5, `0x3E999999` ≈ 0.3, plus small
  values like `0x3D75C290` ≈ 0.06. This indicates normalized control storage.
- Diffing two presets shows changes concentrated in the path slots and in
  `0x0E00..0x1160`, consistent with parameters + embedded name.

## Inferred notes

- Paths are absolute and were written by older installs (FL Studio 11). Current
  installs must remap or ignore them; matching is likely by filename.
- Slot headers likely encode a wave-import flag (AIFF/Microsoft Wave import was
  seen in UI strings), but all sampled presets store `00000000`.
- The parameter block is probably a flat array of the plug-in's automatable
  parameters in declaration order. **Order and ranges are not established**
  without disassembly; do not assume index-to-parameter mapping yet.
- `CRijndael` exists in the binary but these presets are not encrypted.
