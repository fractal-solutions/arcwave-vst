#include "Presets.h"
#include "WavetableBank.h"

#include <algorithm>

namespace
{
float waveIndex (const juce::String& name, int fallback = 0)
{
    const auto& names = WavetableBank::getInstance().names();
    for (int i = 0; i < (int) names.size(); ++i)
        if (names[(size_t) i].equalsIgnoreCase (name))
            return (float) i;

    return (float) fallback;
}
}

std::vector<FactoryPreset> makeFactoryPresets()
{
    using V = std::vector<std::pair<juce::String, float>>;
    const auto W = [] (const char* n) { return waveIndex (n); };

    std::vector<FactoryPreset> p;
    const auto add = [&p] (const char* name, const char* cat, V vals)
    {
        p.push_back ({ name, cat, std::move (vals) });
    };

    add ("Init", "Init", {});

    // ---------------------------------------------------------------- Bass
    add ("Velvet Sub", "Bass", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Saw") },
        { "osc1Mix", 1.0f }, { "osc2Mix", 0.6f }, { "osc3Mix", 0.15f }, { "osc2Pitch", -12 },
        { "octave", -1 }, { "unisonVoices", 2 }, { "unisonDetune", 0.05f }, { "unisonStereo", 0.2f },
        { "filterCutoff", 500 }, { "filterResonance", 0.8f }, { "filterEnvAmount", 0.2f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.4f }, { "ampSustain", 0.9f }, { "ampRelease", 0.3f },
        { "fxDrive", 0.15f } });

    add ("Reese Reactor", "Bass", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 18 }, { "osc3Detune", -22 }, { "osc3Mix", 0.5f },
        { "unisonVoices", 5 }, { "unisonDetune", 0.22f }, { "unisonStereo", 0.7f },
        { "filterCutoff", 900 }, { "filterResonance", 1.1f }, { "filterEnvAmount", 0.3f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.5f }, { "ampSustain", 0.85f }, { "ampRelease", 0.5f },
        { "fxDrive", 0.2f }, { "fxReverbMix", 0.08f } });

    add ("Acid Filament", "Bass", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc2Mix", 0.4f }, { "osc3Mix", 0.0f },
        { "filterCutoff", 380 }, { "filterResonance", 2.6f }, { "filterEnvAmount", 0.9f },
        { "filterKeytrack", 0.4f }, { "filterDecay", 0.18f },
        { "ampAttack", 0.002f }, { "ampDecay", 0.25f }, { "ampSustain", 0.6f }, { "ampRelease", 0.2f },
        { "fxDrive", 0.25f }, { "fxDelayMix", 0.15f }, { "fxDelayTime", 0.18f } });

    add ("Rubber Pulse", "Bass", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", -12 }, { "osc2Mix", 0.5f }, { "osc3Mix", 0.2f },
        { "unisonVoices", 3 }, { "unisonDetune", 0.08f },
        { "filterCutoff", 700 }, { "filterResonance", 1.4f }, { "filterEnvAmount", 0.5f },
        { "ampAttack", 0.004f }, { "ampDecay", 0.3f }, { "ampSustain", 0.7f }, { "ampRelease", 0.25f } });

    add ("Wobble Engine", "Bass", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Mix", 0.5f }, { "osc3Mix", 0.25f }, { "unisonVoices", 4 }, { "unisonDetune", 0.15f },
        { "filterCutoff", 600 }, { "filterResonance", 1.8f }, { "filterEnvAmount", 0.6f },
        { "filterDecay", 0.4f }, { "lfoRate", 2.0f }, { "lfoAmount", 0.7f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.3f },
        { "fxDrive", 0.25f } });

    add ("Growl Bass", "Bass", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.35f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.2f }, { "osc1Pitch", 12 },
        { "filterCutoff", 900 }, { "filterResonance", 1.5f }, { "filterEnvAmount", 0.4f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.3f },
        { "fxDrive", 0.35f } });

    add ("Sub Punch", "Bass", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", -12 }, { "osc2Mix", 0.5f }, { "osc3Mix", 0.1f },
        { "pitchEnvAmount", 24 }, { "pitchEnvDecay", 0.06f },
        { "filterCutoff", 180 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.35f }, { "ampSustain", 0.0f }, { "ampRelease", 0.15f },
        { "fxDrive", 0.2f } });

    add ("Pluck Bass", "Bass", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc2Mix", 0.4f }, { "osc3Mix", 0.0f },
        { "filterCutoff", 400 }, { "filterResonance", 1.2f }, { "filterEnvAmount", 0.8f }, { "filterDecay", 0.16f },
        { "ampAttack", 0.002f }, { "ampDecay", 0.3f }, { "ampSustain", 0.0f }, { "ampRelease", 0.2f },
        { "fxDrive", 0.2f }, { "fxDelayMix", 0.1f }, { "fxDelayTime", 0.2f } });

    // --------------------------------------------------------------- Leads
    add ("Prism Lead", "Leads", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Sine") },
        { "osc2Detune", 10 }, { "osc3Mix", 0.2f },
        { "unisonVoices", 5 }, { "unisonDetune", 0.16f }, { "unisonStereo", 0.6f },
        { "filterCutoff", 6500 }, { "filterResonance", 0.9f }, { "filterKeytrack", 0.3f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.5f },
        { "fxDrive", 0.1f }, { "fxDelayMix", 0.25f }, { "fxDelayTime", 0.28f }, { "fxDelayFeedback", 0.35f },
        { "fxReverbMix", 0.2f }, { "fxReverbSize", 0.6f } });

    add ("Neon Wire", "Leads", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Saw") }, { "osc2Detune", -12 }, { "osc3Mix", 0.0f },
        { "unisonVoices", 3 }, { "unisonDetune", 0.12f },
        { "filterCutoff", 3800 }, { "filterResonance", 1.8f }, { "filterEnvAmount", 0.6f }, { "filterDecay", 0.3f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.3f }, { "ampSustain", 0.7f }, { "ampRelease", 0.4f },
        { "fxDrive", 0.35f }, { "fxDelayMix", 0.2f } });

    add ("Vocal Beacon", "Leads", V {
        { "osc1Wave", W ("Vocal") }, { "osc2Wave", W ("Throat") }, { "osc3Wave", W ("Sine") },
        { "osc2Detune", 8 }, { "osc3Mix", 0.15f },
        { "unisonVoices", 4 }, { "unisonDetune", 0.14f }, { "unisonStereo", 0.8f },
        { "filterCutoff", 4200 }, { "filterResonance", 1.2f }, { "filterEnvAmount", 0.4f },
        { "fxReverbMix", 0.35f }, { "fxReverbSize", 0.7f }, { "fxDelayMix", 0.2f } });

    add ("Glass Anthem", "Leads", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Triangle") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", 12 }, { "osc2Mix", 0.5f }, { "osc3Pitch", 24 }, { "osc3Mix", 0.2f },
        { "filterCutoff", 9000 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.6f }, { "ampSustain", 0.6f }, { "ampRelease", 0.9f },
        { "fxReverbMix", 0.4f }, { "fxReverbSize", 0.75f }, { "fxDelayMix", 0.25f } });

    add ("Screamer Lead", "Leads", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Saw") },
        { "osc2Mix", 0.6f }, { "osc3Mix", 0.3f }, { "unisonVoices", 4 }, { "unisonDetune", 0.18f },
        { "filterCutoff", 2600 }, { "filterResonance", 2.2f }, { "filterEnvAmount", 0.6f }, { "filterDecay", 0.25f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.3f }, { "ampSustain", 0.75f }, { "ampRelease", 0.4f },
        { "fxDrive", 0.5f }, { "fxDelayMix", 0.15f } });

    add ("Pure Whistle", "Leads", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", 12 }, { "osc2Mix", 0.2f }, { "osc3Mix", 0.0f },
        { "filterCutoff", 8000 }, { "filterResonance", 0.7f },
        { "lfoRate", 5.5f }, { "lfoAmount", 0.05f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.3f }, { "ampSustain", 0.9f }, { "ampRelease", 0.4f },
        { "fxDelayMix", 0.25f }, { "fxDelayTime", 0.3f }, { "fxReverbMix", 0.2f } });

    add ("Trance Lead", "Leads", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 12 }, { "osc3Detune", -12 }, { "osc3Mix", 0.5f },
        { "unisonVoices", 7 }, { "unisonDetune", 0.28f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 5200 }, { "filterResonance", 0.9f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.4f }, { "ampSustain", 0.85f }, { "ampRelease", 0.6f },
        { "fxDelayMix", 0.3f }, { "fxDelayTime", 0.28f }, { "fxDelayFeedback", 0.4f },
        { "fxReverbMix", 0.25f }, { "fxReverbSize", 0.65f } });

    add ("Sync Lead", "Leads", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.5f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.15f }, { "osc1Pitch", 12 },
        { "filterCutoff", 4200 }, { "filterResonance", 1.4f }, { "filterEnvAmount", 0.5f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.35f }, { "ampSustain", 0.8f }, { "ampRelease", 0.45f },
        { "fxDrive", 0.25f }, { "fxDelayMix", 0.2f } });

    // ---------------------------------------------------------------- Saws
    add ("Supersaw Anthem", "Saws", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 15 }, { "osc3Detune", -15 }, { "osc3Mix", 0.6f },
        { "unisonVoices", 7 }, { "unisonDetune", 0.30f }, { "unisonStereo", 0.95f },
        { "filterCutoff", 7000 }, { "filterResonance", 0.8f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.4f }, { "ampSustain", 0.85f }, { "ampRelease", 0.6f },
        { "fxDelayMix", 0.25f }, { "fxReverbMix", 0.25f }, { "fxReverbSize", 0.7f } });

    add ("Detuned Saw", "Saws", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 25 }, { "osc3Detune", -25 }, { "osc3Mix", 0.5f },
        { "unisonVoices", 5 }, { "unisonDetune", 0.25f }, { "unisonStereo", 0.8f },
        { "filterCutoff", 4500 }, { "filterResonance", 1.0f },
        { "ampAttack", 0.02f }, { "ampDecay", 0.5f }, { "ampSustain", 0.8f }, { "ampRelease", 0.6f },
        { "fxReverbMix", 0.2f } });

    add ("Retro Saw", "Saws", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Sine") },
        { "osc2Detune", 8 }, { "osc3Mix", 0.2f }, { "unisonVoices", 3 }, { "unisonDetune", 0.12f },
        { "filterCutoff", 3200 }, { "filterResonance", 1.2f }, { "filterEnvAmount", 0.25f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.3f }, { "ampSustain", 0.7f }, { "ampRelease", 0.4f },
        { "fxDrive", 0.15f }, { "fxDelayMix", 0.2f } });

    add ("Saw Stack", "Saws", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Pitch", -12 }, { "osc3Pitch", 12 }, { "osc2Mix", 0.6f }, { "osc3Mix", 0.4f },
        { "unisonVoices", 6 }, { "unisonDetune", 0.22f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 6000 }, { "filterResonance", 0.9f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.5f },
        { "fxReverbMix", 0.2f } });

    add ("Saw Pluck", "Saws", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.1f }, { "unisonVoices", 4 }, { "unisonDetune", 0.15f },
        { "filterCutoff", 3000 }, { "filterResonance", 1.4f }, { "filterEnvAmount", 0.7f }, { "filterDecay", 0.18f },
        { "ampAttack", 0.002f }, { "ampDecay", 0.35f }, { "ampSustain", 0.0f }, { "ampRelease", 0.3f },
        { "fxDelayMix", 0.2f }, { "fxDelayTime", 0.22f }, { "fxDelayFeedback", 0.35f }, { "fxReverbMix", 0.2f } });

    add ("Wide Saw", "Saws", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 20 }, { "osc3Detune", -20 }, { "osc3Mix", 0.5f }, { "noiseMix", 0.05f },
        { "unisonVoices", 8 }, { "unisonDetune", 0.35f }, { "unisonStereo", 1.0f },
        { "filterCutoff", 8000 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.02f }, { "ampDecay", 0.5f }, { "ampSustain", 0.85f }, { "ampRelease", 0.8f },
        { "fxReverbMix", 0.3f }, { "fxReverbSize", 0.7f } });

    // ------------------------------------------------------- Pads - Ambient
    add ("Ambient Aurora", "Pads - Ambient", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Silk") },
        { "osc2Detune", 15 }, { "osc3Mix", 0.3f }, { "unisonVoices", 7 }, { "unisonDetune", 0.30f },
        { "unisonStereo", 1.0f }, { "filterCutoff", 2200 }, { "filterResonance", 1.0f },
        { "filterEnvAmount", 0.25f }, { "filterAttack", 0.8f }, { "filterDecay", 1.2f },
        { "ampAttack", 0.9f }, { "ampDecay", 1.2f }, { "ampSustain", 0.8f }, { "ampRelease", 1.8f },
        { "lfoRate", 0.35f }, { "lfoAmount", 0.25f },
        { "fxReverbMix", 0.45f }, { "fxReverbSize", 0.85f }, { "fxDelayMix", 0.2f } });

    add ("Deep Space", "Pads - Ambient", V {
        { "osc1Wave", W ("Smooth Sub") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Saw") },
        { "osc2Pitch", -12 }, { "osc2Detune", 5 }, { "osc3Mix", 0.15f }, { "octave", -1 },
        { "filterCutoff", 700 }, { "filterResonance", 0.9f },
        { "ampAttack", 1.5f }, { "ampDecay", 2.0f }, { "ampSustain", 0.8f }, { "ampRelease", 2.5f },
        { "lfoRate", 0.15f }, { "lfoAmount", 0.2f }, { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.9f } });

    add ("Celestial Choir", "Pads - Ambient", V {
        { "osc1Wave", W ("Vocal") }, { "osc2Wave", W ("Vocal") }, { "osc3Wave", W ("Silk") },
        { "osc2Detune", -10 }, { "osc3Mix", 0.4f }, { "unisonVoices", 6 }, { "unisonDetune", 0.24f },
        { "unisonStereo", 0.9f }, { "filterCutoff", 3000 }, { "filterResonance", 1.1f },
        { "ampAttack", 1.2f }, { "ampDecay", 1.5f }, { "ampSustain", 0.85f }, { "ampRelease", 2.2f },
        { "fxReverbMix", 0.55f }, { "fxReverbSize", 0.9f } });

    add ("Ambient Drift", "Pads - Ambient", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("String") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.3f }, { "unisonVoices", 5 }, { "unisonDetune", 0.2f }, { "unisonStereo", 0.8f },
        { "filterCutoff", 1800 }, { "filterResonance", 1.2f },
        { "ampAttack", 1.0f }, { "ampDecay", 1.5f }, { "ampSustain", 0.8f }, { "ampRelease", 2.0f },
        { "lfoRate", 0.25f }, { "lfoAmount", 0.3f }, { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.85f } });

    add ("Frozen Horizon", "Pads - Ambient", V {
        { "osc1Wave", W ("Digital") }, { "osc2Wave", W ("Silk") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.3f }, { "unisonVoices", 5 }, { "unisonDetune", 0.22f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 2500 }, { "filterResonance", 1.4f }, { "filterAttack", 1.0f }, { "filterDecay", 1.5f },
        { "ampAttack", 1.2f }, { "ampDecay", 1.8f }, { "ampSustain", 0.8f }, { "ampRelease", 2.4f },
        { "fxReverbMix", 0.55f }, { "fxReverbSize", 0.9f } });

    // --------------------------------------------------------- Pads - Bloom
    add ("Bloom Cathedral", "Pads - Bloom", V {
        { "osc1Wave", W ("Vocal") }, { "osc2Wave", W ("String") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.3f }, { "unisonVoices", 6 }, { "unisonDetune", 0.2f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 2600 }, { "filterResonance", 1.2f }, { "filterEnvAmount", 0.5f },
        { "filterAttack", 1.2f }, { "filterDecay", 2.5f },
        { "ampAttack", 1.4f }, { "ampDecay", 2.0f }, { "ampSustain", 0.85f }, { "ampRelease", 2.3f },
        { "fxReverbMix", 0.6f }, { "fxReverbSize", 0.92f } });

    add ("Bloom Rising", "Pads - Bloom", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Silk") },
        { "osc2Detune", 12 }, { "osc3Mix", 0.35f }, { "unisonVoices", 7 }, { "unisonDetune", 0.3f },
        { "unisonStereo", 1.0f }, { "filterCutoff", 800 }, { "filterResonance", 1.6f },
        { "filterEnvAmount", 0.8f }, { "filterAttack", 2.0f }, { "filterDecay", 3.0f },
        { "ampAttack", 2.0f }, { "ampDecay", 2.5f }, { "ampSustain", 0.9f }, { "ampRelease", 3.0f },
        { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.9f } });

    add ("Bloom Glass", "Pads - Bloom", V {
        { "osc1Wave", W ("Silk") }, { "osc2Wave", W ("Digital") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.3f }, { "unisonVoices", 5 }, { "unisonDetune", 0.18f }, { "unisonStereo", 0.85f },
        { "filterCutoff", 5000 }, { "filterResonance", 1.3f }, { "filterEnvAmount", 0.4f },
        { "filterAttack", 0.8f }, { "filterDecay", 1.6f },
        { "ampAttack", 0.8f }, { "ampDecay", 1.5f }, { "ampSustain", 0.7f }, { "ampRelease", 2.2f },
        { "fxReverbMix", 0.55f }, { "fxReverbSize", 0.85f }, { "fxDelayMix", 0.2f } });

    add ("Bloom Choir", "Pads - Bloom", V {
        { "osc1Wave", W ("Vocal") }, { "osc2Wave", W ("Vocal") }, { "osc3Wave", W ("Silk") },
        { "osc2Detune", -8 }, { "osc3Mix", 0.4f }, { "unisonVoices", 7 }, { "unisonDetune", 0.26f },
        { "unisonStereo", 1.0f }, { "filterCutoff", 3500 }, { "filterResonance", 1.1f },
        { "filterEnvAmount", 0.3f }, { "filterAttack", 1.0f }, { "filterDecay", 2.0f },
        { "ampAttack", 1.2f }, { "ampDecay", 1.8f }, { "ampSustain", 0.85f }, { "ampRelease", 2.6f },
        { "fxReverbMix", 0.6f }, { "fxReverbSize", 0.9f } });

    add ("Bloom Sub", "Pads - Bloom", V {
        { "osc1Wave", W ("Smooth Sub") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Saw") },
        { "osc2Pitch", -12 }, { "osc3Mix", 0.2f }, { "octave", -1 },
        { "filterCutoff", 600 }, { "filterResonance", 1.0f }, { "filterEnvAmount", 0.5f },
        { "filterAttack", 1.5f }, { "filterDecay", 2.5f },
        { "ampAttack", 1.5f }, { "ampDecay", 2.0f }, { "ampSustain", 0.8f }, { "ampRelease", 2.8f },
        { "fxReverbMix", 0.4f }, { "fxReverbSize", 0.8f } });

    // ------------------------------------------------------ Pads - Textured
    add ("Textured Ice", "Pads - Textured", V {
        { "osc1Wave", W ("Digital") }, { "osc2Wave", W ("Silk") }, { "osc3Wave", W ("Sine") },
        { "noiseMix", 0.08f }, { "unisonVoices", 5 }, { "unisonDetune", 0.25f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 4000 }, { "filterResonance", 1.6f },
        { "ampAttack", 0.8f }, { "ampDecay", 1.2f }, { "ampSustain", 0.7f }, { "ampRelease", 2.0f },
        { "lfoRate", 0.5f }, { "lfoAmount", 0.3f },
        { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.85f }, { "fxDelayMix", 0.2f } });

    add ("Textured Grain", "Pads - Textured", V {
        { "osc1Wave", W ("Filth") }, { "osc2Wave", W ("Dirt") }, { "osc3Wave", W ("Sine") },
        { "noiseMix", 0.15f }, { "unisonVoices", 4 }, { "unisonDetune", 0.2f },
        { "filterCutoff", 2200 }, { "filterResonance", 1.5f },
        { "ampAttack", 0.6f }, { "ampDecay", 1.0f }, { "ampSustain", 0.7f }, { "ampRelease", 1.8f },
        { "fxDrive", 0.15f }, { "fxReverbMix", 0.45f }, { "fxReverbSize", 0.8f } });

    add ("Textured Vocal", "Pads - Textured", V {
        { "osc1Wave", W ("Vocal") }, { "osc2Wave", W ("Throat") }, { "osc3Wave", W ("Silk") },
        { "noiseMix", 0.05f }, { "unisonVoices", 5 }, { "unisonDetune", 0.18f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 3200 }, { "filterResonance", 1.3f },
        { "ampAttack", 1.0f }, { "ampDecay", 1.4f }, { "ampSustain", 0.8f }, { "ampRelease", 2.2f },
        { "fxReverbMix", 0.55f }, { "fxReverbSize", 0.9f } });

    add ("Textured Metal", "Pads - Textured", V {
        { "osc1Wave", W ("Beast") }, { "osc2Wave", W ("Ogre") }, { "osc3Wave", W ("Digital") },
        { "noiseMix", 0.1f }, { "unisonVoices", 3 }, { "unisonDetune", 0.15f },
        { "filterCutoff", 3000 }, { "filterResonance", 2.0f },
        { "ampAttack", 0.5f }, { "ampDecay", 1.0f }, { "ampSustain", 0.7f }, { "ampRelease", 1.6f },
        { "fxDrive", 0.25f }, { "fxReverbMix", 0.4f }, { "fxReverbSize", 0.8f } });

    add ("Textured Dust", "Pads - Textured", V {
        { "osc1Wave", W ("Crunch") }, { "osc2Wave", W ("Dirt") }, { "osc3Wave", W ("Sine") },
        { "noiseMix", 0.2f }, { "unisonVoices", 3 }, { "unisonDetune", 0.1f },
        { "filterCutoff", 1800 }, { "filterResonance", 1.4f },
        { "ampAttack", 0.8f }, { "ampDecay", 1.2f }, { "ampSustain", 0.7f }, { "ampRelease", 1.8f },
        { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.85f } });

    // ---------------------------------------------------------- Percussion
    add ("Kick 909", "Percussion", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", -12 }, { "osc2Mix", 0.3f }, { "osc3Mix", 0.0f },
        { "pitchEnvAmount", 42 }, { "pitchEnvDecay", 0.045f },
        { "filterCutoff", 150 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.22f }, { "ampSustain", 0.0f }, { "ampRelease", 0.1f },
        { "fxDrive", 0.35f } });

    add ("Kick Sub", "Percussion", V {
        { "osc1Wave", W ("Smooth Sub") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", -12 }, { "osc2Mix", 0.2f }, { "osc3Mix", 0.0f }, { "octave", -1 },
        { "pitchEnvAmount", 30 }, { "pitchEnvDecay", 0.08f },
        { "filterCutoff", 120 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.4f }, { "ampSustain", 0.0f }, { "ampRelease", 0.15f },
        { "fxDrive", 0.2f } });

    add ("Snare Noise", "Percussion", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc1Mix", 0.3f }, { "osc2Mix", 0.1f }, { "osc3Mix", 0.0f }, { "noiseMix", 0.9f },
        { "pitchEnvAmount", 12 }, { "pitchEnvDecay", 0.03f },
        { "filterType", 1 }, { "filterCutoff", 1200 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.18f }, { "ampSustain", 0.0f }, { "ampRelease", 0.08f },
        { "fxDrive", 0.2f }, { "fxReverbMix", 0.15f } });

    add ("Hat Closed", "Percussion", V {
        { "osc1Mix", 0.0f }, { "osc2Mix", 0.0f }, { "osc3Mix", 0.0f }, { "noiseMix", 1.0f },
        { "filterType", 1 }, { "filterCutoff", 8000 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.05f }, { "ampSustain", 0.0f }, { "ampRelease", 0.03f } });

    add ("Hat Open", "Percussion", V {
        { "osc1Mix", 0.0f }, { "osc2Mix", 0.0f }, { "osc3Mix", 0.0f }, { "noiseMix", 1.0f },
        { "filterType", 1 }, { "filterCutoff", 7000 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.35f }, { "ampSustain", 0.0f }, { "ampRelease", 0.2f } });

    add ("Tom Low", "Percussion", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", -5 }, { "osc2Mix", 0.3f }, { "osc3Mix", 0.0f },
        { "pitchEnvAmount", 24 }, { "pitchEnvDecay", 0.12f },
        { "filterCutoff", 800 }, { "filterResonance", 0.9f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.5f }, { "ampSustain", 0.0f }, { "ampRelease", 0.2f },
        { "fxDrive", 0.1f } });

    add ("Clave", "Percussion", V {
        { "osc1Wave", W ("Triangle") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", 12 }, { "osc2Mix", 0.3f }, { "osc3Mix", 0.0f },
        { "pitchEnvAmount", 18 }, { "pitchEnvDecay", 0.015f },
        { "filterType", 1 }, { "filterCutoff", 1500 }, { "filterResonance", 0.8f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.12f }, { "ampSustain", 0.0f }, { "ampRelease", 0.08f } });

    add ("Metallic Hit", "Percussion", V {
        { "osc1Wave", W ("Digital") }, { "osc2Wave", W ("Double Spike") }, { "osc3Wave", W ("Sine") },
        { "modKind", 2 }, { "modAmount", 0.6f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.2f }, { "osc1Pitch", 7 }, { "noiseMix", 0.05f },
        { "filterCutoff", 6000 }, { "filterResonance", 1.0f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.6f }, { "ampSustain", 0.0f }, { "ampRelease", 0.4f },
        { "fxReverbMix", 0.3f } });

    add ("Laser Zap", "Percussion", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.3f }, { "osc1Mix", 0.0f }, { "osc2Mix", 0.8f },
        { "osc1Pitch", 12 }, { "pitchEnvAmount", -36 }, { "pitchEnvDecay", 0.12f },
        { "filterCutoff", 5000 }, { "filterResonance", 1.5f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.3f }, { "ampSustain", 0.0f }, { "ampRelease", 0.15f },
        { "fxDelayMix", 0.2f } });

    // ----------------------------------------------------------- Electronic
    add ("EDM Stab", "Electronic", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 10 }, { "osc3Mix", 0.4f }, { "unisonVoices", 5 }, { "unisonDetune", 0.18f },
        { "unisonStereo", 0.7f }, { "filterCutoff", 3000 }, { "filterResonance", 1.2f },
        { "filterEnvAmount", 0.5f }, { "filterDecay", 0.2f },
        { "ampAttack", 0.002f }, { "ampDecay", 0.35f }, { "ampSustain", 0.0f }, { "ampRelease", 0.3f },
        { "fxDrive", 0.2f }, { "fxReverbMix", 0.3f }, { "fxDelayMix", 0.2f } });

    add ("Rave Chord", "Electronic", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "osc2Detune", 12 }, { "osc3Detune", -12 }, { "osc3Mix", 0.5f },
        { "unisonVoices", 7 }, { "unisonDetune", 0.25f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 6000 }, { "filterResonance", 1.0f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.5f },
        { "fxDrive", 0.15f }, { "fxReverbMix", 0.25f }, { "fxDelayMix", 0.2f } });

    add ("Techno Pluck", "Electronic", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.1f }, { "filterCutoff", 1200 }, { "filterResonance", 1.8f },
        { "filterEnvAmount", 0.8f }, { "filterDecay", 0.12f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.25f }, { "ampSustain", 0.0f }, { "ampRelease", 0.2f },
        { "fxDrive", 0.3f }, { "fxDelayMix", 0.2f }, { "fxDelayTime", 0.18f }, { "fxDelayFeedback", 0.4f },
        { "fxReverbMix", 0.2f } });

    add ("Future Bass", "Electronic", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Saw") },
        { "osc2Mix", 0.6f }, { "osc3Mix", 0.4f }, { "unisonVoices", 6 }, { "unisonDetune", 0.28f },
        { "unisonStereo", 0.9f }, { "filterCutoff", 3500 }, { "filterResonance", 1.3f },
        { "filterEnvAmount", 0.4f }, { "lfoRate", 3.0f }, { "lfoAmount", 0.2f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.5f },
        { "fxDelayMix", 0.2f }, { "fxReverbMix", 0.25f } });

    add ("Digital Arp", "Electronic", V {
        { "osc1Wave", W ("Digital") }, { "osc2Wave", W ("Square") }, { "osc3Wave", W ("Sine") },
        { "osc3Mix", 0.1f }, { "filterCutoff", 4000 }, { "filterResonance", 1.4f },
        { "filterEnvAmount", 0.6f }, { "filterDecay", 0.15f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.2f }, { "ampSustain", 0.0f }, { "ampRelease", 0.15f },
        { "fxDelayMix", 0.35f }, { "fxDelayTime", 0.16f }, { "fxDelayFeedback", 0.45f },
        { "fxReverbMix", 0.25f } });

    add ("Trance Pluck", "Electronic", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Silk") },
        { "osc2Detune", 12 }, { "osc3Mix", 0.3f }, { "unisonVoices", 7 }, { "unisonDetune", 0.25f },
        { "unisonStereo", 0.9f }, { "filterCutoff", 6000 }, { "filterResonance", 0.9f },
        { "ampAttack", 0.001f }, { "ampDecay", 0.12f }, { "ampSustain", 0.0f }, { "ampRelease", 0.15f },
        { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.8f }, { "fxDelayMix", 0.2f } });

    // ------------------------------------------------------------------ FM
    add ("FM Bell", "FM", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.6f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.2f }, { "osc1Pitch", 24 },
        { "filterCutoff", 9000 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.001f }, { "ampDecay", 1.4f }, { "ampSustain", 0.0f }, { "ampRelease", 1.6f },
        { "fxReverbMix", 0.3f }, { "fxDelayMix", 0.2f } });

    add ("FM Bass", "FM", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.4f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.0f }, { "osc1Pitch", 12 }, { "osc2Pitch", -12 },
        { "filterCutoff", 800 }, { "filterResonance", 1.2f }, { "filterEnvAmount", 0.3f },
        { "ampAttack", 0.003f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.3f },
        { "fxDrive", 0.25f } });

    add ("FM Lead", "FM", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.45f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.15f }, { "osc1Pitch", 12 }, { "unisonVoices", 3 }, { "unisonDetune", 0.08f },
        { "filterCutoff", 5000 }, { "filterResonance", 1.1f }, { "filterEnvAmount", 0.3f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.35f }, { "ampSustain", 0.8f }, { "ampRelease", 0.4f },
        { "fxDelayMix", 0.2f }, { "fxReverbMix", 0.2f } });

    add ("FM Keys", "FM", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Sine") },
        { "modKind", 1 }, { "modAmount", 0.3f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.0f }, { "osc1Pitch", 19 },
        { "filterCutoff", 6000 }, { "filterResonance", 0.8f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.9f }, { "ampSustain", 0.4f }, { "ampRelease", 0.7f },
        { "fxReverbMix", 0.25f }, { "fxDelayMix", 0.1f } });

    add ("FM Growl", "FM", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Saw") }, { "osc3Wave", W ("Saw") },
        { "modKind", 1 }, { "modAmount", 0.55f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.3f }, { "osc1Pitch", 7 },
        { "filterCutoff", 1500 }, { "filterResonance", 1.6f }, { "filterEnvAmount", 0.4f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.4f }, { "ampSustain", 0.8f }, { "ampRelease", 0.3f },
        { "fxDrive", 0.4f } });

    add ("FM Drone", "FM", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Silk") },
        { "modKind", 1 }, { "modAmount", 0.35f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.3f }, { "osc1Pitch", 5 },
        { "filterCutoff", 2000 }, { "filterResonance", 1.3f },
        { "ampAttack", 1.2f }, { "ampDecay", 1.8f }, { "ampSustain", 0.8f }, { "ampRelease", 2.5f },
        { "lfoRate", 0.2f }, { "lfoAmount", 0.2f }, { "fxReverbMix", 0.5f }, { "fxReverbSize", 0.9f } });

    // ---------------------------------------------------------------- Keys
    add ("Velvet Rhodes", "Keys", V {
        { "osc1Wave", W ("Rhodes") }, { "osc2Wave", W ("Keys") }, { "osc3Wave", W ("Sine") },
        { "osc2Detune", 6 }, { "osc2Mix", 0.4f }, { "osc3Mix", 0.1f },
        { "unisonVoices", 3 }, { "unisonDetune", 0.08f }, { "unisonStereo", 0.5f },
        { "filterCutoff", 4500 }, { "filterResonance", 0.8f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.9f }, { "ampSustain", 0.5f }, { "ampRelease", 0.7f },
        { "fxReverbMix", 0.25f }, { "fxDelayMix", 0.1f } });

    add ("Neon Organ", "Keys", V {
        { "osc1Wave", W ("Square") }, { "osc2Wave", W ("Sine") }, { "osc3Wave", W ("Square") },
        { "osc2Pitch", 12 }, { "osc2Mix", 0.5f }, { "osc3Pitch", 19 }, { "osc3Mix", 0.25f },
        { "filterCutoff", 6000 }, { "filterResonance", 0.7f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.1f }, { "ampSustain", 1.0f }, { "ampRelease", 0.2f },
        { "fxDrive", 0.1f }, { "fxReverbMix", 0.15f } });

    add ("Digital EP", "Keys", V {
        { "osc1Wave", W ("Keys") }, { "osc2Wave", W ("Rhodes") }, { "osc3Wave", W ("Sine") },
        { "osc2Detune", 5 }, { "osc2Mix", 0.4f }, { "osc3Mix", 0.1f }, { "unisonVoices", 2 },
        { "unisonDetune", 0.05f }, { "filterCutoff", 3800 }, { "filterResonance", 0.9f },
        { "ampAttack", 0.005f }, { "ampDecay", 0.8f }, { "ampSustain", 0.4f }, { "ampRelease", 0.6f },
        { "fxReverbMix", 0.3f }, { "fxDelayMix", 0.15f } });

    add ("Glass Keys", "Keys", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Triangle") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", 12 }, { "osc2Mix", 0.4f }, { "osc3Pitch", 24 }, { "osc3Mix", 0.15f },
        { "filterCutoff", 8000 }, { "filterResonance", 0.8f },
        { "ampAttack", 0.002f }, { "ampDecay", 0.9f }, { "ampSustain", 0.3f }, { "ampRelease", 0.8f },
        { "fxReverbMix", 0.35f }, { "fxDelayMix", 0.2f } });

    // ------------------------------------------------------------------ FX
    add ("Riser Engine", "FX", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Filth") }, { "osc3Wave", W ("Dirt") },
        { "osc2Mix", 0.5f }, { "osc3Mix", 0.3f }, { "noiseMix", 0.3f },
        { "unisonVoices", 6 }, { "unisonDetune", 0.4f }, { "unisonStereo", 0.9f },
        { "filterCutoff", 400 }, { "filterResonance", 2.2f }, { "filterEnvAmount", 0.8f },
        { "filterAttack", 1.8f }, { "filterDecay", 2.5f },
        { "ampAttack", 1.6f }, { "ampDecay", 1.0f }, { "ampSustain", 1.0f }, { "ampRelease", 0.6f },
        { "lfoRate", 0.5f }, { "lfoAmount", 0.4f }, { "fxReverbMix", 0.4f }, { "fxReverbSize", 0.8f } });

    add ("Impact Bloom", "FX", V {
        { "osc1Wave", W ("Sine") }, { "osc2Wave", W ("Smooth Sub") }, { "osc3Wave", W ("Sine") },
        { "osc2Pitch", -12 }, { "osc3Mix", 0.0f }, { "noiseMix", 0.4f },
        { "filterCutoff", 300 }, { "filterResonance", 1.5f }, { "filterEnvAmount", 0.6f },
        { "ampAttack", 0.002f }, { "ampDecay", 1.5f }, { "ampSustain", 0.0f }, { "ampRelease", 2.0f },
        { "fxDrive", 0.3f }, { "fxReverbMix", 0.6f }, { "fxReverbSize", 0.95f } });

    add ("Downlifter", "FX", V {
        { "osc1Wave", W ("Saw") }, { "osc2Wave", W ("Filth") }, { "osc3Wave", W ("Sine") },
        { "osc2Mix", 0.4f }, { "noiseMix", 0.2f }, { "pitchEnvAmount", 48 }, { "pitchEnvDecay", 1.5f },
        { "filterCutoff", 2000 }, { "filterResonance", 1.5f }, { "filterEnvAmount", 0.5f },
        { "filterAttack", 1.5f }, { "ampAttack", 1.0f }, { "ampDecay", 1.5f }, { "ampSustain", 0.8f },
        { "ampRelease", 1.0f }, { "fxReverbMix", 0.4f }, { "fxReverbSize", 0.8f } });

    add ("Noise Sweep", "FX", V {
        { "osc1Mix", 0.0f }, { "osc2Mix", 0.0f }, { "osc3Mix", 0.0f }, { "noiseMix", 1.0f },
        { "filterType", 2 }, { "filterCutoff", 200 }, { "filterResonance", 2.0f },
        { "filterEnvAmount", 0.8f }, { "filterAttack", 2.0f }, { "filterDecay", 3.0f },
        { "ampAttack", 1.5f }, { "ampDecay", 2.0f }, { "ampSustain", 1.0f }, { "ampRelease", 1.5f },
        { "fxReverbMix", 0.4f }, { "fxReverbSize", 0.8f } });

    add ("Alien Transmission", "FX", V {
        { "osc1Wave", W ("Digital") }, { "osc2Wave", W ("Beast") }, { "osc3Wave", W ("Sine") },
        { "modKind", 2 }, { "modAmount", 0.7f }, { "osc1Mix", 0.0f }, { "osc2Mix", 1.0f },
        { "osc3Mix", 0.2f }, { "osc1Pitch", 7 }, { "noiseMix", 0.1f },
        { "filterCutoff", 3000 }, { "filterResonance", 1.6f }, { "lfoRate", 6.0f }, { "lfoAmount", 0.5f },
        { "ampAttack", 0.01f }, { "ampDecay", 0.5f }, { "ampSustain", 0.7f }, { "ampRelease", 0.5f },
        { "fxDelayMix", 0.3f }, { "fxReverbMix", 0.35f } });

    return p;
}

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& apvtsToUse)
    : apvts (apvtsToUse), factory (makeFactoryPresets())
{
    userDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                  .getChildFile ("Qua$$a")
                  .getChildFile ("ArcWave Presets");
    userDir.createDirectory();
    rescanUserPresets();
    currentIndex = 0;
}

void PresetManager::rescanUserPresets()
{
    userFiles = userDir.findChildFiles (juce::File::findFiles, false, "*.arcpreset");
    std::sort (userFiles.begin(), userFiles.end(), [] (const juce::File& a, const juce::File& b)
    {
        return a.getFileName().compareIgnoreCase (b.getFileName()) < 0;
    });
}

juce::StringArray PresetManager::getNames() const
{
    juce::StringArray names;
    for (auto& f : factory)
        names.add (f.name);

    for (auto& f : userFiles)
        names.add (f.getFileNameWithoutExtension());

    return names;
}

juce::String PresetManager::getCategory (int index) const
{
    if (index < 0)
        return "Random";

    if (index < (int) factory.size())
        return factory[(size_t) index].category;

    return "User";
}

int PresetManager::getNumPresets() const
{
    return (int) factory.size() + userFiles.size();
}

juce::String PresetManager::getCurrentName() const
{
    if (currentIndex < 0)
        return "Random";

    if (currentIndex < (int) factory.size())
        return factory[(size_t) currentIndex].name;

    const int ui = currentIndex - (int) factory.size();
    if (ui >= 0 && ui < userFiles.size())
        return userFiles[ui].getFileNameWithoutExtension();

    return {};
}

void PresetManager::setRaw (const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

void PresetManager::resetToDefaults()
{
    for (auto* p : apvts.processor.getParameters())
        if (p != nullptr)
            p->setValueNotifyingHost (p->getDefaultValue());
}

void PresetManager::applyValues (const std::vector<std::pair<juce::String, float>>& values)
{
    for (auto& kv : values)
        setRaw (kv.first, kv.second);
}

void PresetManager::apply (int index)
{
    if (index < 0 || index >= getNumPresets())
        return;

    if (index < (int) factory.size())
    {
        resetToDefaults();
        applyValues (factory[(size_t) index].values);
    }
    else
    {
        const int ui = index - (int) factory.size();
        if (ui < 0 || ui >= userFiles.size())
            return;

        if (auto xml = juce::XmlDocument::parse (userFiles[ui]))
            if (xml->hasTagName (apvts.state.getType()))
                apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }

    currentIndex = index;
}

void PresetManager::next()
{
    const int n = getNumPresets();
    if (n == 0)
        return;
    apply ((currentIndex + 1) % n);
}

void PresetManager::previous()
{
    const int n = getNumPresets();
    if (n == 0)
        return;
    apply ((currentIndex - 1 + n) % n);
}

bool PresetManager::saveCurrent (const juce::String& name)
{
    const auto safe = juce::File::createLegalFileName (name).trim();
    if (safe.isEmpty())
        return false;

    auto xml = apvts.copyState().createXml();
    if (xml == nullptr)
        return false;

    userDir.createDirectory();
    const auto file = userDir.getChildFile (safe + ".arcpreset");
    if (! xml->writeTo (file))
        return false;

    rescanUserPresets();
    for (int i = 0; i < userFiles.size(); ++i)
        if (userFiles[i].getFileNameWithoutExtension().equalsIgnoreCase (safe))
            currentIndex = (int) factory.size() + i;

    return true;
}

void PresetManager::randomize()
{
    juce::Random r (juce::Time::currentTimeMillis());
    const int numWaves = juce::jmax (1, WavetableBank::getInstance().size());

    auto setRange = [this, &r] (const char* id, float lo, float hi)
    {
        setRaw (id, lo + r.nextFloat() * (hi - lo));
    };
    auto setChoice = [this, &r] (const char* id, int count)
    {
        setRaw (id, (float) r.nextInt (juce::jmax (1, count)));
    };

    setChoice ("osc1Wave", numWaves);
    setChoice ("osc2Wave", numWaves);
    setChoice ("osc3Wave", numWaves);

    setRange ("osc1Pitch", -12, 12);
    setRange ("osc2Pitch", -12, 12);
    setRange ("osc3Pitch", -12, 12);
    setRange ("osc1Detune", -25, 25);
    setRange ("osc2Detune", -25, 25);
    setRange ("osc3Detune", -25, 25);
    setRange ("osc1Mix", 0.4f, 1.0f);
    setRange ("osc2Mix", 0.2f, 0.8f);
    setRange ("osc3Mix", 0.0f, 0.5f);
    setRange ("noiseMix", 0.0f, 0.15f);
    setRange ("octave", -1, 1);
    setRange ("unisonVoices", 1, 6);
    setRange ("unisonDetune", 0.0f, 0.4f);
    setRange ("unisonStereo", 0.0f, 1.0f);

    setChoice ("filterType", 3);
    setRange ("filterCutoff", 300, 9000);
    setRange ("filterResonance", 0.7f, 2.5f);
    setRange ("filterKeytrack", 0.0f, 0.6f);
    setRange ("filterEnvAmount", -0.5f, 0.9f);

    setRange ("ampAttack", 0.001f, 0.4f);
    setRange ("ampDecay", 0.05f, 1.0f);
    setRange ("ampSustain", 0.3f, 1.0f);
    setRange ("ampRelease", 0.05f, 1.5f);
    setRange ("filterAttack", 0.001f, 0.5f);
    setRange ("filterDecay", 0.05f, 0.8f);

    setRange ("lfoRate", 0.2f, 8.0f);
    setRange ("lfoAmount", 0.0f, 0.6f);

    setChoice ("modKind", 4);
    setRange ("modAmount", 0.0f, 0.5f);
    setRange ("pitchEnvAmount", -24, 24);
    setRange ("pitchEnvDecay", 0.01f, 0.5f);

    setRange ("fxDrive", 0.0f, 0.5f);
    setRange ("fxCrusher", 0.0f, 0.4f);
    setRange ("fxDelayMix", 0.0f, 0.35f);
    setRange ("fxDelayTime", 0.08f, 0.5f);
    setRange ("fxDelayFeedback", 0.1f, 0.6f);
    setRange ("fxReverbMix", 0.0f, 0.4f);
    setRange ("fxReverbSize", 0.2f, 0.9f);
    setRaw ("masterGain", -12.0f);

    currentIndex = -1;
}
