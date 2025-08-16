/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>

//==============================================================================
SingleSampleFeedbackAudioProcessor::SingleSampleFeedbackAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
	 ), parameters(*this, nullptr, "Parameters", createParameterLayout()),
        tanhLUT(TanhLookupTable(-3.f, 3.f, 1024))
#endif
{
    // Initialize variables that are tied to audio parameters
    int blockSize = parameters.getRawParameterValue("fb_block_size")->load();
    pre_block = AudioBuffer<float>(2, blockSize);

    // Remove these
	freqCarrier.store(440.f);
	tanhClippingEnabled.store(false);

	parameters.addParameterListener("amp_level", this);
    parameters.addParameterListener("freq_carrier", this);
	parameters.addParameterListener("freq_mod", this);
	parameters.addParameterListener("freq_mod_exp", this);
    parameters.addParameterListener("fb_block_size", this);
	parameters.addParameterListener("tanh_clipping", this);
}

SingleSampleFeedbackAudioProcessor::~SingleSampleFeedbackAudioProcessor()
{
}

//==============================================================================
const juce::String SingleSampleFeedbackAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SingleSampleFeedbackAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SingleSampleFeedbackAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SingleSampleFeedbackAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SingleSampleFeedbackAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SingleSampleFeedbackAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SingleSampleFeedbackAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SingleSampleFeedbackAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SingleSampleFeedbackAudioProcessor::getProgramName (int index)
{
    return {};
}

void SingleSampleFeedbackAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SingleSampleFeedbackAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Get some initialization variables
	float amp = parameters.getRawParameterValue("amp_level")->load();
	float frequency_carrier = parameters.getRawParameterValue("freq_carrier")->load();
    float frequency_mod = parameters.getRawParameterValue("freq_mod")->load();
    float frequency_mod_exp = parameters.getRawParameterValue("freq_mod_exp")->load();


	// Store currently used sample rate for ...
    current_samplerate.store(static_cast<float>(sampleRate));
    
	// Initialize the feedback buffer, requires knowledge of the number of output channels
    setFeedbackBlockSize(getNumOutputChannels());

    // Do not allocate the wavetables like this
    const unsigned int tableSize = 1 << 7; // 128

	// Initialize first carrier and modulator
    carriers.push_back(std::make_unique<SawWaveTable>(tableSize));
    modulators.push_back(std::make_unique<SineWaveTable>());

    // To change the type, simply replace the pointer:
	// carriers[0] = std::make_unique<SineWaveTable>();
	carriers[0]->setFrequency(frequency_carrier, sampleRate);
	modulators[0]->setFrequency(frequency_mod, sampleRate);
	modulators[0]->updateAngle();
}

void SingleSampleFeedbackAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    pre_block.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SingleSampleFeedbackAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SingleSampleFeedbackAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    /*
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    */

    // Get tanh enabled state from APVTS and cache it
	auto* tanhClippingParam = parameters.getRawParameterValue("tanh_clipping");
	bool tanhClippingEnabled = tanhClippingParam->load();

    // Get feddback buffer's block size from APVTS and cache it
    auto* blockSizeParam = parameters.getRawParameterValue("fb_block_size");
	int blockSizeLocal = static_cast<int>(blockSizeParam->load());

    // Get the active oscillators via indices
    WaveTableBase* carrierOsc = carriers[activeCarrierIndex].get();
	WaveTableBase* modOsc = modulators[activeModulatorIndex].get();

    // Access buffers
    float* leftBuffer = buffer.getWritePointer(0, 0);
    float* rightBuffer = buffer.getWritePointer(1, 0);
    float* fb_left = pre_block.getWritePointer(0, 0);
    float* fb_right = pre_block.getWritePointer(1, 0);

    // Remove this block!
    auto feedbackValue = feedback.getCurrentValue();
    auto levelValue = level.getNextValue();

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float levelSample = carrierOsc->getNextSample();
        float modSample = modOsc->getNextSample();
        leftBuffer[sample] = levelSample;
        leftBuffer[sample] *= (modSample + (fb_left[sample % blockSizeLocal] * feedbackValue));
        leftBuffer[sample] = tanhLUT.getValue(leftBuffer[sample]);
        fb_left[sample % blockSizeLocal] = leftBuffer[sample];
        leftBuffer[sample] *= levelValue;
        rightBuffer[sample] = levelSample;
        rightBuffer[sample] *= (modSample + (fb_right[sample % blockSizeLocal] * feedbackValue));
        rightBuffer[sample] = tanhLUT.getValue(rightBuffer[sample]);
        fb_right[sample % blockSizeLocal] = rightBuffer[sample];
        rightBuffer[sample] *= levelValue;
    }
}

//==============================================================================
bool SingleSampleFeedbackAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

/**
*   The Editor's standard constructor has been modified to also pass down the apvts.
*/
juce::AudioProcessorEditor* SingleSampleFeedbackAudioProcessor::createEditor()
{
    return new SingleSampleFeedbackAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void SingleSampleFeedbackAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SingleSampleFeedbackAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SingleSampleFeedbackAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "amp_level")
    {
        // Rather check for this in the beginning of processBlock() and set a local bool there...
    }
    else if (parameterID == "fb_block_size")
    {
        setFeedbackBlockSize(getNumOutputChannels());
    }
    else if (parameterID == "freq_carrier")
	{
		float freqCarrier = parameters.getRawParameterValue("freq_carrier")->load();
        
        // Set Frequency for each carrier in carriers[] vector
        for (const auto& carrier : carriers)
        {
            if (carrier)
                carrier->setFrequency(freqCarrier, current_samplerate.load());
        }
	}
    else if (parameterID == "freq_mod")
    {
        float freqCarrierMod = parameters.getRawParameterValue("freq_mod")->load();

        // Set Frequency for each carrier in modulators[] vector
        for (const auto& modulator : modulators)
        {
            if (modulator)
                modulator->setFrequency(freqCarrierMod, current_samplerate.load());
        }
    }
    else if (parameterID == "freq_mod_exp")
    {
        float freqCarrierMod = parameters.getRawParameterValue("freq_mod_exp")->load();

		// Only needs to be set once, not for multiple modulators
    }
    else if (parameterID == "tanh_clipping")
    {
        // Rather check for this in the beginning of processBlock() and set a local bool there...
        
        auto* tanhClippingParam = parameters.getRawParameterValue("tanh_clipping");
		tanhClippingEnabled.store(static_cast<bool>(tanhClippingParam->load()));
    }
	else
	{
		jassertfalse; // unknown parameter
	}
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SingleSampleFeedbackAudioProcessor();
}
