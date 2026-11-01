#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Theme
{
inline const juce::Colour accent  { 0xFFD6FF12 }; // electric lime
inline const juce::Colour mid     { 0xFF2EFFC8 }; // aqua-mint
inline const juce::Colour accent2 { 0xFF7A5CFF }; // ultraviolet
inline const juce::Colour hot     { 0xFFFF2E88 }; // hot pink
inline const juce::Colour ink     { 0xFF04060B };
inline const juce::Colour ink2    { 0xFF0A0F1C };
inline const juce::Colour panel   { 0xFF0C1220 };
inline const juce::Colour panel2  { 0xFF060A13 };
inline const juce::Colour line    { 0xFF2A3350 };
inline const juce::Colour text    { 0xFFE8F0FF };
inline const juce::Colour dim     { 0xFF7387A8 };

inline juce::Colour neonAt (float t)
{
    t = juce::jlimit (0.0f, 1.0f, t);
    if (t < 0.5f)
        return accent.interpolatedWith (mid, t * 2.0f);
    return mid.interpolatedWith (accent2, (t - 0.5f) * 2.0f);
}

inline juce::Path chamfer (juce::Rectangle<float> r, float cut)
{
    cut = juce::jmin (cut, juce::jmin (r.getWidth(), r.getHeight()) * 0.5f);

    juce::Path p;
    p.startNewSubPath (r.getX() + cut, r.getY());
    p.lineTo (r.getRight() - cut, r.getY());
    p.lineTo (r.getRight(), r.getY() + cut);
    p.lineTo (r.getRight(), r.getBottom() - cut);
    p.lineTo (r.getRight() - cut, r.getBottom());
    p.lineTo (r.getX() + cut, r.getBottom());
    p.lineTo (r.getX(), r.getBottom() - cut);
    p.lineTo (r.getX(), r.getY() + cut);
    p.closeSubPath();
    return p;
}
}
