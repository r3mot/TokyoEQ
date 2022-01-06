/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TokyoEQAudioProcessor::TokyoEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TokyoEQAudioProcessor::~TokyoEQAudioProcessor()
{
}

//==============================================================================
const juce::String TokyoEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TokyoEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TokyoEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TokyoEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TokyoEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TokyoEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TokyoEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TokyoEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TokyoEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void TokyoEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TokyoEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    // produce coefficients
    auto chainSettings = getChainSettings(apvts);
    updatePeakFilter(chainSettings);
    /*auto peakCoefficients = dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                          chainSettings.peakFreq, 
                                                                          chainSettings.peakQuality, 
                                                                          Decibels::decibelsToGain(chainSettings.peakGainInDecibles));

    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;*/

    auto cutCoefficients = dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 
                                                                                                2 * (chainSettings.lowCutSlope + 1)); // 2,4,6,8

    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    // Assigning coeff to the first filter in CutFilter chain & stop bypassing
    switch ( chainSettings.lowCutSlope)
    {
        case Slope_12:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            break;
        }

    }

    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope)
    {
        case Slope_12:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
            break;
        }

    }

}

void TokyoEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TokyoEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TokyoEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // TO:DO - refactor
    auto chainSettings = getChainSettings(apvts);
    updatePeakFilter(chainSettings);
  /*  auto peakCoefficients = dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
        chainSettings.peakFreq,
        chainSettings.peakQuality,
        Decibels::decibelsToGain(chainSettings.peakGainInDecibles));

    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;*/
    

    auto cutCoefficients = dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(),
        2 * (chainSettings.lowCutSlope + 1)); // 2,4,6,8

    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    // Assigning coeff to the first filter in CutFilter chain & stop bypassing
    switch (chainSettings.lowCutSlope)
    {
    case Slope_12:
    {
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        break;
    }
    case Slope_24:
    {
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        break;
    }
    case Slope_36:
    {
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        break;
    }
    case Slope_48:
    {
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
        leftLowCut.setBypassed<3>(false);
        break;
    }

    }

    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope)
    {
    case Slope_12:
    {
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        break;
    }
    case Slope_24:
    {
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        break;
    }
    case Slope_36:
    {
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        break;
    }
    case Slope_48:
    {
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
        rightLowCut.setBypassed<3>(false);
        break;
    }

    }
    // End TO:DO



    // Audio blocks for each channel
    dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

}

//==============================================================================
bool TokyoEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TokyoEQAudioProcessor::createEditor()
{
   // return new TokyoEQAudioProcessorEditor (*this);
    return new GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TokyoEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TokyoEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts) // init params
{
    
    ChainSettings settings;
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibles = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    return settings;
}

void TokyoEQAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings)
{
    auto peakCoefficients = dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
        chainSettings.peakFreq,
        chainSettings.peakQuality,
        Decibels::decibelsToGain(chainSettings.peakGainInDecibles));

    /**leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;*/
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

}

void TokyoEQAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}
AudioProcessorValueTreeState::ParameterLayout TokyoEQAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    //Deals with range slider is sensitive to changing
    float lowCutSkew = 0.25f;
    float HighCutSkew = 0.25f;
    float PeakFreqSkew = 0.25f;
    float PeakGainSkew = 0.25f;
    float PeakQualitySkew = 0.25f;


    layout.add(std::make_unique<AudioParameterFloat>("LowCut Freq", "LowCut Freq", 
        NormalisableRange<float>(20.f, 20000.f, 1.f, lowCutSkew), 20.f));

    layout.add(std::make_unique<AudioParameterFloat>("HighCut Freq", "HighCut Freq",
        NormalisableRange<float>(20.f, 20000.f, 1.f, HighCutSkew), 20000.f));

    layout.add(std::make_unique<AudioParameterFloat>("Peak Freq", "Peak Freq",
        NormalisableRange<float>(20.f, 20000.f, 1.f, PeakFreqSkew), 750.f));

    layout.add(std::make_unique<AudioParameterFloat>("Peak Gain", "Peak Gain",
        NormalisableRange<float>(-24.f, 24.f, 0.5f, PeakGainSkew), 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>("Peak Quality", "Peak Quality",
        NormalisableRange<float>(0.1f, 10.f, 0.05f, PeakQualitySkew), 1.f));


    StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique<AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TokyoEQAudioProcessor();
}
