/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// slope amount
enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// Data structure for all param values
struct ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibles{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    int lowCutSlope{ Slope::Slope_12 }, highCutSlope{ Slope::Slope_12 };
};

// Helper function to get all param values from ChainSettings
ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts);


//==============================================================================
/**
*/
class TokyoEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TokyoEQAudioProcessor();
    ~TokyoEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };


private:

    using Filter    = dsp::IIR::Filter<float>;
    using CutFilter = dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using Monochain = dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    Monochain leftChain, rightChain;

    enum ChainPositions
    {
        LowCut,
        Peak,
        HighCut
    };

    void updatePeakFilter(const ChainSettings& chainSettings);

    // unsure of dsp::IIR class, so I'll use namespace
    using Coefficients = Filter::CoefficientsPtr;

    // Doesn't contain mem vars so will make static instead of free
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TokyoEQAudioProcessor)
};
