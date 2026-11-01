#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
JUCE_DIR="$ROOT/third_party/JUCE"
BUILD="$ROOT/build-macos"
JUCE_TAG="9.0.4"

if [ ! -f "$JUCE_DIR/CMakeLists.txt" ]; then
    echo "Fetching JUCE $JUCE_TAG..."
    mkdir -p "$(dirname "$JUCE_DIR")"
    git clone --depth 1 --branch "$JUCE_TAG" https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
fi

cmake -S "$ROOT" -B "$BUILD" -G Xcode \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0

cmake --build "$BUILD" --config Release --target ArcWave_VST3 --parallel
cmake --build "$BUILD" --config Release --target ArcWaveRenderTest --parallel

TEST_BIN="$(find "$BUILD" -name ArcWaveRenderTest -type f | head -n 1)"
if [ -n "$TEST_BIN" ]; then
    "$TEST_BIN"
fi

VST3="$(find "$BUILD" -type d -name ArcWave.vst3 | head -n 1)"
if [ -z "$VST3" ]; then
    echo "ArcWave.vst3 bundle not found."
    exit 1
fi

mkdir -p "$ROOT/dist"
rm -rf "$ROOT/dist/ArcWave.vst3"
cp -R "$VST3" "$ROOT/dist/ArcWave.vst3"
echo "Staged $ROOT/dist/ArcWave.vst3"
