#include "PluginEditor.h"
#include "Theme.h"
#include "Typography.h"

#include <cmath>

ArcWaveEditor::ArcWaveEditor (ArcWaveProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      presetManager (p.apvts),
      scopeView (p.getScopeBuffer())
{
    setLookAndFeel (&lookAndFeel);
    setOpaque (true);

    for (int i = 0; i < 3; ++i)
    {
        auto& c = oscCaptions[(size_t) i];
        c.setText ("OSC " + juce::String (i + 1), juce::dontSendNotification);
        c.setFont (Typography::displayFont (12.0f, 0.12f));
        c.setColour (juce::Label::textColourId, Theme::accent);
        c.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (c);
    }

    // Oscillators
    addChoice ("osc1Wave", 0);
    addChoice ("osc2Wave", 0);
    addChoice ("osc3Wave", 0);
    addKnob ("osc1Pitch", "PITCH", 0);
    addKnob ("osc1Detune", "DETUNE", 0);
    addKnob ("osc1Mix", "MIX", 0);
    addKnob ("osc2Pitch", "PITCH", 0);
    addKnob ("osc2Detune", "DETUNE", 0);
    addKnob ("osc2Mix", "MIX", 0);
    addKnob ("osc3Pitch", "PITCH", 0);
    addKnob ("osc3Detune", "DETUNE", 0);
    addKnob ("osc3Mix", "MIX", 0);
    addToggle ("osc2Invert", "INVERT", 0);

    for (int i = 0; i < 3; ++i)
    {
        wavePreviews[(size_t) i] = std::make_unique<WavePreview> (
            processor.apvts, "osc" + juce::String (i + 1) + "Wave");
        addAndMakeVisible (*wavePreviews[(size_t) i]);
    }

    // Voice
    addKnob ("octave", "OCTAVE", 0);
    addKnob ("unisonVoices", "UNISON", 0);
    addKnob ("unisonDetune", "DETUNE", 0);
    addKnob ("unisonStereo", "STEREO", 0);
    addKnob ("noiseMix", "NOISE", 0);
    addToggle ("monoVoice", "MONO", 0);

    // Modulation
    addChoice ("modKind", 0);
    addKnob ("modAmount", "MOD AMT", 0);
    addKnob ("pitchEnvAmount", "P.ENV", 0);
    addKnob ("pitchEnvDecay", "P.DECAY", 0);

    // Filter
    addChoice ("filterType", 1);
    addKnob ("filterCutoff", "CUTOFF", 1);
    addKnob ("filterResonance", "RESONANCE", 1);
    addKnob ("filterKeytrack", "KEYTRACK", 1);
    addKnob ("filterEnvAmount", "ENV AMT", 1);

    // Envelopes
    addKnob ("ampAttack", "ATTACK", 1);
    addKnob ("ampDecay", "DECAY", 1);
    addKnob ("ampSustain", "SUSTAIN", 1);
    addKnob ("ampRelease", "RELEASE", 1);
    addKnob ("filterAttack", "ATTACK", 1);
    addKnob ("filterDecay", "DECAY", 1);

    // LFO
    addKnob ("lfoRate", "RATE", 1);
    addKnob ("lfoAmount", "AMOUNT", 1);

    // FX
    addKnob ("fxDrive", "DRIVE", 2);
    addKnob ("fxCrusher", "CRUSHER", 2);
    addKnob ("fxDelayMix", "DLY MIX", 2);
    addKnob ("fxDelayTime", "DLY TIME", 2);
    addKnob ("fxDelayFeedback", "DLY FB", 2);
    addKnob ("fxReverbMix", "REV MIX", 2);
    addKnob ("fxReverbSize", "REV SIZE", 2);
    addKnob ("masterGain", "MASTER", 2);

    // Page buttons
    for (auto* b : { &pageSynth, &pageTone, &pageFx })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (1001);
        addAndMakeVisible (*b);
    }
    pageSynth.onClick = [this] { setPage (0); };
    pageTone.onClick  = [this] { setPage (1); };
    pageFx.onClick    = [this] { setPage (2); };
    pageSynth.setToggleState (true, juce::dontSendNotification);

    // Preset bar
    prevButton.onClick     = [this] { presetManager.previous(); refreshPresetList(); };
    nextButton.onClick     = [this] { presetManager.next(); refreshPresetList(); };
    randomButton.onClick   = [this] { presetManager.randomize(); refreshPresetList(); };
    saveButton.onClick     = [this] { showSaveDialog(); };
    for (auto* b : { &prevButton, &nextButton, &randomButton, &saveButton })
        addAndMakeVisible (*b);

    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0)
            presetManager.apply (idx);
    };
    addAndMakeVisible (presetBox);
    refreshPresetList();

    addAndMakeVisible (scopeView);

    aboutLabel.setText ("ArcWave  //  by Qua$$a\nWavetable Synthesizer  //  v0.3.0",
                        juce::dontSendNotification);
    aboutLabel.setJustificationType (juce::Justification::centred);
    aboutLabel.setColour (juce::Label::textColourId, Theme::dim);
    aboutLabel.setFont (Typography::mediumFont (12.5f, 0.02f));
    addAndMakeVisible (aboutLabel);

    levelMeter = std::make_unique<LevelMeter> (processor);
    addAndMakeVisible (*levelMeter);

    setSize (960, 600);
    setResizable (true, true);
    setResizeLimits (860, 520, 1400, 900);
    startTimerHz (30);
    setPage (0);
}

ArcWaveEditor::~ArcWaveEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

ArcWaveEditor::Knob& ArcWaveEditor::addKnob (const juce::String& id, const juce::String& caption, int page)
{
    auto k = std::make_unique<Knob>();
    k->page = page;

    k->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 68, 16);
    k->slider.setPopupDisplayEnabled (true, true, this);
    k->slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xFFE8F0FF));
    k->slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    k->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    k->slider.setColour (juce::Slider::textBoxHighlightColourId, lookAndFeel.accent.withAlpha (0.25f));
    addAndMakeVisible (k->slider);

    k->label.setText (caption, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setFont (Typography::mediumFont (10.5f, 0.08f));
    k->label.setColour (juce::Label::textColourId, Theme::dim);
    addAndMakeVisible (k->label);

    k->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, id, k->slider);

    auto* raw = k.get();
    knobs[id] = std::move (k);
    return *raw;
}

void ArcWaveEditor::addChoice (const juce::String& id, int page)
{
    auto c = std::make_unique<Choice>();
    c->page = page;
    c->box.setJustificationType (juce::Justification::centred);

    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (id)))
    {
        c->box.addItemList (p->getAllValueStrings(), 1);
        c->box.setSelectedItemIndex (p->getIndex(), juce::dontSendNotification);
    }

    addAndMakeVisible (c->box);

    c->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, id, c->box);

    choices[id] = std::move (c);
}

void ArcWaveEditor::addToggle (const juce::String& id, const juce::String& text, int page)
{
    auto t = std::make_unique<Toggle>();
    t->page = page;
    t->button.setButtonText (text);
    addAndMakeVisible (t->button);

    t->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, id, t->button);

    toggles[id] = std::move (t);
}

void ArcWaveEditor::setPage (int page)
{
    currentPage = juce::jlimit (0, 2, page);
    pageSynth.setToggleState (currentPage == 0, juce::dontSendNotification);
    pageTone.setToggleState (currentPage == 1, juce::dontSendNotification);
    pageFx.setToggleState (currentPage == 2, juce::dontSendNotification);
    resized();
    repaint();
}

void ArcWaveEditor::updateVisibility()
{
    for (auto& [id, k] : knobs)
    {
        k->slider.setVisible (k->page == currentPage);
        k->label.setVisible (k->page == currentPage);
    }

    for (auto& [id, c] : choices)
        c->box.setVisible (c->page == currentPage);

    for (auto& [id, t] : toggles)
        t->button.setVisible (t->page == currentPage);

    for (auto& c : oscCaptions)
        c.setVisible (currentPage == 0);

    for (auto& w : wavePreviews)
        if (w != nullptr)
            w->setVisible (currentPage == 0);

    if (levelMeter != nullptr)
        levelMeter->setVisible (currentPage == 2);

    aboutLabel.setVisible (currentPage == 2);
}

void ArcWaveEditor::refreshPresetList()
{
    presetBox.clear (juce::dontSendNotification);
    const auto names = presetManager.getNames();

    juce::String lastCategory;
    for (int i = 0; i < names.size(); ++i)
    {
        const auto category = presetManager.getCategory (i).toUpperCase();
        if (category != lastCategory)
        {
            presetBox.addSectionHeading (category);
            lastCategory = category;
        }

        presetBox.addItem (names[i], i + 1);
    }

    const int idx = presetManager.getCurrentIndex();
    if (idx >= 0 && idx < names.size())
    {
        presetBox.setSelectedId (idx + 1, juce::dontSendNotification);
        presetBox.setText (presetManager.getCategory (idx).toUpperCase() + "  //  " + names[idx],
                           juce::dontSendNotification);
    }
    else
    {
        presetBox.setText ("RANDOM", juce::dontSendNotification);
    }
}

void ArcWaveEditor::showSaveDialog()
{
    auto* aw = new juce::AlertWindow ("Save Preset", "Preset name:",
                                      juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor ("name", presetManager.getCurrentName());
    aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create ([this, aw] (int result)
    {
        if (result == 1)
        {
            const auto name = aw->getTextEditorContents ("name").trim();
            if (name.isNotEmpty())
            {
                presetManager.saveCurrent (name);
                refreshPresetList();
            }
        }
    }), true);
}

void ArcWaveEditor::timerCallback()
{
    scopeView.repaint();
    for (auto& w : wavePreviews)
        if (w != nullptr && w->isVisible())
            w->repaint();

    if (levelMeter != nullptr && levelMeter->isVisible())
        levelMeter->repaint();
}

void ArcWaveEditor::layoutKnob (Knob& k, juce::Rectangle<int> cell)
{
    auto c = cell.reduced (4, 2);
    k.label.setBounds (c.removeFromTop (14));
    k.slider.setBounds (c);
}

void ArcWaveEditor::layoutKnobRow (const juce::StringArray& ids, juce::Rectangle<int> area)
{
    const int n = ids.size();
    if (n <= 0)
        return;

    const int w = area.getWidth() / n;
    for (int i = 0; i < n; ++i)
    {
        auto cell = area.withX (area.getX() + i * w).withWidth (w);
        if (auto it = knobs.find (ids[i]); it != knobs.end())
            layoutKnob (*it->second, cell);
    }
}

void ArcWaveEditor::resized()
{
    auto b = getLocalBounds();

    b.removeFromTop (112);                 // header
    auto presetBar = b.removeFromTop (50);
    auto content = b.reduced (18, 10);

    // Page buttons (top-right)
    const int pw = 92, ph = 30, gap = 8;
    const int totalW = 3 * pw + 2 * gap;
    const int bx = getWidth() - 24 - totalW;
    pageSynth.setBounds (bx, 46, pw, ph);
    pageTone.setBounds (bx + pw + gap, 46, pw, ph);
    pageFx.setBounds (bx + 2 * (pw + gap), 46, pw, ph);

    const int scopeLeft = 360;
    const int scopeRight = bx - 20;
    scopeView.setBounds (scopeLeft, 16, juce::jmax (200, scopeRight - scopeLeft), 80);

    // Preset bar
    auto pb = presetBar.reduced (18, 6);
    prevButton.setBounds (pb.removeFromLeft (38));
    pb.removeFromLeft (6);
    presetBox.setBounds (pb.removeFromLeft (360).reduced (0, 3));
    pb.removeFromLeft (6);
    nextButton.setBounds (pb.removeFromLeft (38));
    pb.removeFromLeft (20);
    randomButton.setBounds (pb.removeFromLeft (128));
    pb.removeFromLeft (8);
    saveButton.setBounds (pb.removeFromLeft (92));

    panels.clear();

    if (currentPage == 0)
    {
        const int g = 14;
        auto area = content;
        auto left = area.removeFromLeft ((int) (area.getWidth() * 0.72f) - g / 2);
        area.removeFromLeft (g);
        auto right = area;

        auto voiceArea = right.removeFromTop ((int) (right.getHeight() * 0.58f) - g / 2);
        right.removeFromTop (g);
        auto modArea = right;

        panels.push_back ({ "OSCILLATORS", left });
        panels.push_back ({ "VOICE", voiceArea });
        panels.push_back ({ "MOD", modArea });

        auto osc = left.reduced (14);
        osc.removeFromTop (30);
        const int colW = osc.getWidth() / 3;

        for (int i = 0; i < 3; ++i)
        {
            auto col = osc.withX (osc.getX() + i * colW).withWidth (colW).reduced (8, 0);

            auto captionRow = col.removeFromTop (22);
            if (i == 1)
                if (auto it = toggles.find ("osc2Invert"); it != toggles.end())
                    it->second->button.setBounds (captionRow.removeFromRight (112));

            oscCaptions[(size_t) i].setBounds (captionRow);
            col.removeFromTop (4);

            const auto waveId = "osc" + juce::String (i + 1) + "Wave";
            if (auto it = choices.find (waveId); it != choices.end())
                it->second->box.setBounds (col.removeFromTop (28).reduced (10, 0));

            col.removeFromTop (10);

            juce::StringArray ids;
            ids.add ("osc" + juce::String (i + 1) + "Pitch");
            ids.add ("osc" + juce::String (i + 1) + "Detune");
            ids.add ("osc" + juce::String (i + 1) + "Mix");

            const int cellW = col.getWidth() / 3;
            const int knobH = juce::jmin (col.getHeight() - 30, juce::jmax (110, cellW + 34));
            layoutKnobRow (ids, col.removeFromTop (knobH));

            col.removeFromTop (8);
            if (wavePreviews[(size_t) i] != nullptr)
                wavePreviews[(size_t) i]->setBounds (col);
        }

        auto v = voiceArea.reduced (14);
        v.removeFromTop (30);

        const int cw = v.getWidth() / 3;
        const int ch = v.getHeight() / 2;
        auto cell = [v, cw, ch] (int cx, int cy)
        {
            return juce::Rectangle<int> (v.getX() + cx * cw, v.getY() + cy * ch, cw, ch).reduced (4, 2);
        };

        auto place = [this] (const juce::String& id, juce::Rectangle<int> area)
        {
            if (auto it = knobs.find (id); it != knobs.end())
                layoutKnob (*it->second, area);
        };

        place ("octave", cell (0, 0));
        place ("unisonVoices", cell (1, 0));
        place ("unisonDetune", cell (2, 0));
        place ("unisonStereo", cell (0, 1));
        place ("noiseMix", cell (1, 1));

        if (auto it = toggles.find ("monoVoice"); it != toggles.end())
            it->second->button.setBounds (cell (2, 1).reduced (4, ch / 2 - 15));

        auto m = modArea.reduced (14);
        m.removeFromTop (30);

        if (auto it = choices.find ("modKind"); it != choices.end())
        {
            it->second->box.setBounds (m.removeFromTop (28).reduced (16, 0));
            m.removeFromTop (6);
        }

        const int modCellW = m.getWidth() / 3;
        const int modKnobH = juce::jmin (m.getHeight(), juce::jmax (80, modCellW + 30));
        layoutKnobRow ({ "modAmount", "pitchEnvAmount", "pitchEnvDecay" }, m.removeFromTop (modKnobH));
    }
    else if (currentPage == 1)
    {
        const int g = 14;
        auto area = content;
        const int topH = (int) (area.getHeight() * 0.55f) - g / 2;
        auto top = area.removeFromTop (topH);
        area.removeFromTop (g);
        auto bottom = area;

        auto tL = top.removeFromLeft ((int) (top.getWidth() * 0.55f) - g / 2);
        top.removeFromLeft (g);
        auto tR = top;
        auto bL = bottom.removeFromLeft ((int) (bottom.getWidth() * 0.42f) - g / 2);
        bottom.removeFromLeft (g);
        auto bR = bottom;

        panels.push_back ({ "FILTER", tL });
        panels.push_back ({ "AMP ENVELOPE", tR });
        panels.push_back ({ "FILTER ENVELOPE", bL });
        panels.push_back ({ "LFO", bR });

        auto f = tL.reduced (14);
        f.removeFromTop (30);
        if (auto it = choices.find ("filterType"); it != choices.end())
        {
            it->second->box.setBounds (f.removeFromTop (30).reduced (30, 0));
            f.removeFromTop (6);
        }
        layoutKnobRow ({ "filterCutoff", "filterResonance", "filterKeytrack", "filterEnvAmount" }, f);

        auto a = tR.reduced (14);
        a.removeFromTop (30);
        layoutKnobRow ({ "ampAttack", "ampDecay", "ampSustain", "ampRelease" }, a);

        auto fe = bL.reduced (14);
        fe.removeFromTop (30);
        layoutKnobRow ({ "filterAttack", "filterDecay" }, fe);

        auto l = bR.reduced (14);
        l.removeFromTop (30);
        layoutKnobRow ({ "lfoRate", "lfoAmount" }, l);
    }
    else
    {
        const int g = 14;
        auto area = content;
        auto left = area.removeFromLeft ((int) (area.getWidth() * 0.70f) - g / 2);
        area.removeFromLeft (g);
        auto right = area;

        panels.push_back ({ "FX CHAIN", left });
        panels.push_back ({ "OUTPUT", right });

        auto fx = left.reduced (14);
        fx.removeFromTop (30);
        const int half = fx.getHeight() / 2;
        layoutKnobRow ({ "fxDrive", "fxCrusher", "fxDelayMix", "fxDelayTime" }, fx.removeFromTop (half));
        layoutKnobRow ({ "fxDelayFeedback", "fxReverbMix", "fxReverbSize" }, fx);

        auto out = right.reduced (14);
        out.removeFromTop (30);
        layoutKnobRow ({ "masterGain" }, out.removeFromTop ((int) (out.getHeight() * 0.55f)));
        aboutLabel.setBounds (out.removeFromTop (60));

        if (levelMeter != nullptr)
            levelMeter->setBounds (out.reduced (10, 8));
    }

    updateVisibility();
}

void ArcWaveEditor::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float W = b.getWidth();
    const float H = b.getHeight();

    juce::ColourGradient bg (Theme::ink, 0.0f, 0.0f, Theme::ink2, 0.0f, H, false);
    g.setGradientFill (bg);
    g.fillRect (b);

    // Diagonal hatch
    g.setColour (Theme::accent.withAlpha (0.018f));
    for (float x = -H; x < W; x += 22.0f)
        g.drawLine (x, H, x + H, 0.0f, 1.0f);

    // Ambient glows
    juce::ColourGradient glow1 (Theme::accent.withAlpha (0.13f), W * 0.10f, 40.0f,
                                juce::Colours::transparentBlack, W * 0.10f, 470.0f, true);
    g.setGradientFill (glow1);
    g.fillRect (b);

    juce::ColourGradient glow2 (Theme::accent2.withAlpha (0.16f), W * 0.92f, H - 30.0f,
                                juce::Colours::transparentBlack, W * 0.92f, H - 540.0f, true);
    g.setGradientFill (glow2);
    g.fillRect (b);

    juce::ColourGradient glow3 (Theme::hot.withAlpha (0.06f), W * 0.55f, H * 0.55f,
                                juce::Colours::transparentBlack, W * 0.55f, H * 0.55f + 260.0f, true);
    g.setGradientFill (glow3);
    g.fillRect (b);

    // Vignette
    juce::ColourGradient vignette (juce::Colours::transparentBlack, W * 0.5f, H * 0.45f,
                                   juce::Colour (0x88000000), W * 0.5f, H * 0.45f + H * 0.75f, true);
    g.setGradientFill (vignette);
    g.fillRect (b);

    // Wordmark
    {
        auto font = Typography::displayFont (40.0f, 0.10f);
        juce::GlyphArrangement ga;
        ga.addFittedText (font, "ARCWAVE", 24.0f, 8.0f, 360.0f, 52.0f,
                          juce::Justification::centredLeft, 1);
        juce::Path tp;
        ga.createPath (tp);

        g.setColour (Theme::hot.withAlpha (0.30f));
        g.fillPath (tp, juce::AffineTransform::translation (2.5f, 2.5f));

        const auto tb = tp.getBounds();
        const float midX = tb.getCentreX();

        g.saveState();
        g.reduceClipRegion (juce::Rectangle<float> (tb.getX() - 2.0f, tb.getY() - 2.0f,
                                                    midX - tb.getX() + 2.0f, tb.getHeight() + 4.0f).toNearestInt());
        g.setGradientFill (juce::ColourGradient (Theme::accent, tb.getX(), tb.getY(),
                                                 Theme::mid, midX, tb.getBottom(), false));
        g.fillPath (tp);
        g.restoreState();

        g.saveState();
        g.reduceClipRegion (juce::Rectangle<float> (midX, tb.getY() - 2.0f,
                                                    tb.getRight() - midX + 2.0f, tb.getHeight() + 4.0f).toNearestInt());
        g.setGradientFill (juce::ColourGradient (Theme::mid, midX, tb.getY(),
                                                 Theme::accent2, tb.getRight(), tb.getBottom(), false));
        g.fillPath (tp);
        g.restoreState();
    }

    // Accent rule
    g.setColour (Theme::hot);
    g.fillRect (26.0f, 62.0f, 118.0f, 2.5f);
    g.setColour (Theme::accent.withAlpha (0.6f));
    g.fillRect (150.0f, 62.0f, 40.0f, 2.5f);

    // Tagline
    g.setColour (Theme::text);
    g.setFont (Typography::semiFont (15.0f, 0.06f));
    g.drawText ("by Qua$$a", 26, 68, 260, 20, juce::Justification::centredLeft, false);

    g.setColour (Theme::dim);
    g.setFont (Typography::monoFont (9.5f, 0.12f));
    g.drawText ("WAVETABLE SYNTHESIZER  //  WT-01", 26, 89, 340, 14,
                juce::Justification::centredLeft, false);

    // Header divider
    g.setColour (Theme::line.withAlpha (0.8f));
    g.drawHorizontalLine (111, 20.0f, W - 20.0f);
    g.setColour (Theme::accent.withAlpha (0.5f));
    g.drawHorizontalLine (111, 20.0f, 160.0f);

    // Panels
    int idx = 1;
    for (auto& p : panels)
    {
        auto rf = p.bounds.toFloat();
        auto path = Theme::chamfer (rf, 12.0f);

        juce::ColourGradient fill (Theme::panel, rf.getX(), rf.getY(),
                                   Theme::panel2, rf.getX(), rf.getBottom(), false);
        g.setGradientFill (fill);
        g.fillPath (path);

        g.setColour (Theme::accent.withAlpha (0.16f));
        g.drawLine (rf.getX() + 16.0f, rf.getY() + 1.0f, rf.getRight() - 16.0f, rf.getY() + 1.0f, 1.0f);

        g.setColour (Theme::line);
        g.strokePath (path, juce::PathStrokeType (1.0f));

        g.setColour (Theme::hot.withAlpha (0.9f));
        g.setFont (Typography::monoFont (9.5f, 0.08f));
        g.drawText (juce::String (idx).paddedLeft ('0', 2),
                    p.bounds.reduced (14, 6).removeFromTop (16),
                    juce::Justification::centredRight, false);

        g.setColour (Theme::accent);
        g.setFont (Typography::displayFont (11.5f, 0.14f));
        g.drawText (p.title, p.bounds.reduced (16, 8).removeFromTop (16),
                    juce::Justification::centredLeft, false);

        g.setColour (Theme::hot.withAlpha (0.8f));
        g.fillRect ((float) p.bounds.getX() + 16.0f, (float) p.bounds.getY() + 29.0f, 22.0f, 2.0f);

        ++idx;
    }
}

void ArcWaveEditor::LevelMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    if (b.getHeight() < 30.0f)
        return;

    auto labelArea = b.removeFromTop (14.0f);
    const float peak = processor.getOutputPeak();
    const float db = juce::Decibels::gainToDecibels (peak, -60.0f);

    g.setColour (Theme::dim);
    g.setFont (Typography::monoFont (9.5f, 0.10f));
    g.drawText ("OUTPUT LEVEL", labelArea.toNearestInt(), juce::Justification::centredLeft, false);

    g.setColour (Theme::accent);
    g.setFont (Typography::monoFont (10.0f));
    g.drawText (juce::String (db, 1) + " dB", labelArea.toNearestInt(),
                juce::Justification::centredRight, false);

    auto bar = b.removeFromTop (18.0f).reduced (0.0f, 2.0f);
    auto bgPath = Theme::chamfer (bar, 3.0f);
    g.setColour (Theme::ink2);
    g.fillPath (bgPath);

    const int segs = 40;
    const float gap = 1.5f;
    const float w = (bar.getWidth() - (float) (segs - 1) * gap) / (float) segs;
    const float level = juce::jlimit (0.0f, 1.0f, std::sqrt (peak));

    for (int i = 0; i < segs; ++i)
    {
        const float t = (float) (i + 1) / (float) segs;
        juce::Rectangle<float> seg (bar.getX() + (float) i * (w + gap),
                                    bar.getY() + 2.0f, w, bar.getHeight() - 4.0f);
        g.setColour (t <= level ? Theme::neonAt (t) : Theme::line.withAlpha (0.35f));
        g.fillRect (seg);
    }

    if (level > 0.001f)
    {
        const float x = bar.getX() + level * bar.getWidth();
        g.setColour (Theme::hot);
        g.fillRect (x - 1.0f, bar.getY(), 2.0f, bar.getHeight());
    }

    auto scale = b.removeFromTop (12.0f);
    g.setColour (Theme::dim.withAlpha (0.7f));
    g.setFont (Typography::monoFont (8.0f, 0.05f));
    g.drawText ("-48        -24        -12     -6    0",
                scale.toNearestInt(), juce::Justification::centredLeft, false);
}

void ArcWaveEditor::WavePreview::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    if (b.getHeight() < 20.0f)
        return;

    g.setColour (Theme::panel.withAlpha (0.72f));
    g.fillRoundedRectangle (b, 8.0f);
    g.setColour (Theme::line.withAlpha (0.7f));
    g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

    int idx = 0;
    if (auto* p = state.getRawParameterValue (paramId))
        idx = (int) p->load();

    const auto& tbl = WavetableBank::getInstance().table (idx);
    if (tbl.size() < 2)
        return;

    const auto& accent = Theme::accent;
    const auto& accent2 = Theme::accent2;
    const int n = (int) tbl.size();

    std::vector<juce::Point<float>> pts;
    pts.reserve ((size_t) n);
    juce::Path path;
    for (int i = 0; i < n; ++i)
    {
        const float x = b.getX() + 8.0f + (float) i / (float) (n - 1) * (b.getWidth() - 16.0f);
        const float y = b.getCentreY()
                        - juce::jlimit (-1.0f, 1.0f, tbl[(size_t) i]) * (b.getHeight() * 0.30f);
        pts.push_back ({ x, y });
        if (i == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    juce::Path fill (path);
    fill.lineTo (b.getRight() - 8.0f, b.getBottom() - 6.0f);
    fill.lineTo (b.getX() + 8.0f, b.getBottom() - 6.0f);
    fill.closeSubPath();
    g.setColour (accent.withAlpha (0.10f));
    g.fillPath (fill);

    const int chunks = 48;
    for (int c = 0; c < chunks; ++c)
    {
        const int i0 = (int) ((float) c / (float) chunks * (float) (n - 1));
        const int i1 = (int) ((float) (c + 1) / (float) chunks * (float) (n - 1));
        if (i1 <= i0)
            continue;

        juce::Path q;
        q.startNewSubPath (pts[(size_t) i0]);
        for (int i = i0 + 1; i <= i1; ++i)
            q.lineTo (pts[(size_t) i]);

        g.setColour (Theme::neonAt ((float) (c + 1) / (float) chunks));
        g.strokePath (q, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    const auto& names = WavetableBank::getInstance().names();
    const auto label = (idx >= 0 && idx < (int) names.size()) ? names[(size_t) idx] : juce::String();
    g.setColour (Theme::dim);
    g.setFont (Typography::monoFont (10.0f, 0.06f));
    g.drawText (label, b.toNearestInt().reduced (8, 4).removeFromTop (14),
                juce::Justification::centredLeft, false);
}

void ArcWaveEditor::ScopeView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    g.setColour (Theme::panel.withAlpha (0.72f));
    g.fillRoundedRectangle (b, 10.0f);
    g.setColour (Theme::line.withAlpha (0.7f));
    g.drawRoundedRectangle (b.reduced (0.5f), 10.0f, 1.0f);

    const int n = buffer.read (samples, ScopeBuffer::kSize);
    if (n < 2)
        return;

    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawHorizontalLine ((int) b.getCentreY(), b.getX() + 8.0f, b.getRight() - 8.0f);

    std::vector<juce::Point<float>> pts;
    pts.reserve ((size_t) n);
    juce::Path p;
    for (int i = 0; i < n; ++i)
    {
        const float x = b.getX() + 6.0f + (float) i / (float) (n - 1) * (b.getWidth() - 12.0f);
        const float y = b.getCentreY()
                        - juce::jlimit (-1.0f, 1.0f, samples[i]) * (b.getHeight() * 0.44f);
        pts.push_back ({ x, y });
        if (i == 0)
            p.startNewSubPath (x, y);
        else
            p.lineTo (x, y);
    }

    g.setColour (Theme::accent.withAlpha (0.28f));
    g.strokePath (p, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const int chunks = 64;
    for (int c = 0; c < chunks; ++c)
    {
        const int i0 = (int) ((float) c / (float) chunks * (float) (n - 1));
        const int i1 = (int) ((float) (c + 1) / (float) chunks * (float) (n - 1));
        if (i1 <= i0)
            continue;

        juce::Path q;
        q.startNewSubPath (pts[(size_t) i0]);
        for (int i = i0 + 1; i <= i1; ++i)
            q.lineTo (pts[(size_t) i]);

        g.setColour (Theme::neonAt ((float) (c + 1) / (float) chunks));
        g.strokePath (q, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }
}
