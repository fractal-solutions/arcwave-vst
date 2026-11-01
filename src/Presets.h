#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

struct FactoryPreset
{
    juce::String name;
    juce::String category;
    std::vector<std::pair<juce::String, float>> values;
};

std::vector<FactoryPreset> makeFactoryPresets();

class PresetManager
{
public:
    explicit PresetManager (juce::AudioProcessorValueTreeState& apvtsToUse);

    juce::StringArray getNames() const;
    juce::String getCategory (int index) const;
    int getNumPresets() const;
    int getCurrentIndex() const noexcept { return currentIndex; }
    juce::String getCurrentName() const;

    void apply (int index);
    void next();
    void previous();
    bool saveCurrent (const juce::String& name);
    void randomize();
    void rescanUserPresets();

private:
    void resetToDefaults();
    void setRaw (const juce::String& id, float value);
    void applyValues (const std::vector<std::pair<juce::String, float>>& values);

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<FactoryPreset> factory;
    juce::Array<juce::File> userFiles;
    juce::File userDir;
    int currentIndex = 0;
};
