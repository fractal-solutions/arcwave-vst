#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="0.3.0"

"$ROOT/scripts/build-macos.sh"

PKG="$ROOT/release/ArcWave-v$VERSION-macos"
rm -rf "$PKG"
mkdir -p "$PKG"

cp -R "$ROOT/dist/ArcWave.vst3" "$PKG/ArcWave.vst3"
cp "$ROOT/install-macos.sh" "$ROOT/LICENSE" "$PKG/"
cp "$ROOT/packaging/README-macos.txt" "$PKG/README.txt"
cp -R "$ROOT/assets/wavetables-original" "$PKG/"

mkdir -p "$ROOT/release"
ditto -c -k --sequesterRsrc --keepParent "$PKG" "$ROOT/release/ArcWave-v$VERSION-macos.zip"

echo "Created $ROOT/release/ArcWave-v$VERSION-macos.zip"
echo "Upload it to GitHub Releases, or copy it into downloads/ and commit it."
