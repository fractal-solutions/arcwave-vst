#include "SynthVoice.h"

#include <cmath>

ArcWaveVoice::ArcWaveVoice (juce::AudioProcessorValueTreeState& stateToUse)
    : state (stateToUse)
{
}

bool ArcWaveVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<ArcWaveSound*> (sound) != nullptr;
}

void ArcWaveVoice::setCurrentPlaybackSampleRate (double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
    ampEnv.setSampleRate (newRate);
    filtEnv.setSampleRate (newRate);
}

float ArcWaveVoice::readTable (const std::vector<float>& t, float phase) const noexcept
{
    const int n = (int) t.size();
    if (n == 0)
        return 0.0f;

    const float x = phase * (float) n;
    int i0 = (int) std::floor (x);
    const float frac = x - (float) i0;
    i0 %= n;
    if (i0 < 0)
        i0 += n;
    const int i1 = (i0 + 1) % n;
    return t[(size_t) i0] * (1.0f - frac) + t[(size_t) i1] * frac;
}

float ArcWaveVoice::nextNoise() noexcept
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return (float) (rng & 0x00FFFFFFu) / (float) 0x00800000u - 1.0f;
}

float ArcWaveVoice::SVF::process (float x, float cutoff, float q, double sr, int mode) noexcept
{
    const float fc = juce::jlimit (20.0f, (float) sr * 0.45f, cutoff);
    const float g = std::tan (juce::MathConstants<float>::pi * fc / (float) sr);
    const float k = 1.0f / juce::jmax (0.05f, q);
    const float a1 = 1.0f / (1.0f + g * (g + k));
    const float a2 = g * a1;
    const float a3 = g * a2;
    const float v3 = x - ic2;
    const float v1 = a1 * ic1 + a2 * v3;
    const float v2 = ic2 + a2 * ic1 + a3 * v3;
    ic1 = 2.0f * v1 - ic1;
    ic2 = 2.0f * v2 - ic2;

    if (mode == 1)
        return x - k * v1 - v2; // high-pass
    if (mode == 2)
        return v1;              // band-pass
    return v2;                  // low-pass
}

void ArcWaveVoice::startNote (int midiNoteNumber, float vel, juce::SynthesiserSound*, int)
{
    note = midiNoteNumber;
    velocity = vel;
    lfoPhase = 0.0f;
    pitchEnvLevel = 1.0f;

    for (int o = 0; o < 3; ++o)
        for (int v = 0; v < 8; ++v)
            phases[o][v] = 0.0f;

    filtL.reset();
    filtR.reset();

    auto get = [this] (const char* id, float fallback)
    {
        auto* p = state.getRawParameterValue (id);
        return p != nullptr ? p->load() : fallback;
    };

    ampEnv.setParameters ({ get ("ampAttack", 0.01f), get ("ampDecay", 0.3f),
                            get ("ampSustain", 0.8f), get ("ampRelease", 0.4f) });
    filtEnv.setParameters ({ get ("filterAttack", 0.01f), get ("filterDecay", 0.2f), 0.0f, 0.1f });

    ampEnv.noteOn();
    filtEnv.noteOn();
}

void ArcWaveVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnv.noteOff();
        filtEnv.noteOff();
    }
    else
    {
        ampEnv.reset();
        filtEnv.reset();
        clearCurrentNote();
    }
}

void ArcWaveVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    auto get = [this] (const char* id, float fallback)
    {
        auto* p = state.getRawParameterValue (id);
        return p != nullptr ? p->load() : fallback;
    };

    const auto& bank = WavetableBank::getInstance();

    const std::vector<float>* tables[3] =
    {
        &bank.table ((int) get ("osc1Wave", 0.0f)),
        &bank.table ((int) get ("osc2Wave", 0.0f)),
        &bank.table ((int) get ("osc3Wave", 0.0f))
    };
    const float mix[3]    = { get ("osc1Mix", 1.0f), get ("osc2Mix", 0.5f), get ("osc3Mix", 0.25f) };
    const float pitch[3]  = { get ("osc1Pitch", 0.0f), get ("osc2Pitch", 0.0f), get ("osc3Pitch", 0.0f) };
    const float detune[3] = { get ("osc1Detune", 0.0f), get ("osc2Detune", 0.0f), get ("osc3Detune", 0.0f) };

    const int unison = juce::jlimit (1, 8, (int) get ("unisonVoices", 1.0f));
    const float unisonDet = get ("unisonDetune", 0.1f);
    const float unisonStereo = get ("unisonStereo", 0.5f);
    const float noiseMix = get ("noiseMix", 0.0f);
    const float octave = get ("octave", 0.0f);

    const int filterType = (int) get ("filterType", 0.0f);
    const float baseCutoff = get ("filterCutoff", 2400.0f);
    const float resonance = get ("filterResonance", 0.707f);
    const float keytrack = get ("filterKeytrack", 0.0f);
    const float filtEnvAmount = get ("filterEnvAmount", 0.0f);
    const float lfoRate = get ("lfoRate", 3.0f);
    const float lfoAmount = get ("lfoAmount", 0.0f);

    const int modKind = (int) get ("modKind", 0.0f);
    const float modAmount = get ("modAmount", 0.0f);
    const float pitchEnvAmount = get ("pitchEnvAmount", 0.0f);
    const float pitchEnvDecay = get ("pitchEnvDecay", 0.1f);

    const double sr = getSampleRate();
    const float noteHzBase = 440.0f * std::pow (2.0f, (float) (note + octave * 12.0f - 69.0f) / 12.0f);
    const float keytrackMul = std::pow (2.0f, keytrack * (float) (note - 60) / 12.0f);
    const float pitchEnvCoeff = 1.0f - std::exp (-1.0f / (juce::jmax (0.001f, pitchEnvDecay) * (float) sr));

    for (int i = 0; i < numSamples; ++i)
    {
        const float amp = ampEnv.getNextSample();
        const float fe = filtEnv.getNextSample();

        pitchEnvLevel += (0.0f - pitchEnvLevel) * pitchEnvCoeff;
        const float noteHz = noteHzBase * std::pow (2.0f, (pitchEnvLevel * pitchEnvAmount) / 12.0f);

        const float lfo = std::sin (juce::MathConstants<float>::twoPi * lfoPhase);
        lfoPhase += lfoRate / (float) sr;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;

        float cutoff = baseCutoff * keytrackMul
                     * std::pow (2.0f, lfo * lfoAmount * 2.0f + fe * filtEnvAmount * 4.0f);
        cutoff = juce::jlimit (20.0f, (float) sr * 0.45f, cutoff);

        float l = 0.0f, r = 0.0f;
        float modSignal = 0.0f;

        for (int o = 0; o < 3; ++o)
        {
            if (mix[o] <= 0.0001f && ! (o == 0 && modAmount > 0.0001f))
                continue;

            const auto& tbl = *tables[o];

            float phaseOffset = 0.0f;
            float gainMod = 1.0f;

            if (o > 0 && modAmount > 0.0001f)
            {
                if (modKind == 1)
                    phaseOffset = modSignal * modAmount * 2.0f;          // FM / phase mod
                else if (modKind == 2)
                    gainMod = (1.0f - modAmount) + modAmount * modSignal; // ring
                else if (modKind == 3)
                    gainMod = (1.0f - modAmount) + modAmount * (0.5f + 0.5f * modSignal); // AM
            }

            float oscMono = 0.0f;

            for (int v = 0; v < unison; ++v)
            {
                const float spread = unison > 1 ? ((float) v / (float) (unison - 1)) * 2.0f - 1.0f : 0.0f;
                const float cents = spread * unisonDet * 100.0f + detune[o];
                const float freq = noteHz * std::pow (2.0f, (pitch[o] * 100.0f + cents) / 1200.0f);
                const float raw = readTable (tbl, phases[o][v] + phaseOffset);
                const float s = raw * mix[o] / (float) unison * gainMod;
                const float pan = spread * unisonStereo;

                if (o == 0)
                    oscMono += raw / (float) unison;

                l += s * std::sqrt ((1.0f - pan) * 0.5f);
                r += s * std::sqrt ((1.0f + pan) * 0.5f);

                float ph = phases[o][v] + freq / (float) sr;
                phases[o][v] = ph - std::floor (ph);
            }

            if (o == 0)
                modSignal = oscMono;
        }

        const float nz = nextNoise() * noiseMix;
        l += nz;
        r += nz;

        const float outL = filtL.process (l, cutoff, resonance, sr, filterType);
        const float outR = filtR.process (r, cutoff, resonance, sr, filterType);

        const float g = amp * velocity;
        outputBuffer.addSample (0, startSample + i, outL * g);
        if (outputBuffer.getNumChannels() > 1)
            outputBuffer.addSample (1, startSample + i, outR * g);
    }

    if (! ampEnv.isActive())
        clearCurrentNote();
}
