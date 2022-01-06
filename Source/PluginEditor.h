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
class TokyoEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TokyoEQAudioProcessorEditor (TokyoEQAudioProcessor&);
    ~TokyoEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
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

    std::vector<Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TokyoEQAudioProcessorEditor)
};
