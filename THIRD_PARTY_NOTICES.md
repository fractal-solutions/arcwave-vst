# Third-Party Notices

ArcWave itself is licensed under the GNU Affero General Public License v3.0
(see `LICENSE`).

## JUCE

ArcWave is built with JUCE (https://juce.com), © Raw Material Software Limited.
JUCE is available under the AGPLv3 or a commercial license. This project uses
the AGPLv3 option, so the complete ArcWave source is provided in this repository.

JUCE is not committed to this repository. `scripts/build.ps1` fetches the pinned
JUCE release (9.0.4) into `third_party/JUCE/` at build time. JUCE retains its own
copyright and license terms.

## Fonts

- **Chakra Petch** — SIL Open Font License 1.1, see `assets/fonts/OFL-ChakraPetch.txt`.
- **Share Tech Mono** — SIL Open Font License 1.1, see `assets/fonts/OFL-ShareTechMono.txt`.

## Wavetables

The 29 tables in `assets/wavetables-original/` are original waveforms generated
by ArcWave's own code (`WavetableBank::makeOriginalTables`). They are released
under the same AGPLv3 license as ArcWave.

ArcWave does **not** include or redistribute Image-Line GMS wavetables or any
other Image-Line assets. If GMS is installed, ArcWave reads those files locally
at runtime; they remain the property of Image-Line.

## Trademarks

ArcWave and Qua$$a are the branding of this project. GMS, Groove Machine Synth,
FL Studio, and Image-Line are trademarks of Image-Line. ArcWave is not
affiliated with or endorsed by Image-Line.
