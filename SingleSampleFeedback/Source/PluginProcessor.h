/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TanhLookupTable.h"
#include "WaveTable.h"

//==============================================================================
/**
*/
class SingleSampleFeedbackAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    SingleSampleFeedbackAudioProcessor();
    ~SingleSampleFeedbackAudioProcessor() override;

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

    /**
    *   Refers to the feedback buffer's block size
    */
    AudioBuffer<float> pre_block;
    void setFeedbackBlockSize(int numChannels)
    {
        int blockSizeInt = parameters.getRawParameterValue("fb_block_size")->load();
        pre_block.setSize(numChannels, blockSizeInt, true);
        pre_block.clear();
    }

    std::atomic<float> current_samplerate;
    std::atomic<float> freqCarrier;
    std::atomic<bool> tanhClippingEnabled;
    
	// Storage vectors for the wavetables
    std::vector<std::unique_ptr<WaveTableBase>> carriers;
    std::vector<std::unique_ptr<WaveTableBase>> modulators;
	// Indices for the active carrier and modulator
    int activeCarrierIndex = 0;
    int activeModulatorIndex = 0;

    SmoothedValue<float> level = 0.25f;
    SmoothedValue<float> feedback = 0.0f;

	AudioProcessorValueTreeState parameters;
	AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
        return {
            std::make_unique<AudioParameterFloat>("amp_level", "Linear Gain Level", 0.f, 1.f, 0.5f),
			std::make_unique<AudioParameterInt>("fb_block_size", "Block Size", 1, 128, 64),
            std::make_unique<AudioParameterFloat>("freq_carrier", "Carrier Frequency", 50.0f, 15000.0f, 440.0f),
            std::make_unique<AudioParameterFloat>("freq_mod", "Modulator Frequency", -2.f, 2.f, 1.f),
            std::make_unique<AudioParameterFloat>("freq_mod_exp", "Modulator Frequency Exponent", 0.f, 1.f, 0.5f),
			std::make_unique<AudioParameterBool>("tanh_clipping", "Tanh Clipping", false)
        };
	}

    void parameterChanged(const juce::String& parameterID, float newValue) override;
private:
    //==============================================================================
    TanhLookupTable tanhLUT;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleSampleFeedbackAudioProcessor)
};


