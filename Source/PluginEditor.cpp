/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics & g,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    float sliderPosProportional,
                                    float rotaryStartAngle,
                                    float rotaryEndAngle,
                                    juce::Slider& slider)
{
    using namespace juce;
    auto bounds = Rectangle<float>(x, y, width, height);
    
    g.setColour(Colour(244u, 236u, 227u)); // base
    g.fillEllipse(bounds);

    g.setColour(Colour(20u, 21u, 24u)); // border
    g.drawEllipse(bounds, 1.f);

    auto center = bounds.getCentre();

    Path p; // define path for rotation
    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());
    p.addRectangle(r);

    jassert(rotaryStartAngle < rotaryEndAngle); // smoothing

    // mapping normalized starting value between two rotary pos
    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
    g.fillPath(p);

    //Draw 

}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f); 
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();
    auto sliderBounds = getSliderBounds();

    // Draw slider & local bounds
    g.setColour(Colours::blue);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::red);
    g.drawRect(sliderBounds);


    getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng, endAng, *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
   // return getLocalBounds();
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2; // square around sliders
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2); // 2 pixels below top of component

    return r;
}

//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(TokyoEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colours::transparentBlack);

    auto responseArea = getLocalBounds();
    auto width = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> magnitudes;

    // One magnitude per pixel
    magnitudes.resize(width);

    // Compute magnitude at frequency 
    for (int freqItr = 0; freqItr < width; ++freqItr)
    {
        double mag = 1.f;
        auto freq = juce::mapToLog10(double(freqItr) / double(width), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        magnitudes[freqItr] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(magnitudes.front()));

    for (size_t otherMag = 1; otherMag < magnitudes.size(); ++otherMag)
    {
        responseCurve.lineTo(responseArea.getX() + otherMag, map(magnitudes[otherMag]));
        g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
        g.setColour(Colours::white);
        g.strokePath(responseCurve, PathStrokeType(2.f));
    }

}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        DBG("params changed");

        //update monochain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
        //signal repaint for new response curve
        repaint();
    }
}

//==============================================================================
TokyoEQAudioProcessorEditor::TokyoEQAudioProcessorEditor(TokyoEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    peakFreqSlider      (*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider      (*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider   (*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider    (*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider   (*audioProcessor.apvts.getParameter("HiCut Freq"), "Hz"),
    lowCutSlopeSlider   (*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider  (*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),


    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts,      "Peak Freq",        peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts,      "Peak Gain",        peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts,   "Peak Quality",     peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts,    "LowCut Freq",      lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts,   "HighCut Freq",     highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts,   "LowCut Slope",     lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts,  "HighCut Slope",    highCutSlopeSlider)
    
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    setSize(600, 400); // Window Size
}

TokyoEQAudioProcessorEditor::~TokyoEQAudioProcessorEditor()
{
}

////==============================================================================
void TokyoEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightslategrey);
}

void TokyoEQAudioProcessorEditor::resized()
{
    // component positions
    auto bounds         = getLocalBounds();

    auto responseArea   = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto lowCutArea     = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea    = bounds.removeFromRight(bounds.getWidth() * 0.5);

    responseCurveComponent.setBounds(responseArea);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> TokyoEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}