#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
float softClip (float x) noexcept
{
    constexpr float t = 0.95f;
    const float a = std::abs (x);
    if (a <= t)
        return x;

    const float y = t + (1.0f - t) * std::tanh ((a - t) / (1.0f - t));
    return x < 0.0f ? -y : y;
}

juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto& bank = WavetableBank::getInstance();
    juce::StringArray waveNames;
    for (auto& n : bank.names())
        waveNames.add (n);
    if (waveNames.isEmpty())
        waveNames.add ("Saw");

    auto findIdx = [&waveNames] (const juce::String& name, int fallback)
    {
        const int idx = waveNames.indexOf (name);
        return idx >= 0 ? idx : juce::jlimit (0, waveNames.size() - 1, fallback);
    };

    const int def1 = findIdx ("Saw", 0);
    const int def2 = findIdx ("Square", 1);
    const int def3 = findIdx ("Sine", 2);

    auto wave = [&layout, &waveNames] (const juce::String& id, const juce::String& name, int def)
    {
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id, 1 }, name, waveNames, def));
    };

    wave ("osc1Wave", "Osc 1 Wave", def1);
    wave ("osc2Wave", "Osc 2 Wave", def2);
    wave ("osc3Wave", "Osc 3 Wave", def3);

    auto floatParam = [&layout] (const juce::String& id, const juce::String& name,
                                 juce::NormalisableRange<float> range, float def)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, def));
    };

    auto intParam = [&layout] (const juce::String& id, const juce::String& name, int mn, int mx, int def)
    {
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id, 1 }, name, mn, mx, def));
    };

    auto boolParam = [&layout] (const juce::String& id, const juce::String& name, bool def)
    {
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id, 1 }, name, def));
    };

    // Oscillators
    intParam ("osc1Pitch", "Osc 1 Pitch (semitones)", -24, 24, 0);
    intParam ("osc2Pitch", "Osc 2 Pitch (semitones)", -24, 24, 0);
    intParam ("osc3Pitch", "Osc 3 Pitch (semitones)", -24, 24, 0);
    floatParam ("osc1Detune", "Osc 1 Detune (cents)", { -100.0f, 100.0f, 1.0f }, 0.0f);
    floatParam ("osc2Detune", "Osc 2 Detune (cents)", { -100.0f, 100.0f, 1.0f }, 0.0f);
    floatParam ("osc3Detune", "Osc 3 Detune (cents)", { -100.0f, 100.0f, 1.0f }, 0.0f);
    floatParam ("osc1Mix", "Osc 1 Mix", { 0.0f, 1.0f, 0.001f }, 1.0f);
    floatParam ("osc2Mix", "Osc 2 Mix", { 0.0f, 1.0f, 0.001f }, 0.5f);
    floatParam ("osc3Mix", "Osc 3 Mix", { 0.0f, 1.0f, 0.001f }, 0.25f);
    boolParam ("osc2Invert", "Osc 2 Invert", false);
    floatParam ("noiseMix", "Noise Mix", { 0.0f, 1.0f, 0.001f }, 0.0f);
    intParam ("octave", "Octave", -3, 3, 0);
    boolParam ("monoVoice", "Mono Voice", false);

    // Unison
    intParam ("unisonVoices", "Unison Voices", 1, 8, 1);
    floatParam ("unisonDetune", "Unison Detune", { 0.0f, 1.0f, 0.001f }, 0.1f);
    floatParam ("unisonStereo", "Unison Stereo", { 0.0f, 1.0f, 0.001f }, 0.5f);

    // Filter
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "filterType", 1 }, "Filter Type",
        juce::StringArray { "Low Pass", "High Pass", "Band Pass" }, 0));
    floatParam ("filterCutoff", "Filter Cutoff (Hz)",
                juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 2400.0f);
    floatParam ("filterResonance", "Filter Resonance", { 0.5f, 4.0f, 0.01f }, 0.707f);
    floatParam ("filterKeytrack", "Filter KBD Tracking", { 0.0f, 1.0f, 0.001f }, 0.0f);
    floatParam ("filterEnvAmount", "Filter Envelope Amount", { -1.0f, 1.0f, 0.001f }, 0.0f);

    // Cross-modulation + pitch envelope
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "modKind", 1 }, "Mod Kind",
        juce::StringArray { "Off", "FM", "Ring", "AM" }, 0));
    floatParam ("modAmount", "Modulation", { 0.0f, 1.0f, 0.001f }, 0.0f);
    floatParam ("pitchEnvAmount", "Pitch Env Amount (semitones)", { -48.0f, 48.0f, 0.1f }, 0.0f);
    floatParam ("pitchEnvDecay", "Pitch Env Decay (s)", { 0.001f, 2.0f, 0.001f }, 0.1f);

    // Envelopes
    floatParam ("ampAttack", "Amp Attack (s)", { 0.001f, 5.0f, 0.001f }, 0.01f);
    floatParam ("ampDecay", "Amp Decay (s)", { 0.001f, 5.0f, 0.001f }, 0.3f);
    floatParam ("ampSustain", "Amp Sustain", { 0.0f, 1.0f, 0.001f }, 0.8f);
    floatParam ("ampRelease", "Amp Release (s)", { 0.001f, 5.0f, 0.001f }, 0.4f);
    floatParam ("filterAttack", "Filter Attack (s)", { 0.001f, 5.0f, 0.001f }, 0.01f);
    floatParam ("filterDecay", "Filter Decay (s)", { 0.001f, 5.0f, 0.001f }, 0.2f);

    // LFO
    floatParam ("lfoRate", "LFO Rate (Hz)", { 0.05f, 20.0f, 0.001f }, 3.0f);
    floatParam ("lfoAmount", "LFO Amount", { 0.0f, 1.0f, 0.001f }, 0.0f);

    // FX rack (subset of the recovered 10-slot chain)
    floatParam ("fxDrive", "FX Drive", { 0.0f, 1.0f, 0.001f }, 0.0f);
    floatParam ("fxCrusher", "FX Crusher", { 0.0f, 1.0f, 0.001f }, 0.0f);
    floatParam ("fxDelayMix", "FX Delay Mix", { 0.0f, 1.0f, 0.001f }, 0.2f);
    floatParam ("fxDelayTime", "FX Delay Time (s)", { 0.02f, 1.0f, 0.001f }, 0.25f);
    floatParam ("fxDelayFeedback", "FX Delay Feedback", { 0.0f, 0.9f, 0.001f }, 0.3f);
    floatParam ("fxReverbMix", "FX Reverb Mix", { 0.0f, 1.0f, 0.001f }, 0.15f);
    floatParam ("fxReverbSize", "FX Reverb Size", { 0.0f, 1.0f, 0.001f }, 0.5f);
    floatParam ("masterGain", "Master Gain (dB)", { -36.0f, 6.0f, 0.1f }, -12.0f);

    return layout;
}
}

ArcWaveProcessor::ArcWaveProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", makeLayout())
{
    for (int i = 0; i < 16; ++i)
        synth.addVoice (new ArcWaveVoice (apvts));

    synth.addSound (new ArcWaveSound());
}

void ArcWaveProcessor::reset()
{
    synth.allNotesOff (0, false);
    outputPeak.store (0.0f);
}

void ArcWaveProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    reverb.setSampleRate (sampleRate);
    reverb.reset();

    const int maxDelay = (int) (sampleRate * 2.0) + 1;
    delayL.assign ((size_t) maxDelay, 0.0f);
    delayR.assign ((size_t) maxDelay, 0.0f);
    delayWrite = 0;
}

bool ArcWaveProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ArcWaveProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2)
        return;

    auto get = [this] (const char* id, float fallback)
    {
        auto* p = apvts.getRawParameterValue (id);
        return p != nullptr ? p->load() : fallback;
    };

    const float drive = get ("fxDrive", 0.0f);
    const float crush = get ("fxCrusher", 0.0f);
    const float delayMix = get ("fxDelayMix", 0.2f);
    const float delayTime = get ("fxDelayTime", 0.25f);
    const float delayFb = get ("fxDelayFeedback", 0.3f);
    const float reverbMix = get ("fxReverbMix", 0.15f);
    const float reverbSize = get ("fxReverbSize", 0.5f);
    const float master = juce::Decibels::decibelsToGain (get ("masterGain", 0.0f));

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);
    const int n = buffer.getNumSamples();

    const int delaySize = (int) delayL.size();
    const int delaySamples = (delaySize > 1)
        ? juce::jlimit (1, delaySize - 1, (int) (delayTime * (float) getSampleRate()))
        : 0;

    const float driveGain = 1.0f + drive * 3.0f;
    const float crushLevels = 2.0f + (1.0f - crush) * 254.0f;

    for (int i = 0; i < n; ++i)
    {
        float l = left[i];
        float r = right[i];

        if (drive > 0.0001f)
        {
            l = std::tanh (l * driveGain);
            r = std::tanh (r * driveGain);
        }

        if (crush > 0.0001f)
        {
            l = std::round (l * crushLevels) / crushLevels;
            r = std::round (r * crushLevels) / crushLevels;
        }

        if (delayMix > 0.0001f && delaySamples > 0)
        {
            int readIdx = delayWrite - delaySamples;
            if (readIdx < 0)
                readIdx += delaySize;

            const float dl = delayL[(size_t) readIdx];
            const float dr = delayR[(size_t) readIdx];
            delayL[(size_t) delayWrite] = l + dl * delayFb;
            delayR[(size_t) delayWrite] = r + dr * delayFb;
            l = l * (1.0f - delayMix) + dl * delayMix;
            r = r * (1.0f - delayMix) + dr * delayMix;
            delayWrite = (delayWrite + 1) % delaySize;
        }

        left[i] = l;
        right[i] = r;
    }

    juce::Reverb::Parameters rp;
    rp.roomSize = reverbSize;
    rp.damping = 0.5f;
    rp.wetLevel = reverbMix;
    rp.dryLevel = 1.0f - reverbMix;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    reverb.processStereo (left, right, n);

    buffer.applyGain (master);

    float blockPeak = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        left[i] = softClip (left[i]);
        right[i] = softClip (right[i]);
        blockPeak = juce::jmax (blockPeak, std::abs (left[i]), std::abs (right[i]));
    }

    outputPeak.store (juce::jmax (blockPeak, outputPeak.load() * 0.92f));

    const int hop = 8;
    const auto* scopeSamples = buffer.getReadPointer (0);
    for (int i = 0; i < n; i += hop)
        scope.pushSample (scopeSamples[i]);
}

juce::AudioProcessorEditor* ArcWaveProcessor::createEditor()
{
    return new ArcWaveEditor (*this);
}

void ArcWaveProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void ArcWaveProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ArcWaveProcessor();
}
