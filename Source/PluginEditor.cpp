/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TokyoEQAudioProcessorEditor::TokyoEQAudioProcessorEditor (TokyoEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    for ( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    setSize (600, 400); // Window Size
}

TokyoEQAudioProcessorEditor::~TokyoEQAudioProcessorEditor()
{
}

//==============================================================================
void TokyoEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void TokyoEQAudioProcessorEditor::resized()
{
    // component positions
    auto bounds         = getLocalBounds();

    auto responseArea   = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto lowCutArea     = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea    = bounds.removeFromRight(bounds.getWidth() * 0.5);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);



}

std::vector<Component*> TokyoEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}