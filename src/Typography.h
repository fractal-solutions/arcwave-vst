#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Typography
{
juce::Typeface::Ptr display();
juce::Typeface::Ptr semi();
juce::Typeface::Ptr medium();
juce::Typeface::Ptr mono();

juce::Font displayFont (float height, float kerning = 0.0f);
juce::Font semiFont (float height, float kerning = 0.0f);
juce::Font mediumFont (float height, float kerning = 0.0f);
juce::Font monoFont (float height, float kerning = 0.0f);
}
