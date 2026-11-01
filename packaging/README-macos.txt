ArcWave - macOS VST3 (universal: Apple Silicon + Intel)

1) Extract this folder.
2) Run ./install-macos.sh (installs to ~/Library/Audio/Plug-Ins/VST3), or copy
   ArcWave.vst3 there yourself.
3) In FL Studio: Options > Manage plugins > Find plugins.

If macOS blocks the plugin, run:
  xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ArcWave.vst3

This build is unsigned. Distributors should sign and notarize it with an Apple
Developer account.

License: GNU AGPLv3. See LICENSE.
Not affiliated with Image-Line. No Image-Line assets are included.
