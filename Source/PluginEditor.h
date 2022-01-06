/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Base class initialization for sliders
struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalDrag, 
                                            juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
private:
 
};


struct ResponseCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    ResponseCurveComponent(TokyoEQAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };
    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    TokyoEQAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };
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
    void paint (juce::Graphics&) override;
    void resized() override;


private:
  
    TokyoEQAudioProcessor& audioProcessor;

    RotarySliderWithLabels peakFreqSlider, // sliders
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;

    ResponseCurveComponent responseCurveComponent;

    using APVTS = juce::AudioProcessorValueTreeState;
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
