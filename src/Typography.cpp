#include "Typography.h"
#include <BinaryData.h>

namespace
{
juce::Font makeFont (const juce::Typeface::Ptr& typeface, float height, float kerning)
{
    auto opts = juce::FontOptions (typeface).withHeight (height);
    if (kerning != 0.0f)
        opts = opts.withKerningFactor (kerning);
    return juce::Font (opts);
}
}

juce::Typeface::Ptr Typography::display()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::ChakraPetchBold_ttf, BinaryData::ChakraPetchBold_ttfSize);
    return typeface;
}

juce::Typeface::Ptr Typography::semi()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::ChakraPetchSemiBold_ttf, BinaryData::ChakraPetchSemiBold_ttfSize);
    return typeface;
}

juce::Typeface::Ptr Typography::medium()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::ChakraPetchMedium_ttf, BinaryData::ChakraPetchMedium_ttfSize);
    return typeface;
}

juce::Typeface::Ptr Typography::mono()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::ShareTechMono_ttf, BinaryData::ShareTechMono_ttfSize);
    return typeface;
}

juce::Font Typography::displayFont (float height, float kerning)
{
    return makeFont (display(), height, kerning);
}

juce::Font Typography::semiFont (float height, float kerning)
{
    return makeFont (semi(), height, kerning);
}

juce::Font Typography::mediumFont (float height, float kerning)
{
    return makeFont (medium(), height, kerning);
}

juce::Font Typography::monoFont (float height, float kerning)
{
    return makeFont (mono(), height, kerning);
}
