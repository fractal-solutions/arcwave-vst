#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "SynthVoice.h"
#include "ScopeBuffer.h"

#include <atomic>

// ArcWave: an independent, clean-room wavetable synth built from the
// architecture recovered from GMS. It is not affiliated with Image-Line and
// does not include any Image-Line code or assets.
class ArcWaveProcessor : public juce::AudioProcessor
{
public:
    ArcWaveProcessor();
    ~ArcWaveProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ArcWave"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    ScopeBuffer& getScopeBuffer() noexcept { return scope; }
    float getOutputPeak() const noexcept { return outputPeak.load(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    ScopeBuffer scope;
    std::atomic<float> outputPeak { 0.0f };
    juce::Synthesiser synth;
    juce::Reverb reverb;
    std::vector<float> delayL, delayR;
    int delayWrite = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArcWaveProcessor)
};
