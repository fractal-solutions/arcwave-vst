#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

// Wait-free single-producer / single-consumer ring buffer for the UI scope.
class ScopeBuffer
{
public:
    static constexpr int kSize = 2048;

    void pushSample (float v) noexcept
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite (1, start1, size1, start2, size2);

        if (size1 > 0)
            data[(size_t) start1] = v;
        else if (size2 > 0)
            data[(size_t) start2] = v;

        fifo.finishedWrite (size1 + size2);
    }

    int read (float* dest, int maxSamples) noexcept
    {
        int start1, size1, start2, size2;
        fifo.prepareToRead (maxSamples, start1, size1, start2, size2);

        for (int i = 0; i < size1; ++i)
            dest[i] = data[(size_t) (start1 + i)];

        for (int i = 0; i < size2; ++i)
            dest[size1 + i] = data[(size_t) (start2 + i)];

        fifo.finishedRead (size1 + size2);
        return size1 + size2;
    }

private:
    juce::AbstractFifo fifo { kSize };
    std::array<float, kSize> data {};
};
