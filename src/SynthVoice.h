#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "WavetableBank.h"

class ArcWaveSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class ArcWaveVoice : public juce::SynthesiserVoice
{
public:
    explicit ArcWaveVoice (juce::AudioProcessorValueTreeState& stateToUse);

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void setCurrentPlaybackSampleRate (double newRate) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    struct SVF
    {
        float ic1 = 0.0f, ic2 = 0.0f;
        void reset() noexcept { ic1 = ic2 = 0.0f; }
        float process (float x, float cutoff, float q, double sr, int mode) noexcept;
    };

    float readTable (const std::vector<float>& t, float phase) const noexcept;
    float nextNoise() noexcept;

    juce::AudioProcessorValueTreeState& state;
    juce::ADSR ampEnv;
    juce::ADSR filtEnv;
    float phases[3][8] {};
    SVF filtL, filtR;
    int note = 60;
    float velocity = 0.0f;
    float lfoPhase = 0.0f;
    float pitchEnvLevel = 0.0f;
    juce::uint32 rng = 0x9E3779B9u;
};
