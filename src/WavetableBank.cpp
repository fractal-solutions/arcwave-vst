#include "WavetableBank.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
#if JUCE_MAC
juce::String searchForGmsWaves (const juce::File& root)
{
    if (! root.isDirectory())
        return {};

    auto hits = root.findChildFiles (juce::File::findDirectories, true, "Synth Waves");
    for (auto& hit : hits)
        if (hit.getParentDirectory().getFileName().equalsIgnoreCase ("GMS"))
            return hit.getFullPathName();

    return {};
}
#endif

juce::String defaultWaveDir()
{
    auto env = juce::SystemStats::getEnvironmentVariable ("ARCWAVE_WAVETABLES", {});
    if (env.isNotEmpty())
        return env;

#if JUCE_WINDOWS
    // Prefer the newest installed FL Studio's GMS wavetables when present.
    const juce::File imageLine ("C:\\Program Files\\Image-Line");
    if (imageLine.isDirectory())
    {
        auto installs = imageLine.findChildFiles (juce::File::findDirectories, false, "FL Studio*");
        std::sort (installs.begin(), installs.end(), [] (const juce::File& a, const juce::File& b)
        {
            return a.getFileName().compareIgnoreCase (b.getFileName()) > 0;
        });

        for (auto& install : installs)
        {
            auto waves = install.getChildFile ("Plugins").getChildFile ("Fruity")
                               .getChildFile ("Generators").getChildFile ("GMS")
                               .getChildFile ("Synth Waves");
            if (waves.isDirectory())
                return waves.getFullPathName();
        }
    }

    return "C:\\Program Files\\Image-Line\\FL Studio 2025\\Plugins\\Fruity\\Generators\\GMS\\Synth Waves";
#elif JUCE_MAC
    const std::vector<juce::File> roots
    {
        juce::File ("/Applications"),
        juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Applications"),
        juce::File::getSpecialLocation (juce::File::userHomeDirectory)
            .getChildFile ("Documents").getChildFile ("Image-Line"),
        juce::File ("/Library/Application Support/Image-Line"),
        juce::File::getSpecialLocation (juce::File::userHomeDirectory)
            .getChildFile ("Library").getChildFile ("Application Support").getChildFile ("Image-Line")
    };

    for (auto& root : roots)
    {
        auto found = searchForGmsWaves (root);
        if (found.isNotEmpty())
            return found;
    }

    return {};
#else
    return {};
#endif
}
}

WavetableBank& WavetableBank::getInstance()
{
    static WavetableBank instance;
    return instance;
}

WavetableBank::WavetableBank()
{
    load();
}

void WavetableBank::addTable (const juce::String& name, std::vector<float> data)
{
    if ((int) data.size() != kTableSize)
        return;

    tableNames.push_back (name);
    tables.push_back (std::move (data));
}

const std::vector<float>& WavetableBank::table (int index) const noexcept
{
    static const std::vector<float> empty;
    if (tables.empty())
        return empty;

    index = juce::jlimit (0, (int) tables.size() - 1, index);
    return tables[(size_t) index];
}

std::vector<std::pair<juce::String, std::vector<float>>> WavetableBank::makeOriginalTables()
{
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float pi = juce::MathConstants<float>::pi;

    auto make = [] (auto fn)
    {
        std::vector<float> t ((size_t) kTableSize);
        double mean = 0.0;

        for (int i = 0; i < kTableSize; ++i)
        {
            t[(size_t) i] = fn ((float) i / (float) kTableSize);
            mean += (double) t[(size_t) i];
        }

        mean /= (double) kTableSize;

        float peak = 1.0e-6f;
        for (auto& v : t)
        {
            v -= (float) mean;
            peak = std::max (peak, std::abs (v));
        }

        const float g = 0.98f / peak;
        for (auto& v : t)
            v *= g;

        return t;
    };

    auto additive = [twoPi] (float p, int H, auto amp, auto phase)
    {
        float s = 0.0f;
        for (int h = 1; h <= H; ++h)
        {
            const float a = amp (h);
            if (a != 0.0f)
                s += a * std::sin (twoPi * (float) h * p + phase (h));
        }
        return s;
    };

    auto rawSaw = [] (float p) { return 2.0f * p - 1.0f; };

    auto saw = [additive] (float p)
    {
        return additive (p, 64, [] (int h) { return 1.0f / (float) h; }, [] (int) { return 0.0f; });
    };

    auto square = [additive] (float p)
    {
        return additive (p, 63, [] (int h) { return (h % 2) ? 1.0f / (float) h : 0.0f; },
                         [] (int) { return 0.0f; });
    };

    auto triangle = [additive] (float p)
    {
        return additive (p, 31, [] (int h)
        {
            if ((h % 2) == 0)
                return 0.0f;
            const int k = h / 2;
            const float sign = (k % 2) ? -1.0f : 1.0f;
            return sign / ((float) h * (float) h);
        }, [] (int) { return 0.0f; });
    };

    auto formant = [additive] (float p, float f1, float f2)
    {
        return additive (p, 32, [f1, f2] (int h)
        {
            float g = 0.12f / (float) h;
            g += std::exp (-0.5f * std::pow (((float) h - f1) / 1.5f, 2.0f));
            g += std::exp (-0.5f * std::pow (((float) h - f2) / 2.0f, 2.0f));
            return g;
        }, [] (int) { return 0.0f; });
    };

    std::vector<std::pair<juce::String, std::vector<float>>> out;
    auto add = [&out] (const char* name, std::vector<float> data)
    {
        out.emplace_back (juce::String (name), std::move (data));
    };

    add ("Beast", make ([&] (float p) { return std::tanh (4.0f * (rawSaw (p) + 0.4f * square (p))); }));

    add ("Crunch", make ([&] (float p)
    {
        return std::tanh (3.5f * square (p)) + 0.2f * std::sin (twoPi * 9.0f * p);
    }));

    add ("Digital", make ([&] (float p)
    {
        const float amps[16] = { 0.0f, 1.0f, 0.5f, 0.4f, 0.0f, 0.35f, 0.0f, 0.25f,
                                 0.2f, 0.0f, 0.0f, 0.18f, 0.0f, 0.15f, 0.0f, 0.1f };
        return additive (p, 15, [&amps] (int h) { return amps[h]; },
                         [] (int h) { return (float) (h % 3) * 1.1f; });
    }));

    juce::uint32 noiseState = 0x12345678u;
    add ("Dirt", make ([&] (float p)
    {
        noiseState = noiseState * 1664525u + 1013904223u;
        const float n = (float) ((noiseState >> 8) & 0xFFFFu) / 32768.0f - 1.0f;
        return std::tanh (2.8f * rawSaw (p)) + 0.16f * n;
    }));

    add ("Double Spike", make ([&] (float p)
    {
        return std::sin (twoPi * p)
             + 0.9f * std::sin (twoPi * 2.0f * p + 1.570796f)
             + 0.5f * std::sin (twoPi * 4.0f * p);
    }));

    add ("Electro", make ([&] (float p)
    {
        return additive (p, 64, [pi] (int h)
        {
            return (2.0f / (pi * (float) h)) * std::sin (pi * (float) h * 0.3f);
        }, [] (int) { return 0.0f; });
    }));

    add ("Filth", make ([&] (float p)
    {
        return std::tanh (6.0f * rawSaw (p)) + 0.1f * std::sin (twoPi * 17.0f * p);
    }));

    add ("Fox", make ([&] (float p) { return formant (p, 4.0f, 9.0f); }));

    add ("Funky", make ([&] (float p)
    {
        return saw (p) - 0.5f * std::sin (twoPi * 3.0f * p) + 0.4f * std::sin (twoPi * 7.0f * p);
    }));

    add ("Grubby", make ([&] (float p)
    {
        return std::round (rawSaw (p) * 8.0f) / 8.0f + 0.05f * std::sin (twoPi * 23.0f * p);
    }));

    add ("Keys", make ([&] (float p)
    {
        return std::sin (twoPi * p)
             + 0.5f * std::sin (twoPi * 4.0f * p)
             + 0.25f * std::sin (twoPi * 6.0f * p)
             + 0.12f * std::sin (twoPi * 8.0f * p);
    }));

    add ("Liquid", make ([&] (float p)
    {
        return std::sin (twoPi * p)
             + 0.5f * std::sin (twoPi * 2.01f * p)
             + 0.3f * std::sin (twoPi * 3.02f * p);
    }));

    add ("Ogre", make ([&] (float p)
    {
        return additive (p, 8, [] (int h)
        {
            return ((h == 1 || h == 3) ? 1.2f : 0.6f) / (float) h;
        }, [] (int) { return 0.0f; });
    }));

    add ("Rhodes", make ([&] (float p)
    {
        return std::sin (twoPi * p)
             + 0.5f * std::sin (twoPi * 4.0f * p)
             + 0.2f * std::sin (twoPi * 6.0f * p);
    }));

    add ("Saw Mean", make ([&] (float p)
    {
        return additive (p, 64, [] (int h)
        {
            float a = 1.0f / (float) h;
            if (h >= 6 && h <= 10)
                a *= 2.5f;
            return a;
        }, [] (int) { return 0.0f; });
    }));

    add ("Saw Retro", make ([&] (float p)
    {
        return additive (p, 16, [] (int h) { return 1.0f / (float) h; }, [] (int) { return 0.0f; });
    }));

    add ("Saw", make (saw));

    add ("Silk", make ([&] (float p)
    {
        return additive (p, 12, [] (int h) { return 1.0f / ((float) h * (float) h); },
                         [] (int) { return 0.0f; });
    }));

    add ("Sine", make ([&] (float p) { return std::sin (twoPi * p); }));

    add ("Smooth Sub", make ([&] (float p)
    {
        return std::sin (twoPi * p)
             + 0.3f * std::sin (twoPi * 2.0f * p)
             + 0.15f * std::sin (twoPi * 3.0f * p);
    }));

    add ("Square Retro", make ([&] (float p)
    {
        return additive (p, 15, [] (int h) { return (h % 2) ? 1.0f / (float) h : 0.0f; },
                         [] (int) { return 0.0f; });
    }));

    add ("Square Smooth", make ([&] (float p)
    {
        return additive (p, 63, [] (int h)
        {
            return (h % 2) ? 1.0f / std::pow ((float) h, 1.6f) : 0.0f;
        }, [] (int) { return 0.0f; });
    }));

    add ("Square", make (square));

    add ("Strat", make ([&] (float p) { return 0.6f * saw (p) + 0.4f * square (p); }));

    add ("String", make ([&] (float p)
    {
        return additive (p, 24, [] (int h) { return 1.0f / std::pow ((float) h, 1.1f); },
                         [] (int h) { return 0.02f * (float) h * (float) h; });
    }));

    add ("Throat", make ([&] (float p) { return formant (p, 3.0f, 7.0f); }));

    add ("Triangle", make (triangle));

    add ("Troll", make ([&] (float p)
    {
        return additive (p, 9, [] (int h)
        {
            return (h % 2) ? 1.0f / std::pow ((float) h, 1.5f) : 0.0f;
        }, [] (int) { return 0.0f; }) + 0.5f * std::sin (twoPi * p);
    }));

    add ("Vocal", make ([&] (float p) { return formant (p, 4.0f, 9.0f); }));

    return out;
}

void WavetableBank::generateFallback()
{
    tables.clear();
    tableNames.clear();

    for (auto& entry : makeOriginalTables())
        addTable (entry.first, std::move (entry.second));
}

void WavetableBank::load()
{
    juce::File dir (defaultWaveDir());
    if (! dir.isDirectory())
    {
        generateFallback();
        return;
    }

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();

    std::vector<juce::File> files;
    for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.wav"))
        files.push_back (f);

    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
    {
        return a.getFileName().compareIgnoreCase (b.getFileName()) < 0;
    });

    for (auto& file : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
        if (reader == nullptr)
            continue;

        const int len = (int) juce::jmin ((juce::int64) reader->lengthInSamples, (juce::int64) (1 << 20));
        if (len <= 0)
            continue;

        juce::AudioBuffer<float> buf ((int) reader->numChannels, len);
        reader->read (&buf, 0, len, 0, true, true);

        const int numCh = juce::jmax (1, buf.getNumChannels());
        std::vector<float> out ((size_t) kTableSize);
        for (int i = 0; i < kTableSize; ++i)
        {
            const float x = (float) i * (float) len / (float) kTableSize;
            const int i0 = juce::jlimit (0, len - 1, (int) std::floor (x));
            const int i1 = juce::jlimit (0, len - 1, i0 + 1);
            const float frac = x - std::floor (x);

            float sum = 0.0f;
            for (int c = 0; c < numCh; ++c)
                sum += buf.getSample (c, i0) * (1.0f - frac) + buf.getSample (c, i1) * frac;

            out[(size_t) i] = sum / (float) numCh;
        }

        addTable (file.getFileNameWithoutExtension(), std::move (out));
    }

    if (tables.empty())
        generateFallback();
}
