#include "PluginEditor.h"

namespace
{
constexpr float minFreq = 20.0f, maxFreq = 20000.0f;
constexpr float eqRangeDb = 24.0f;
constexpr float analyzerTopDb = 0.0f, analyzerBottomDb = -90.0f;
constexpr float tiltDbPerOct = 3.0f;   // Neigung, damit Musik/Rosa Rauschen eher flach erscheint
}

//==============================================================================
KlangbildLookAndFeel::KlangbildLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, theme::background);
    setColour (juce::Label::textColourId, theme::textDim);
    setColour (juce::Slider::textBoxTextColourId, theme::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, theme::postSpectrum.withAlpha (0.4f));
    setColour (juce::ComboBox::backgroundColourId, theme::display);
    setColour (juce::ComboBox::outlineColourId, theme::gridStrong);
    setColour (juce::ComboBox::textColourId, theme::text);
    setColour (juce::ComboBox::arrowColourId, theme::textDim);
    setColour (juce::PopupMenu::backgroundColourId, theme::panel);
    setColour (juce::PopupMenu::textColourId, theme::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::gridStrong);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::textColourOffId, theme::textDim);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ToggleButton::textColourId, theme::text);
    setColour (juce::TextEditor::backgroundColourId, theme::display);
    setColour (juce::TextEditor::textColourId, theme::text);
    setColour (juce::CaretComponent::caretColourId, theme::text);
}

void KlangbildLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                             float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float lineW = 4.0f, arcR = r - lineW * 0.5f;

    auto colour = slider.findColour (juce::Slider::rotarySliderFillColourId);
    if (! slider.isEnabled())
        colour = theme::textDim.withAlpha (0.4f);

    juce::Path bg;
    bg.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (theme::gridStrong);
    g.strokePath (bg, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    float fromAngle = startAngle;
    if (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
        fromAngle = startAngle + (float) slider.valueToProportionOfLength (0.0) * (endAngle - startAngle);
    const float toAngle = startAngle + pos * (endAngle - startAngle);

    if (std::abs (toAngle - fromAngle) > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, juce::jmin (fromAngle, toAngle), juce::jmax (fromAngle, toAngle), true);
        g.setColour (colour);
        g.strokePath (val, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float knobR = r - lineW - 4.0f;
    g.setColour (theme::panel.brighter (0.25f));
    g.fillEllipse (juce::Rectangle<float> (knobR * 2, knobR * 2).withCentre (c));
    g.setColour (theme::gridStrong.brighter (0.2f));
    g.drawEllipse (juce::Rectangle<float> (knobR * 2, knobR * 2).withCentre (c), 1.0f);

    const juce::Point<float> tip = c.getPointOnCircumference (knobR - 3.0f, toAngle);
    const juce::Point<float> inner = c.getPointOnCircumference (knobR * 0.35f, toAngle);
    g.setColour (slider.isEnabled() ? theme::curve : theme::textDim);
    g.drawLine ({ inner, tip }, 2.0f);
}

void KlangbildLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                                 bool over, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const auto accent = b.findColour (juce::TextButton::buttonOnColourId);
    const bool on = b.getToggleState();

    g.setColour (on ? accent.withAlpha (0.22f) : theme::display.brighter (over ? 0.08f : 0.0f).darker (down ? 0.2f : 0.0f));
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (on ? accent : theme::gridStrong);
    g.drawRoundedRectangle (r, 5.0f, on ? 1.5f : 1.0f);

    // kleiner Farbpunkt links
    g.setColour (accent.withAlpha (on ? 1.0f : 0.6f));
    g.fillEllipse (r.getX() + 8.0f, r.getCentreY() - 3.0f, 6.0f, 6.0f);
}

void KlangbildLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setColour (b.findColour (b.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId));
    g.setFont (juce::FontOptions (13.0f, b.getToggleState() ? juce::Font::bold : juce::Font::plain));
    g.drawFittedText (b.getButtonText(), b.getLocalBounds().withTrimmedLeft (18).withTrimmedRight (4),
                      juce::Justification::centred, 1, 0.8f);
}

void KlangbildLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    const auto r = b.getLocalBounds().toFloat();
    const auto sw = juce::Rectangle<float> (34.0f, 18.0f).withY (r.getCentreY() - 9.0f).withX (r.getX() + 2.0f);
    const bool on = b.getToggleState();
    const auto accent = b.findColour (juce::ToggleButton::tickColourId);

    g.setColour (on ? accent.withAlpha (0.85f) : theme::gridStrong.brighter (over ? 0.15f : 0.0f));
    g.fillRoundedRectangle (sw, 9.0f);
    g.setColour (juce::Colours::white);
    g.fillEllipse (juce::Rectangle<float> (14.0f, 14.0f).withCentre ({ on ? sw.getRight() - 9.0f : sw.getX() + 9.0f, sw.getCentreY() }));

    g.setColour (b.findColour (juce::ToggleButton::textColourId));
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (sw.getRight() + 8.0f), juce::Justification::centredLeft);
}

//==============================================================================
ResponseDisplay::ResponseDisplay (KlangbildAudioProcessor& p) : proc (p)
{
    setOpaque (false);
    startTimerHz (30);
}

ResponseDisplay::~ResponseDisplay() { stopTimer(); }

juce::Rectangle<float> ResponseDisplay::plotArea() const
{
    return getLocalBounds().toFloat().withTrimmedLeft (8).withTrimmedRight (36).withTrimmedTop (10).withTrimmedBottom (22);
}

float ResponseDisplay::freqToX (float f) const
{
    const auto a = plotArea();
    return a.getX() + a.getWidth() * std::log (f / minFreq) / std::log (maxFreq / minFreq);
}

float ResponseDisplay::xToFreq (float x) const
{
    const auto a = plotArea();
    return minFreq * std::pow (maxFreq / minFreq, (x - a.getX()) / a.getWidth());
}

float ResponseDisplay::gainToY (float g) const
{
    const auto a = plotArea();
    return a.getCentreY() - (g / eqRangeDb) * a.getHeight() * 0.5f;
}

float ResponseDisplay::yToGain (float y) const
{
    const auto a = plotArea();
    return (a.getCentreY() - y) / (a.getHeight() * 0.5f) * eqRangeDb;
}

float ResponseDisplay::analyzerDbToY (float db) const
{
    const auto a = plotArea();
    return juce::jmap (db, analyzerBottomDb, analyzerTopDb, a.getBottom(), a.getY());
}

juce::Point<float> ResponseDisplay::nodePosition (int band) const
{
    const auto s = proc.getBandSettings (band);
    const bool cut = eq::isCut (eq::typeForBand (band));
    return { freqToX (s.freq), gainToY (cut ? 0.0f : juce::jlimit (-eqRangeDb, eqRangeDb, s.gain)) };
}

int ResponseDisplay::hitTestNode (juce::Point<float> p) const
{
    // das ausgewählte Band hat Vorrang, falls Punkte übereinander liegen
    if (selected >= 0 && nodePosition (selected).getDistanceFrom (p) < 12.0f)
        return selected;
    int best = -1;
    float bestDist = 12.0f;
    for (int b = 0; b < eq::numBands; ++b)
    {
        const float d = nodePosition (b).getDistanceFrom (p);
        if (d < bestDist) { bestDist = d; best = b; }
    }
    return best;
}

void ResponseDisplay::setParam (const juce::String& id, float value)
{
    if (auto* p = proc.apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

//==============================================================================
void ResponseDisplay::resized()
{
    const auto n = (size_t) juce::jmax (2, (int) plotArea().getWidth() + 1);
    preDb.assign (n, -120.0f);
    postDb.assign (n, -120.0f);
    totalCurveDb.assign (n, 0.0f);
    selectedCurveDb.assign (n, 0.0f);
    updateCurves();
}

void ResponseDisplay::updateCurves()
{
    const double sr = proc.currentSampleRate.load();
    const auto a = plotArea();
    const size_t n = totalCurveDb.size();

    std::array<eq::BandCoeffs, eq::numBands> bc;
    std::array<bool, eq::numBands> on {};
    for (int b = 0; b < eq::numBands; ++b)
    {
        const auto s = proc.getBandSettings (b);
        on[(size_t) b] = s.on;
        bc[(size_t) b] = eq::makeBand (eq::typeForBand (b), sr, s.freq, s.gain, s.q, s.slope);
    }

    for (size_t i = 0; i < n; ++i)
    {
        const double f = xToFreq (a.getX() + (float) i);
        double total = 0.0;
        for (int b = 0; b < eq::numBands; ++b)
        {
            if (! on[(size_t) b] && b != selected)
                continue;
            const double db = f < sr * 0.5 ? eq::magnitudeDb (bc[(size_t) b], f, sr) : -120.0;
            if (on[(size_t) b]) total += db;
            if (b == selected)  selectedCurveDb[i] = (float) db;
        }
        totalCurveDb[i] = (float) total;
    }
}

void ResponseDisplay::processSpectrum (std::vector<float>& smoothed, bool gotNewData)
{
    if (! gotNewData)
    {
        for (auto& v : smoothed)
            v = juce::jmax (-120.0f, v - 1.5f);   // langsam ausblenden, wenn kein Audio kommt
        return;
    }

    const double sr = proc.currentSampleRate.load();
    window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data(), true);

    const float norm = 4.0f / (float) fftSize;   // Hann-Fenster (0.5) + einseitiges Spektrum
    const int maxBin = fftSize / 2;
    const auto a = plotArea();

    for (size_t i = 0; i < smoothed.size(); ++i)
    {
        const double f0 = xToFreq (a.getX() + (float) i - 0.5f);
        const double f1 = xToFreq (a.getX() + (float) i + 0.5f);
        const double b0 = f0 * fftSize / sr, b1 = f1 * fftSize / sr;

        float mag = 0.0f;
        if (b1 - b0 < 1.0)
        {
            const double b = juce::jlimit (0.0, (double) maxBin - 1.0, 0.5 * (b0 + b1));
            const int idx = (int) b;
            const float frac = (float) (b - idx);
            mag = fftData[(size_t) idx] * (1.0f - frac) + fftData[(size_t) idx + 1] * frac;
        }
        else
        {
            for (int k = juce::jmax (0, (int) b0); k <= juce::jmin ((int) std::ceil (b1), maxBin); ++k)
                mag = juce::jmax (mag, fftData[(size_t) k]);
        }

        const double fc = std::sqrt (f0 * f1);
        float db = juce::Decibels::gainToDecibels (mag * norm, -120.0f)
                 + tiltDbPerOct * (float) std::log2 (fc / 1000.0);
        if (fc >= sr * 0.5)
            db = -120.0f;

        auto& v = smoothed[i];
        v += (db - v) * (db > v ? 0.6f : 0.12f);   // schneller Anstieg, langsamer Abfall
    }
}

void ResponseDisplay::timerCallback()
{
    const int mode = (int) proc.apvts.getRawParameterValue ("analyzer")->load();
    const bool wantPre  = mode == KlangbildAudioProcessor::analyzerPre  || mode == KlangbildAudioProcessor::analyzerBoth;
    const bool wantPost = mode == KlangbildAudioProcessor::analyzerPost || mode == KlangbildAudioProcessor::analyzerBoth;

    if (wantPre)  processSpectrum (preDb,  proc.preFifo.pull (fftData.data()));
    else          std::fill (preDb.begin(), preDb.end(), -120.0f);

    if (wantPost) processSpectrum (postDb, proc.postFifo.pull (fftData.data()));
    else          std::fill (postDb.begin(), postDb.end(), -120.0f);

    updateCurves();
    repaint();
}

//==============================================================================
void ResponseDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto a = plotArea();

    g.setColour (theme::display);
    g.fillRoundedRectangle (bounds, 8.0f);

    // --- Raster ---------------------------------------------------------
    g.setFont (juce::FontOptions (11.0f));
    const int freqLines[] = { 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900,
                                1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    for (int f : freqLines)
    {
        const bool strong = (f == 100 || f == 1000 || f == 10000);
        g.setColour (strong ? theme::gridStrong : theme::grid);
        g.drawVerticalLine (juce::roundToInt (freqToX ((float) f)), a.getY(), a.getBottom());
    }

    const std::pair<float, const char*> freqLabels[] = { { 50, "50" }, { 100, "100" }, { 200, "200" }, { 500, "500" },
                                                         { 1000, "1k" }, { 2000, "2k" }, { 5000, "5k" }, { 10000, "10k" } };
    g.setColour (theme::textDim);
    for (auto& [f, txt] : freqLabels)
        g.drawText (txt, juce::Rectangle<float> (40.0f, 16.0f).withCentre ({ freqToX (f), a.getBottom() + 11.0f }),
                    juce::Justification::centred);

    for (int db = -18; db <= 18; db += 6)
    {
        const float y = gainToY ((float) db);
        g.setColour (db == 0 ? theme::gridStrong.brighter (0.3f) : theme::grid);
        g.drawHorizontalLine (juce::roundToInt (y), a.getX(), a.getRight());
        g.setColour (theme::textDim);
        g.drawText ((db > 0 ? "+" : "") + juce::String (db), juce::Rectangle<float> (a.getRight() + 2.0f, y - 8.0f, 32.0f, 16.0f),
                    juce::Justification::centredLeft);
    }

    juce::Graphics::ScopedSaveState clip (g);
    g.reduceClipRegion (a.toNearestInt());

    // --- Analyzer -------------------------------------------------------
    auto spectrumPath = [&] (const std::vector<float>& data, bool closed)
    {
        juce::Path p;
        for (size_t i = 0; i < data.size(); ++i)
        {
            const float x = a.getX() + (float) i;
            const float y = juce::jlimit (a.getY(), a.getBottom() + 2.0f, analyzerDbToY (data[i]));
            if (i == 0) p.startNewSubPath (x, closed ? a.getBottom() + 2.0f : y);
            if (i == 0 && closed) p.lineTo (x, y);
            else if (i > 0) p.lineTo (x, y);
        }
        if (closed)
        {
            p.lineTo (a.getRight(), a.getBottom() + 2.0f);
            p.closeSubPath();
        }
        return p;
    };

    if (! preDb.empty())
    {
        g.setColour (theme::preSpectrum.withAlpha (0.45f));
        g.fillPath (spectrumPath (preDb, true));
    }
    if (! postDb.empty())
    {
        g.setGradientFill (juce::ColourGradient (theme::postSpectrum.withAlpha (0.28f), 0, a.getY(),
                                                 theme::postSpectrum.withAlpha (0.04f), 0, a.getBottom(), false));
        g.fillPath (spectrumPath (postDb, true));
        g.setColour (theme::postSpectrum.withAlpha (0.85f));
        g.strokePath (spectrumPath (postDb, false), juce::PathStrokeType (1.2f));
    }

    // --- Kurve des gewählten Bands ---------------------------------------
    const float zeroY = gainToY (0.0f);
    if (selected >= 0 && ! selectedCurveDb.empty())
    {
        const bool on = proc.getBandSettings (selected).on;
        juce::Path p;
        p.startNewSubPath (a.getX(), zeroY);
        for (size_t i = 0; i < selectedCurveDb.size(); ++i)
            p.lineTo (a.getX() + (float) i, juce::jlimit (a.getY() - 2.0f, a.getBottom() + 2.0f, gainToY (selectedCurveDb[i])));
        p.lineTo (a.getRight(), zeroY);
        p.closeSubPath();
        const auto col = theme::band (selected);
        g.setColour (col.withAlpha (on ? 0.18f : 0.07f));
        g.fillPath (p);
        g.setColour (col.withAlpha (on ? 0.7f : 0.3f));
        g.strokePath (p, juce::PathStrokeType (1.0f));
    }

    // --- Summenkurve ------------------------------------------------------
    {
        juce::Path p;
        for (size_t i = 0; i < totalCurveDb.size(); ++i)
        {
            const float y = juce::jlimit (a.getY() - 2.0f, a.getBottom() + 2.0f, gainToY (totalCurveDb[i]));
            if (i == 0) p.startNewSubPath (a.getX(), y);
            else        p.lineTo (a.getX() + (float) i, y);
        }
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.strokePath (p, juce::PathStrokeType (4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (theme::curve);
        g.strokePath (p, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // --- Punkte -----------------------------------------------------------
    for (int pass = 0; pass < 2; ++pass)   // gewähltes Band zuletzt zeichnen
    {
        for (int b = 0; b < eq::numBands; ++b)
        {
            if ((pass == 0) == (b == selected))
                continue;
            const auto pos = nodePosition (b);
            const bool on = proc.getBandSettings (b).on;
            const bool hi = (b == selected || b == hovered || b == dragging);
            const float r = hi ? 9.5f : 8.0f;
            const auto col = theme::band (b);
            const auto circle = juce::Rectangle<float> (r * 2, r * 2).withCentre (pos);

            if (on)
            {
                g.setColour (col.withAlpha (0.9f));
                g.fillEllipse (circle);
            }
            else
            {
                g.setColour (theme::display.withAlpha (0.85f));
                g.fillEllipse (circle);
                g.setColour (col.withAlpha (0.55f));
                g.drawEllipse (circle, 1.5f);
            }
            if (b == selected)
            {
                g.setColour (juce::Colours::white);
                g.drawEllipse (circle.expanded (2.5f), 1.5f);
            }
            g.setColour (on ? juce::Colours::black.withAlpha (0.8f) : col.withAlpha (0.8f));
            g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
            g.drawText (juce::String (b + 1), circle, juce::Justification::centred);
        }
    }

    // --- Info-Box beim Ziehen / Überfahren ---------------------------------
    const int infoBand = dragging >= 0 ? dragging : hovered;
    if (infoBand >= 0)
    {
        auto txt = [&] (const char* n) { return proc.apvts.getParameter (KlangbildAudioProcessor::paramID (infoBand, n))->getCurrentValueAsText(); };
        const bool cut = eq::isCut (eq::typeForBand (infoBand));
        juce::String line = txt ("freq");
        if (! cut) line << "   " << txt ("gain");
        line << "   Q " << txt ("q");
        if (cut)   line << "   " << txt ("slope");

        const auto title = KlangbildAudioProcessor::bandNames()[infoBand]
                         + (proc.getBandSettings (infoBand).on ? juce::String() : juce::String (" (aus)"));
        const float w = juce::jmax (150.0f, 7.0f * (float) line.length() + 16.0f);
        auto pos = nodePosition (infoBand);
        auto box = juce::Rectangle<float> (w, 40.0f).withCentre ({ pos.x, pos.y - 38.0f });
        if (box.getY() < a.getY() + 2.0f) box.setY (pos.y + 18.0f);
        box = box.constrainedWithin (a.reduced (2.0f));

        g.setColour (theme::panel.withAlpha (0.95f));
        g.fillRoundedRectangle (box, 5.0f);
        g.setColour (theme::band (infoBand));
        g.drawRoundedRectangle (box, 5.0f, 1.0f);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (title, box.withHeight (20.0f).withTrimmedTop (3.0f), juce::Justification::centred);
        g.setColour (theme::text);
        g.setFont (juce::FontOptions (12.0f));
        g.drawText (line, box.withTrimmedTop (18.0f).withTrimmedBottom (3.0f), juce::Justification::centred);
    }
}

//==============================================================================
void ResponseDisplay::mouseMove (const juce::MouseEvent& e)
{
    const int h = hitTestNode (e.position);
    if (h != hovered)
    {
        hovered = h;
        setMouseCursor (h >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void ResponseDisplay::mouseExit (const juce::MouseEvent&)
{
    hovered = -1;
    repaint();
}

void ResponseDisplay::mouseDown (const juce::MouseEvent& e)
{
    dragging = hitTestNode (e.position);
    if (dragging < 0)
        return;

    selected = dragging;
    if (onBandSelected)
        onBandSelected (dragging);

    for (auto* n : { "freq", "gain" })
        if (auto* p = proc.apvts.getParameter (KlangbildAudioProcessor::paramID (dragging, n)))
            p->beginChangeGesture();
}

void ResponseDisplay::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging < 0 || e.getDistanceFromDragStart() < 2)
        return;

    // Ein ausgeschaltetes Band wird beim Ziehen automatisch eingeschaltet
    if (! proc.getBandSettings (dragging).on)
        setParam (KlangbildAudioProcessor::paramID (dragging, "on"), 1.0f);

    const auto a = plotArea();
    const float x = juce::jlimit (a.getX(), a.getRight(), e.position.x);
    setParam (KlangbildAudioProcessor::paramID (dragging, "freq"), juce::jlimit (minFreq, maxFreq, xToFreq (x)));

    if (! eq::isCut (eq::typeForBand (dragging)))
        setParam (KlangbildAudioProcessor::paramID (dragging, "gain"),
                  juce::jlimit (-eqRangeDb, eqRangeDb, yToGain (e.position.y)));
    repaint();
}

void ResponseDisplay::mouseUp (const juce::MouseEvent&)
{
    if (dragging >= 0)
        for (auto* n : { "freq", "gain" })
            if (auto* p = proc.apvts.getParameter (KlangbildAudioProcessor::paramID (dragging, n)))
                p->endChangeGesture();
    dragging = -1;
}

void ResponseDisplay::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int h = hitTestNode (e.position);
    if (h < 0)
        return;
    if (auto* p = proc.apvts.getParameter (KlangbildAudioProcessor::paramID (h, "on")))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->getValue() > 0.5f ? 0.0f : 1.0f);
        p->endChangeGesture();
    }
}

void ResponseDisplay::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    const int h = hitTestNode (e.position);
    const int band = h >= 0 ? h : selected;
    if (band < 0)
        return;

    if (auto* p = proc.apvts.getParameter (KlangbildAudioProcessor::paramID (band, "q")))
    {
        const float q = proc.getBandSettings (band).q;
        const float delta = wheel.isReversed ? -wheel.deltaY : wheel.deltaY;
        const float newQ = juce::jlimit (0.1f, 18.0f, q * std::pow (2.0f, delta * 2.0f));
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (newQ));
        p->endChangeGesture();
    }
}

//==============================================================================
KlangbildAudioProcessorEditor::KlangbildAudioProcessorEditor (KlangbildAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), display (p)
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (display);
    display.onBandSelected = [this] (int b) { selectBand (b); };

    const auto names = KlangbildAudioProcessor::bandNames();
    for (int b = 0; b < eq::numBands; ++b)
    {
        auto& btn = bandButtons[(size_t) b];
        btn.setButtonText (names[b]);
        btn.setColour (juce::TextButton::buttonOnColourId, theme::band (b));
        btn.onClick = [this, b] { selectBand (b); };
        addAndMakeVisible (btn);
    }

    auto setupSlider = [this] (juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 18);
        s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        addAndMakeVisible (s);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        addAndMakeVisible (l);
    };
    setupSlider (freqSlider, freqLabel, "FREQ");
    setupSlider (gainSlider, gainLabel, "GAIN");
    setupSlider (qSlider,    qLabel,    "Q");
    setupSlider (outSlider,  outLabel,  "OUTPUT");
    outSlider.setColour (juce::Slider::rotarySliderFillColourId, theme::curve);

    addAndMakeVisible (onButton);

    slopeBox.addItemList ({ "12 dB/Okt", "24 dB/Okt", "36 dB/Okt", "48 dB/Okt" }, 1);
    addAndMakeVisible (slopeBox);
    slopeLabel.setText ("FLANKE", juce::dontSendNotification);
    slopeLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    addAndMakeVisible (slopeLabel);

    analyzerBox.addItemList ({ "Aus", "Pre", "Post", "Pre + Post" }, 1);
    addAndMakeVisible (analyzerBox);
    analyzerLabel.setText ("ANALYZER", juce::dontSendNotification);
    analyzerLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    analyzerLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (analyzerLabel);

    hintLabel.setText (juce::CharPointer_UTF8 ("Ziehen: Freq/Gain  \xc2\xb7  Mausrad: Q  \xc2\xb7  Doppelklick: an/aus"),
                       juce::dontSendNotification);
    hintLabel.setFont (juce::FontOptions (11.0f));
    hintLabel.setJustificationType (juce::Justification::bottomRight);
    hintLabel.setMinimumHorizontalScale (1.0f);
    addAndMakeVisible (hintLabel);

    outAtt      = std::make_unique<SliderAttachment> (proc.apvts, "output", outSlider);
    analyzerAtt = std::make_unique<ComboAttachment>  (proc.apvts, "analyzer", analyzerBox);
    outSlider.setDoubleClickReturnValue (true, 0.0);

    selectBand (2);

    setResizable (true, true);
    setResizeLimits (760, 480, 1800, 1100);
    setSize (980, 600);
}

KlangbildAudioProcessorEditor::~KlangbildAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void KlangbildAudioProcessorEditor::selectBand (int band)
{
    selectedBand = band;
    display.setSelectedBand (band);

    for (int b = 0; b < eq::numBands; ++b)
        bandButtons[(size_t) b].setToggleState (b == band, juce::dontSendNotification);

    freqAtt.reset(); gainAtt.reset(); qAtt.reset(); onAtt.reset(); slopeAtt.reset();

    auto id = [band] (const char* n) { return KlangbildAudioProcessor::paramID (band, n); };
    freqAtt = std::make_unique<SliderAttachment> (proc.apvts, id ("freq"), freqSlider);
    gainAtt = std::make_unique<SliderAttachment> (proc.apvts, id ("gain"), gainSlider);
    qAtt    = std::make_unique<SliderAttachment> (proc.apvts, id ("q"),    qSlider);
    onAtt   = std::make_unique<ButtonAttachment> (proc.apvts, id ("on"),   onButton);

    for (auto* s : { &freqSlider, &gainSlider, &qSlider })
    {
        s->setColour (juce::Slider::rotarySliderFillColourId, theme::band (band));
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (proc.apvts.getParameter (
                s == &freqSlider ? id ("freq") : s == &gainSlider ? id ("gain") : id ("q"))))
            s->setDoubleClickReturnValue (true, rp->convertFrom0to1 (rp->getDefaultValue()));
    }
    onButton.setColour (juce::ToggleButton::tickColourId, theme::band (band));

    const bool cut = eq::isCut (eq::typeForBand (band));
    gainSlider.setEnabled (! cut);
    gainLabel.setAlpha (cut ? 0.4f : 1.0f);
    slopeBox.setVisible (cut);
    slopeLabel.setVisible (cut);
    if (cut)
        slopeAtt = std::make_unique<ComboAttachment> (proc.apvts, id ("slope"), slopeBox);

    repaint();
}

void KlangbildAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (theme::background);

    auto header = getLocalBounds().removeFromTop (44).reduced (16, 0);
    g.setColour (theme::curve);
    g.setFont (juce::FontOptions (19.0f, juce::Font::bold));
    g.drawText ("KLANGBILD", header, juce::Justification::centredLeft);
    const int w = juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (19.0f, juce::Font::bold), "KLANGBILD");
    g.setColour (theme::postSpectrum);
    g.setFont (juce::FontOptions (19.0f));
    g.drawText ("EQ", header.withTrimmedLeft (w + 8), juce::Justification::centredLeft);

    auto bottom = getLocalBounds().removeFromBottom (160).reduced (12, 8).toFloat();
    g.setColour (theme::panel);
    g.fillRoundedRectangle (bottom, 8.0f);
}

void KlangbildAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    auto header = area.removeFromTop (44).reduced (16, 9);
    analyzerBox.setBounds (header.removeFromRight (130));
    analyzerLabel.setBounds (header.removeFromRight (90).withTrimmedRight (8));

    auto bottom = area.removeFromBottom (160).reduced (12, 8).reduced (12, 10);
    display.setBounds (area.reduced (12, 0));

    auto outCol = bottom.removeFromRight (100);
    outLabel.setBounds (outCol.removeFromTop (16));
    outSlider.setBounds (outCol);

    bottom.removeFromRight (16);
    auto buttonRow = bottom.removeFromTop (28);
    const int bw = (buttonRow.getWidth() - 4 * (eq::numBands - 1)) / eq::numBands;
    for (int b = 0; b < eq::numBands; ++b)
    {
        bandButtons[(size_t) b].setBounds (buttonRow.removeFromLeft (bw));
        buttonRow.removeFromLeft (4);
    }

    bottom.removeFromTop (8);
    auto knob = [&] (juce::Slider& s, juce::Label& l)
    {
        auto col = bottom.removeFromLeft (100);
        l.setBounds (col.removeFromTop (16));
        s.setBounds (col);
    };
    knob (freqSlider, freqLabel);
    knob (gainSlider, gainLabel);
    knob (qSlider, qLabel);

    bottom.removeFromLeft (24);
    auto ctl = bottom.removeFromLeft (180);
    onButton.setBounds (ctl.removeFromTop (28).withTrimmedTop (4));
    ctl.removeFromTop (6);
    slopeLabel.setBounds (ctl.removeFromTop (16));
    slopeBox.setBounds (ctl.removeFromTop (26).withWidth (130));

    bottom.removeFromLeft (16);
    hintLabel.setBounds (bottom.removeFromBottom (20));
}
