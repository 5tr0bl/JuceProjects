/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), 
    parameters(*this, nullptr, "PARAMETERS", 
        {
			std::make_unique<juce::AudioParameterFloat>("centerFrequency", "Center Frequency", 
                juce::NormalisableRange{20.f, 20000.f, 0.1f, 0.2f, false}, 1000.f),
			std::make_unique<juce::AudioParameterFloat>("gain", "Gain", -24.f, 24.f, 0.f),
			std::make_unique<juce::AudioParameterFloat>("q", "Q", 
                juce::NormalisableRange{0.5f, 5.f, 0.01f}, 10.f),
            std::make_unique<juce::AudioParameterChoice>("filterType", "Filter Type", juce::StringArray{ "Low Pass", "High Pass", "Band Pass", "Notch", "Peak", "Low Shelf", "High Shelf" }, 0)
        })
#endif
{
    centerFrequencyParam = parameters.getRawParameterValue("centerFrequency");
    filterTypeParam = parameters.getRawParameterValue("filterType");
	gainParam = parameters.getRawParameterValue("gain");
	qParam = parameters.getRawParameterValue("q"); // BW = f_c / Q

	parameters.addParameterListener("centerFrequency", this);
	parameters.addParameterListener("filterType", this);
	parameters.addParameterListener("gain", this);
	parameters.addParameterListener("q", this);
}

ParametricEQAudioProcessor::~ParametricEQAudioProcessor()
{
}

//==============================================================================
const juce::String ParametricEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParametricEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParametricEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParametricEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParametricEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParametricEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParametricEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParametricEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ParametricEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    filter.setSampleRate(sampleRate);

    auto centerFrequency = centerFrequencyParam->load();
    filter.setCenterFrequency(centerFrequency);

    auto filterType = static_cast<int>(*filterTypeParam);
    filter.setFilterType(static_cast<FilterType>(filterType));
    
    auto gain = gainParam->load();
    filter.setGain(gain);
    
    auto q = qParam->load();
	filter.setQ(q);

}

void ParametricEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ParametricEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    /*
	const auto q = qParam->load();
	filter.setQ(q);
	const auto centerFrequency = centerFrequencyParam->load();
    filter.setCenterFrequency(centerFrequency);
    const auto filterType = filterTypeParam->load();
	filter.setFilterType(static_cast<FilterType>(filterType));
    */

    filter.processBlock(buffer);

}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
    return new ParametricEQAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void ParametricEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void ParametricEQAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
	if (parameterID == "centerFrequency")
	{
		filter.setCenterFrequency(newValue);
	}
	else if (parameterID == "filterType")
	{
		filter.setFilterType(static_cast<FilterType>(static_cast<int>(newValue)));
	}
	else if (parameterID == "gain")
	{
		filter.setGain(newValue);
	}
	else if (parameterID == "q")
	{
		filter.setQ(newValue);
	}
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
