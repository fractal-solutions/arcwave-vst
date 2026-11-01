#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <utility>
#include <vector>

// Loads single-cycle wavetables from a user-supplied directory at runtime.
// Nothing is bundled: point ARCWAVE_WAVETABLES at the tables you want, or let
// it default to the locally installed GMS "Synth Waves" folder.
class WavetableBank
{
public:
    static WavetableBank& getInstance();

    const std::vector<juce::String>& names() const noexcept { return tableNames; }
    const std::vector<float>& table (int index) const noexcept;
    int size() const noexcept { return (int) tables.size(); }

    // Original, license-free single-cycle tables used when no GMS install is
    // present. Also exposed so the repo can ship a visible wavetable pack.
    static std::vector<std::pair<juce::String, std::vector<float>>> makeOriginalTables();

    static constexpr int kTableSize = 2048;

private:
    WavetableBank();
    void load();
    void generateFallback();
    void addTable (const juce::String& name, std::vector<float> data);

    std::vector<std::vector<float>> tables;
    std::vector<juce::String> tableNames;
};
