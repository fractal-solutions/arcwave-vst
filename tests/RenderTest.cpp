// Headless runtime test: constructs the processor, applies every factory
// preset, renders a note offline, and checks the output is non-silent.

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <cmath>
#include <fstream>
#include <iostream>

namespace
{
void writeWav16 (const juce::File& file, const juce::AudioBuffer<float>& buf, double sampleRate)
{
    const int numCh = juce::jmin (2, buf.getNumChannels());
    const int numFrames = buf.getNumSamples();
    const int bits = 16;
    const int dataSize = numFrames * numCh * (bits / 8);

    file.deleteFile();
    std::ofstream out (file.getFullPathName().toStdString(), std::ios::binary);
    auto w32 = [&out] (juce::uint32 v) { out.write (reinterpret_cast<const char*> (&v), 4); };
    auto w16 = [&out] (juce::uint16 v) { out.write (reinterpret_cast<const char*> (&v), 2); };
    auto tag = [&out] (const char* t) { out.write (t, 4); };

    tag ("RIFF"); w32 ((juce::uint32) (36 + dataSize)); tag ("WAVE");
    tag ("fmt "); w32 (16); w16 (1); w16 ((juce::uint16) numCh);
    w32 ((juce::uint32) sampleRate); w32 ((juce::uint32) (sampleRate * numCh * (bits / 8)));
    w16 ((juce::uint16) (numCh * (bits / 8))); w16 (bits);
    tag ("data"); w32 ((juce::uint32) dataSize);

    for (int i = 0; i < numFrames; ++i)
        for (int c = 0; c < numCh; ++c)
        {
            const float v = juce::jlimit (-1.0f, 1.0f, buf.getSample (c, i));
            w16 ((juce::uint16) (juce::int16) std::lround (v * 32767.0f));
        }
}

juce::File findRepoRoot()
{
    auto dir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();

    for (int i = 0; i < 8; ++i)
    {
        if (dir.getChildFile ("CMakeLists.txt").existsAsFile() && dir.getChildFile ("src").isDirectory())
            return dir;

        auto parent = dir.getParentDirectory();
        if (parent == dir)
            break;
        dir = parent;
    }

    return juce::File::getCurrentWorkingDirectory();
}

float renderNote (ArcWaveProcessor& proc, double seconds, int note, juce::AudioBuffer<float>* outBuf)
{
    const int total = (int) (48000.0 * seconds);
    juce::AudioBuffer<float> out (2, total);
    out.clear();

    int pos = 0;
    while (pos < total)
    {
        const int n = juce::jmin (512, total - pos);
        juce::AudioBuffer<float> block (2, n);
        block.clear();

        juce::MidiBuffer midi;
        if (pos == 0)
            midi.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), 0);

        proc.processBlock (block, midi);
        for (int c = 0; c < 2; ++c)
            out.copyFrom (c, pos, block, c, 0, n);

        pos += n;
    }

    float peak = 0.0f;
    for (int c = 0; c < 2; ++c)
    {
        const auto* r = out.getReadPointer (c);
        for (int i = 0; i < total; ++i)
            peak = juce::jmax (peak, std::abs (r[i]));
    }

    if (outBuf != nullptr)
        *outBuf = std::move (out);

    return peak;
}
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;

    const auto root = findRepoRoot();
    const auto outDir = root.getChildFile ("out");
    outDir.createDirectory();

    ArcWaveProcessor proc;
    proc.prepareToPlay (48000.0, 512);

    PresetManager presets (proc.apvts);
    const auto names = presets.getNames();

    int failures = 0;
    std::cout << "Factory presets: " << presets.getNumPresets() << "\n";

    for (int i = 0; i < presets.getNumPresets(); ++i)
    {
        presets.apply (i);
        proc.reset();
        const float peak = renderNote (proc, 1.4, 48, nullptr);
        std::cout << "  [" << i << "] " << presets.getCurrentName()
                  << "  peak=" << peak << "\n";

        if (peak <= 0.0005f)
            ++failures;
    }

    const int demoIndex = names.indexOf ("Prism Lead");
    if (demoIndex >= 0)
    {
        presets.apply (demoIndex);
        proc.reset();
        juce::AudioBuffer<float> demo;
        renderNote (proc, 2.2, 60, &demo);
        const juce::File wav (outDir.getChildFile ("preset-prism-lead.wav"));
        writeWav16 (wav, demo, 48000.0);
        std::cout << "Wrote demo preset -> " << wav.getFullPathName() << "\n";
    }

    // Write the original (license-free) wavetable pack used when no GMS
    // installation is present.
    {
        const auto originals = WavetableBank::makeOriginalTables();
        const juce::File dir (root.getChildFile ("assets").getChildFile ("wavetables-original"));
        dir.createDirectory();

        for (auto& entry : originals)
        {
            juce::AudioBuffer<float> buf (1, (int) entry.second.size());
            buf.copyFrom (0, 0, entry.second.data(), (int) entry.second.size());
            writeWav16 (dir.getChildFile (entry.first + ".wav"), buf, 44100.0);
        }

        std::cout << "Wrote original wavetable pack: " << originals.size() << " tables\n";
    }

#if JUCE_WINDOWS
    // Render the editor to PNG so the UI can be inspected without a host.
    {
        ArcWaveEditor editor (proc);
        editor.setSize (960, 600);
        editor.setVisible (true);

        for (int page = 0; page < 3; ++page)
        {
            proc.reset();
            renderNote (proc, 0.35, 48, nullptr);
            editor.showPage (page);
            const auto image = editor.createComponentSnapshot (editor.getLocalBounds(), true, 1.0f);
            const juce::File png (outDir.getChildFile ("ui-page-" + juce::String (page) + ".png"));
            png.deleteFile();
            std::unique_ptr<juce::FileOutputStream> stream (png.createOutputStream());
            if (stream != nullptr)
            {
                juce::PNGImageFormat fmt;
                fmt.writeImageToStream (image, *stream);
                stream->flush();
                std::cout << "Wrote UI snapshot -> " << png.getFullPathName() << "\n";
            }
        }
    }
#endif

    std::cout << "failures=" << failures << "\n";
    return failures == 0 ? 0 : 2;
}
