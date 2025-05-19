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
    // Initialize variables that are not tied to audio parameters
    current_samplerate = new std::atomic<float>(44100.f);

    // Initialize variables that are tied to audio parameters
    blockSize = new std::atomic<int>(64);
    pre_block = AudioBuffer<float>(2, static_cast<int>(blockSize->load()));
	freqCarrier = new std::atomic<float>(440.f);
	tanhClippingEnabled = new std::atomic<bool>(false);

    parameters.addParameterListener("freq_carrier", this);
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
    current_samplerate->store(static_cast<float>(sampleRate));
    
    //pre_block.setSize(getNumOutputChannels(), samplesPerBlock, true, true, true); // change to custom block size
    setFeedbackBlockSize(getNumOutputChannels());
    
    /*          Sine Carrier
    auto * osc_main = new SineOscillator();
    osc_main->setFrequency(frequency_main, sampleRate);
    osc_main->updateAngle();*/

    // Do not allocate the wavetables like this
    const unsigned int tableSize = 1 << 7; // 128
    auto* osc_main = new SawWave(tableSize);
    osc_main->setFrequency(freqCarrier->load(), sampleRate);
    //osc_main->fillWavetable();
    wavetables.add(osc_main);
    auto* osc_mod = new SineOscillator();
    osc_mod->setFrequency(frequency_mod, sampleRate);
    osc_mod->updateAngle();
    mod = *osc_mod;
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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	int blockSizeLocal = static_cast<int>(blockSize->load());

    auto* leftBuffer = buffer.getWritePointer(0, 0);
    auto* rightBuffer = buffer.getWritePointer(1, 0);
    auto* fb_left = pre_block.getWritePointer(0, 0);
    auto* fb_right = pre_block.getWritePointer(1, 0);

    auto* oscillatorMain = wavetables.getUnchecked(0);
    SineOscillator* oscillatorMod = &mod;
    auto feedbackValue = feedback.getCurrentValue();
    auto levelValue = level.getNextValue();

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        auto levelSample = oscillatorMain->getNextSample();
        auto modSample = oscillatorMod->getNextSample();
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
	if (parameterID == "freq_carrier")
	{
		auto* freqCarrierParam = parameters.getRawParameterValue("freq_carrier");
        freqCarrier->store(freqCarrierParam->load());
		wavetables[0]->setFrequency(freqCarrier->load(), current_samplerate->load());
	}
    else if (parameterID == "fb_block_size")
    {
        auto* blockSizeParam = parameters.getRawParameterValue("fb_block_size");
        blockSize->store(static_cast<int>(blockSizeParam->load()));
        setFeedbackBlockSize(getNumOutputChannels());
    }
    else if (parameterID == "tanh_clipping")
    {
        auto* tanhClippingParam = parameters.getRawParameterValue("tanh_clipping");
		tanhClippingEnabled->store(static_cast<bool>(tanhClippingParam->load()));
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
