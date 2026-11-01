#include "ArcWaveLookAndFeel.h"
#include "Theme.h"
#include "Typography.h"

#include <cmath>

ArcWaveLookAndFeel::ArcWaveLookAndFeel()
{
    accent = Theme::accent;
    accent2 = Theme::accent2;

    setColour (juce::Label::textColourId,                 Theme::dim);
    setColour (juce::Slider::textBoxTextColourId,         Theme::text);
    setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId,    Theme::accent.withAlpha (0.25f));
    setColour (juce::Slider::thumbColourId,               Theme::accent);

    setColour (juce::ComboBox::textColourId,              Theme::text);
    setColour (juce::ComboBox::backgroundColourId,        juce::Colours::transparentBlack);
    setColour (juce::ComboBox::outlineColourId,           juce::Colours::transparentBlack);
    setColour (juce::ComboBox::arrowColourId,             Theme::accent);
    setColour (juce::ComboBox::focusedOutlineColourId,    Theme::accent);

    setColour (juce::TextButton::textColourOffId,         Theme::text);
    setColour (juce::TextButton::textColourOnId,          Theme::ink);

    setColour (juce::PopupMenu::backgroundColourId,       Theme::panel);
    setColour (juce::PopupMenu::textColourId,             Theme::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::accent.withAlpha (0.30f));
    setColour (juce::PopupMenu::highlightedTextColourId,  Theme::ink);

    setColour (juce::ToggleButton::textColourId,          Theme::text);

    setColour (juce::AlertWindow::backgroundColourId,     Theme::panel);
    setColour (juce::AlertWindow::textColourId,           Theme::text);
    setColour (juce::AlertWindow::outlineColourId,        Theme::line);

    setColour (juce::TextEditor::backgroundColourId,      Theme::ink2);
    setColour (juce::TextEditor::textColourId,            Theme::text);
    setColour (juce::TextEditor::outlineColourId,         Theme::line);
    setColour (juce::TextEditor::focusedOutlineColourId,  Theme::accent);
}

juce::Font ArcWaveLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return Typography::semiFont (12.5f, 0.04f);
}

juce::Font ArcWaveLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return Typography::semiFont (13.5f, 0.02f);
}

juce::Label* ArcWaveLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (slider);
    l->setColour (juce::Label::textColourId, Theme::text);
    l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    l->setFont (Typography::monoFont (12.5f));
    l->setJustificationType (juce::Justification::centred);
    return l;
}

void ArcWaveLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    auto b = label.getLocalBounds().reduced (2);
    g.setColour (label.findColour (juce::Label::backgroundColourId));
    g.fillRoundedRectangle (b.toFloat(), 3.0f);
    g.setColour (label.findColour (juce::Label::textColourId)
                      .withMultipliedAlpha (label.isEnabled() ? 1.0f : 0.5f));
    g.setFont (label.getFont());
    g.drawFittedText (label.getText(), b, label.getJustificationType(), 1,
                      label.getMinimumHorizontalScale());
}

void ArcWaveLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                            juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float lineW = juce::jmax (2.4f, radius * 0.10f);
    const float arcR = radius - lineW * 1.7f;

    // Track
    juce::Path track;
    track.addCentredArc (centreX, centreY, arcR, arcR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xFF141A28));
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Ticks
    g.setColour (Theme::line.withAlpha (0.75f));
    for (int i = 0; i <= 10; ++i)
    {
        const float a = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (float) i / 10.0f;
        const float r1 = arcR + lineW * 0.8f;
        const float r2 = arcR + lineW * 1.45f;
        g.drawLine (centreX + std::sin (a) * r1, centreY - std::cos (a) * r1,
                    centreX + std::sin (a) * r2, centreY - std::cos (a) * r2, 1.0f);
    }

    if (sliderPos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (centreX, centreY, arcR, arcR, 0.0f, rotaryStartAngle, angle, true);

        g.setColour (Theme::accent.withAlpha (0.26f));
        g.strokePath (val, juce::PathStrokeType (lineW * 2.6f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Angular holographic ramp: lime -> aqua -> ultraviolet
        const int segments = 28;
        const float span = angle - rotaryStartAngle;
        for (int s = 0; s < segments; ++s)
        {
            const float f0 = (float) s / (float) segments;
            const float f1 = (float) (s + 1) / (float) segments;
            const float a0 = rotaryStartAngle + span * f0;
            const float a1 = rotaryStartAngle + span * f1;

            juce::Path seg;
            seg.addCentredArc (centreX, centreY, arcR, arcR, 0.0f, a0, a1, true);
            g.setColour (Theme::neonAt (f1));
            g.strokePath (seg, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        }

        // Hot-pink endpoint
        const float ex = centreX + std::sin (angle) * arcR;
        const float ey = centreY - std::cos (angle) * arcR;
        g.setColour (Theme::hot.withAlpha (0.35f));
        g.fillEllipse (ex - 4.0f, ey - 4.0f, 8.0f, 8.0f);
        g.setColour (Theme::hot);
        g.fillEllipse (ex - 1.8f, ey - 1.8f, 3.6f, 3.6f);
    }

    // Body
    auto body = juce::Rectangle<float> (centreX - arcR + lineW,
                                        centreY - arcR + lineW,
                                        (arcR - lineW) * 2.0f,
                                        (arcR - lineW) * 2.0f);

    juce::ColourGradient bodyGrad (juce::Colour (0xFF1A2233), centreX, centreY,
                                   juce::Colour (0xFF05080F), centreX + arcR, centreY + arcR, true);
    g.setGradientFill (bodyGrad);
    g.fillEllipse (body);

    g.setColour (juce::Colour (0xFF2A3350));
    g.drawEllipse (body, 1.0f);

    // Concentric detail rings
    g.setColour (Theme::accent.withAlpha (0.045f));
    for (float rr : { arcR * 0.36f, arcR * 0.58f, arcR * 0.78f })
        g.drawEllipse (centreX - rr, centreY - rr, rr * 2.0f, rr * 2.0f, 1.0f);

    // Glass highlight
    juce::Path highlight;
    highlight.addCentredArc (centreX, centreY, arcR * 0.82f, arcR * 0.82f, 0.0f, -2.55f, -0.75f, true);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.strokePath (highlight, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    // Centre hub
    g.setColour (Theme::line);
    g.fillEllipse (centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);
    g.setColour (Theme::accent.withAlpha (0.65f));
    g.fillEllipse (centreX - 1.3f, centreY - 1.3f, 2.6f, 2.6f);

    // Pointer
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.6f, -arcR + lineW * 1.9f, 3.2f, arcR * 0.50f, 1.6f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (Theme::hot);
    g.fillPath (pointer);
}

void ArcWaveLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                const juce::Colour&, bool highlighted, bool down)
{
    auto b = button.getLocalBounds().toFloat().reduced (0.5f);
    auto path = Theme::chamfer (b, juce::jmin (7.0f, b.getHeight() * 0.28f));
    const bool on = button.getToggleState();

    auto base = on ? Theme::accent : juce::Colour (0xFF111827);
    if (highlighted) base = base.brighter (0.12f);
    if (down)        base = base.darker (0.12f);

    juce::ColourGradient grad (base, b.getX(), b.getY(),
                               base.darker (0.40f), b.getX(), b.getBottom(), false);
    g.setGradientFill (grad);
    g.fillPath (path);

    g.setColour (on ? Theme::accent.brighter (0.15f) : Theme::line);
    g.strokePath (path, juce::PathStrokeType (1.0f));

    if (on)
    {
        g.setColour (Theme::hot);
        g.fillRect (b.getX() + 6.0f, b.getY() + 7.0f, 3.0f, b.getHeight() - 14.0f);
    }
}

void ArcWaveLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                            bool highlighted, bool)
{
    const bool on = button.getToggleState();
    auto lb = button.getLocalBounds().toFloat();
    const float pillW = juce::jmin (40.0f, lb.getWidth() * 0.42f);
    const float pillH = juce::jmin (20.0f, lb.getHeight() - 4.0f);
    juce::Rectangle<float> pill (lb.getX(), lb.getCentreY() - pillH * 0.5f, pillW, pillH);

    g.setColour (on ? Theme::accent : juce::Colour (0xFF151C2B));
    g.fillRoundedRectangle (pill, pillH * 0.5f);
    g.setColour (on ? Theme::accent.brighter (0.2f) : Theme::line);
    g.drawRoundedRectangle (pill.reduced (0.5f), pillH * 0.5f, 1.0f);

    const float d = pillH - 6.0f;
    const float cx = on ? pill.getRight() - d * 0.5f - 3.0f
                        : pill.getX() + d * 0.5f + 3.0f;
    g.setColour (on ? Theme::ink : Theme::text);
    g.fillEllipse (cx - d * 0.5f, pill.getCentreY() - d * 0.5f, d, d);

    if (highlighted)
    {
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRoundedRectangle (pill, pillH * 0.5f);
    }

    g.setColour (on ? Theme::accent : Theme::text.withAlpha (0.72f));
    const float fontH = juce::jlimit (8.0f, 11.0f, pillW * 0.28f);
    g.setFont (Typography::semiFont (fontH, 0.04f));
    auto textArea = button.getLocalBounds().withTrimmedLeft ((int) pillW + 6);
    g.drawText (button.getButtonText(), textArea, juce::Justification::centredLeft, false);
}

void ArcWaveLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&)
{
    auto b = juce::Rectangle<float> (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f);
    auto path = Theme::chamfer (b, 6.0f);

    juce::ColourGradient fill (Theme::ink2, 0.0f, 0.0f,
                               juce::Colour (0xFF05080F), 0.0f, (float) height, false);
    g.setGradientFill (fill);
    g.fillPath (path);

    g.setColour (isButtonDown ? Theme::accent : Theme::line);
    g.strokePath (path, juce::PathStrokeType (1.0f));

    const float cx = (float) buttonX + (float) buttonW * 0.5f;
    const float cy = (float) buttonY + (float) buttonH * 0.5f;
    juce::Path chevron;
    chevron.startNewSubPath (cx - 4.5f, cy - 2.0f);
    chevron.lineTo (cx, cy + 2.5f);
    chevron.lineTo (cx + 4.5f, cy - 2.0f);
    g.setColour (Theme::accent);
    g.strokePath (chevron, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

void ArcWaveLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (10, 1, box.getWidth() - 34, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}
