#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ArcWaveLookAndFeel.h"
#include "Presets.h"
#include "ScopeBuffer.h"

#include <array>
#include <map>
#include <memory>
#include <vector>

class ArcWaveEditor : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit ArcWaveEditor (ArcWaveProcessor&);
    ~ArcWaveEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void showPage (int page) { setPage (page); }

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        int page = 0;
    };

    struct Choice
    {
        juce::ComboBox box;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
        int page = 0;
    };

    struct Toggle
    {
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
        int page = 0;
    };

    struct Panel
    {
        juce::String title;
        juce::Rectangle<int> bounds;
    };

    class ScopeView : public juce::Component
    {
    public:
        explicit ScopeView (ScopeBuffer& bufferToUse) : buffer (bufferToUse) {}
        void paint (juce::Graphics&) override;

    private:
        ScopeBuffer& buffer;
        float samples[ScopeBuffer::kSize] {};
    };

    class WavePreview : public juce::Component
    {
    public:
        WavePreview (juce::AudioProcessorValueTreeState& s, juce::String id)
            : state (s), paramId (std::move (id)) {}

        void paint (juce::Graphics&) override;

    private:
        juce::AudioProcessorValueTreeState& state;
        juce::String paramId;
    };

    class LevelMeter : public juce::Component
    {
    public:
        explicit LevelMeter (ArcWaveProcessor& p) : processor (p) {}
        void paint (juce::Graphics&) override;

    private:
        ArcWaveProcessor& processor;
    };

    void timerCallback() override;

    Knob& addKnob (const juce::String& id, const juce::String& caption, int page);
    void addChoice (const juce::String& id, int page);
    void addToggle (const juce::String& id, const juce::String& text, int page);

    void setPage (int page);
    void updateVisibility();
    void refreshPresetList();
    void showSaveDialog();

    void layoutKnob (Knob&, juce::Rectangle<int>);
    void layoutKnobRow (const juce::StringArray& ids, juce::Rectangle<int>);

    ArcWaveProcessor& processor;
    ArcWaveLookAndFeel lookAndFeel;
    PresetManager presetManager;
    ScopeView scopeView;

    std::map<juce::String, std::unique_ptr<Knob>> knobs;
    std::map<juce::String, std::unique_ptr<Choice>> choices;
    std::map<juce::String, std::unique_ptr<Toggle>> toggles;

    std::array<juce::Label, 3> oscCaptions;
    std::array<std::unique_ptr<WavePreview>, 3> wavePreviews;
    std::unique_ptr<LevelMeter> levelMeter;
    juce::Label aboutLabel;

    juce::TextButton prevButton { "<" }, nextButton { ">" };
    juce::TextButton randomButton { "RANDOMIZE" }, saveButton { "SAVE" };
    juce::TextButton pageSynth { "SYNTH" }, pageTone { "TONE" }, pageFx { "FX" };
    juce::ComboBox presetBox;

    std::vector<Panel> panels;
    int currentPage = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArcWaveEditor)
};
