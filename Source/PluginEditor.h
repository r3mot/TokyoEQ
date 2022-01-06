/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Base class initialization for sliders
struct CustomRotarySlider : Slider
{
    CustomRotarySlider() : Slider(Slider::SliderStyle::RotaryHorizontalDrag, Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};


struct ResponseCurveComponent : Component,
    AudioProcessorParameter::Listener,
    Timer
{
    ResponseCurveComponent(TokyoEQAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };
    void timerCallback() override;

    void paint(Graphics& g) override;
    void resized() override;

private:
    TokyoEQAudioProcessor& audioProcessor;
    Atomic<bool> parametersChanged{ false };
    MonoChain monoChain;
};
//==============================================================================
/**
*/
class TokyoEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TokyoEQAudioProcessorEditor (TokyoEQAudioProcessor&);
    ~TokyoEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TokyoEQAudioProcessor& audioProcessor;

    // Sliders
    CustomRotarySlider peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;

    ResponseCurveComponent responseCurveComponent;


    using APVTS = AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment peakFreqSliderAttachment,
        peakGainSliderAttachment,
        peakQualitySliderAttachment,
        lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        highCutSlopeSliderAttachment;

    std::vector<Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TokyoEQAudioProcessorEditor)
};
