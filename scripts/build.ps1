param(
    [string]$JuceTag = "9.0.4",
    [switch]$SkipTest
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$juceDir = Join-Path $root "third_party\JUCE"
$buildDir = Join-Path $root "build"

$cmake = "C:\Program Files\CMake\bin\cmake.exe"
if (-not (Test-Path -LiteralPath $cmake)) {
    $cmake = "cmake"
}

if (-not (Test-Path -LiteralPath (Join-Path $juceDir "CMakeLists.txt"))) {
    Write-Host "Fetching JUCE $JuceTag..."
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $juceDir) | Out-Null
    git clone --depth 1 --branch $JuceTag https://github.com/juce-framework/JUCE.git $juceDir
}

Write-Host "Configuring..."
& $cmake -S $root -B $buildDir -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

Write-Host "Building ArcWave VST3..."
& $cmake --build $buildDir --config Release --target ArcWave_VST3 --parallel

if (-not $SkipTest) {
    Write-Host "Building and running the render test..."
    & $cmake --build $buildDir --config Release --target ArcWaveRenderTest --parallel
    & (Join-Path $buildDir "ArcWaveRenderTest_artefacts\Release\ArcWaveRenderTest.exe")
}

$src = Join-Path $buildDir "ArcWave_artefacts\Release\VST3\ArcWave.vst3"
$dst = Join-Path $root "dist\ArcWave.vst3"

if (Test-Path -LiteralPath $src) {
    if (Test-Path -LiteralPath $dst) {
        Remove-Item -LiteralPath $dst -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dst) | Out-Null
    Copy-Item -LiteralPath $src -Destination $dst -Recurse -Force
    Write-Host "Staged VST3 at $dst"
}

Write-Host "Done."
