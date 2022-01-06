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

//==============================================================================
/**
*/
class TokyoEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
    AudioProcessorParameter::Listener, // needs thread safety and efficiency
    Timer
{
public:
    TokyoEQAudioProcessorEditor (TokyoEQAudioProcessor&);
    ~TokyoEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TokyoEQAudioProcessor& audioProcessor;

    // Atomic flag
    Atomic<bool> parametersChanged{ false };


    // Sliders
    CustomRotarySlider peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;


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

    MonoChain monoChain;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TokyoEQAudioProcessorEditor)
};
