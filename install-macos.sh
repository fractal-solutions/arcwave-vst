#!/bin/sh
set -e

HERE="$(cd "$(dirname "$0")" && pwd)"

if [ -d "$HERE/dist/ArcWave.vst3" ]; then
    SRC="$HERE/dist/ArcWave.vst3"
else
    SRC="$HERE/ArcWave.vst3"
fi

if [ ! -d "$SRC" ]; then
    echo "ArcWave.vst3 not found next to this script."
    exit 1
fi

if [ "${1:-}" = "--system" ]; then
    DEST="/Library/Audio/Plug-Ins/VST3"
    sudo mkdir -p "$DEST"
    sudo rm -rf "$DEST/ArcWave.vst3"
    sudo cp -R "$SRC" "$DEST/ArcWave.vst3"
    sudo xattr -dr com.apple.quarantine "$DEST/ArcWave.vst3" 2>/dev/null || true
else
    DEST="$HOME/Library/Audio/Plug-Ins/VST3"
    mkdir -p "$DEST"
    rm -rf "$DEST/ArcWave.vst3"
    cp -R "$SRC" "$DEST/ArcWave.vst3"
    xattr -dr com.apple.quarantine "$DEST/ArcWave.vst3" 2>/dev/null || true
fi

echo "Installed ArcWave to $DEST/ArcWave.vst3"
echo "In FL Studio: Options > Manage plugins > Find plugins, then rescan."
