#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"

namespace theme
{
const juce::Colour background  { 0xff14161b };
const juce::Colour panel       { 0xff1c1f26 };
const juce::Colour display     { 0xff101216 };
const juce::Colour grid        { 0xff262a33 };
const juce::Colour gridStrong  { 0xff343945 };
const juce::Colour text        { 0xffc9ced8 };
const juce::Colour textDim     { 0xff7d8493 };
const juce::Colour curve       { 0xfff2f4f8 };
const juce::Colour preSpectrum { 0xff4a5568 };
const juce::Colour postSpectrum{ 0xff5fb3d9 };

inline juce::Colour band (int i)
{
    static const juce::Colour c[] = { juce::Colour (0xffe05d5d), juce::Colour (0xffe8914a), juce::Colour (0xffe8c547),
                                      juce::Colour (0xff7ccf6a), juce::Colour (0xff4cc3b0), juce::Colour (0xff5b9be8),
                                      juce::Colour (0xff9a7be8), juce::Colour (0xffd86fc8) };
    return c[i % 8];
}
} // namespace theme

//==============================================================================
class KlangbildLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KlangbildLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
};

//==============================================================================
// Grafik: Raster, Analyzer, EQ-Kurve und ziehbare Band-Punkte
class ResponseDisplay : public juce::Component, private juce::Timer
{
public:
    explicit ResponseDisplay (KlangbildAudioProcessor&);
    ~ResponseDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseMove (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseExit (const juce::MouseEvent&) override;

    void setSelectedBand (int b) { selected = b; repaint(); }
    std::function<void (int)> onBandSelected;

private:
    void timerCallback() override;

    juce::Rectangle<float> plotArea() const;
    float freqToX (float f) const;
    float xToFreq (float x) const;
    float gainToY (float g) const;
    float yToGain (float y) const;
    float analyzerDbToY (float db) const;
    juce::Point<float> nodePosition (int band) const;
    int hitTestNode (juce::Point<float>) const;

    void setParam (const juce::String& id, float value);
    void updateCurves();
    void processSpectrum (std::vector<float>& smoothed, bool gotNewData);

    KlangbildAudioProcessor& proc;

    static constexpr int fftSize = SpectrumFifo::fftSize;
    juce::dsp::FFT fft { SpectrumFifo::fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, false };
    std::vector<float> fftData = std::vector<float> ((size_t) fftSize * 2, 0.0f);
    std::vector<float> preDb, postDb;      // geglättete Werte pro Pixelspalte
    std::vector<float> totalCurveDb;       // Summen-EQ-Kurve pro Pixelspalte
    std::vector<float> selectedCurveDb;    // Kurve des gewählten Bands

    int selected = 2, hovered = -1, dragging = -1;
    float dragQStart = 1.0f;
};

//==============================================================================
class KlangbildAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit KlangbildAudioProcessorEditor (KlangbildAudioProcessor&);
    ~KlangbildAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void selectBand (int band);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    KlangbildAudioProcessor& proc;
    KlangbildLookAndFeel lnf;

    ResponseDisplay display;
    std::array<juce::TextButton, eq::numBands> bandButtons;

    juce::Slider freqSlider, gainSlider, qSlider, outSlider;
    juce::Label freqLabel, gainLabel, qLabel, outLabel, slopeLabel, analyzerLabel, hintLabel;
    juce::ToggleButton onButton { "Band aktiv" };
    juce::ComboBox slopeBox, analyzerBox;

    std::unique_ptr<SliderAttachment> freqAtt, gainAtt, qAtt, outAtt;
    std::unique_ptr<ButtonAttachment> onAtt;
    std::unique_ptr<ComboAttachment>  slopeAtt, analyzerAtt;

    int selectedBand = 2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlangbildAudioProcessorEditor)
};
