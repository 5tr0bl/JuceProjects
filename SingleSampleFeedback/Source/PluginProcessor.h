/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TanhLookupTable.h"

class SineOscillator
{
public:
    SineOscillator() {}

    void setFrequency(float frequency, float sampleRate)
    {
        auto cyclesPerSample = frequency / sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
}

    forcedinline void updateAngle() noexcept
    {
        currentAngle += angleDelta;
        if (currentAngle >= juce::MathConstants<float>::twoPi)
            currentAngle -= juce::MathConstants<float>::twoPi;
    }

    forcedinline float getNextSample() noexcept
    {
        auto currentSample = std::sin(currentAngle);
        updateAngle();
        return currentSample;
    }

private:
    float currentAngle = 0.0f, angleDelta = 0.0f;
};

class SawWave
{
public:
    SawWave(unsigned int tableSizeExt) 
    {
        tableSizeInternal = (int)tableSizeExt - 1;
        wavetable.setSize(1, (int)tableSizeExt);
        fillWavetable();
    };

    forcedinline void fillWavetable()
    {
        auto* samples = wavetable.getWritePointer(0);

        auto angleDelta = juce::MathConstants<double>::twoPi / (double)(tableSizeInternal - 1);
        auto currentAngle = 0.0;

        for (unsigned int i = 0; i < tableSizeInternal; ++i)
        {
            //float flr = floorf(float(1.f / 2.f) + float(i / tableSize));
            auto sample = 2.f * (float(i / tableSizeInternal) - floorf( (1.f / 2.f) + ( float(i) / float(tableSizeInternal) ) )); // Saw
            samples[i] = (float)sample;
            currentAngle += angleDelta;
        }
        samples[tableSizeInternal] = samples[0];    // Value of first & last sample identical
    }

    forcedinline float getNextSample()
    {
        auto* table = wavetable.getReadPointer(0);
        auto index0 = (unsigned int)currentIndex;
        auto index1 = index0 + 1;
        //auto index1 = index0 == (tableSizeInternal - 1) ? (unsigned int)0 : index0 + 1;

        auto frac = currentIndex - (float)index0;              

        auto value0 = table[index0];
        auto value1 = table[index1];

        auto currentSample = value0 + frac * (value1 - value0); // linear Interpolation

        if ((currentIndex += tableDelta) > (float)tableSizeInternal)   // wrap around table
            currentIndex -= (float)tableSizeInternal;

        return currentSample;
    }

    void setFrequency(float frequency, float sampleRate)
    {
        auto tableSizeOverSampleRate = (float)tableSizeInternal / sampleRate;
        tableDelta = frequency * tableSizeOverSampleRate;
    }

private:
    int tableSizeInternal;
    AudioBuffer<float> wavetable;
    float currentIndex = 0.0f, tableDelta = 0.0f;
};
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
    std::atomic<int>* blockSize;
    AudioBuffer<float> pre_block;
    void setFeedbackBlockSize(int numChannels)
    {
        int blockSizeInt = static_cast<int>(blockSize->load());
        pre_block.setSize(numChannels, blockSizeInt, true);
        pre_block.clear();
    }

    std::atomic<float>* current_samplerate;
    std::atomic<float>* freqCarrier;
    std::atomic<bool>* tanhClippingEnabled;

    double frequency_mod = 30;
    //double currentAngle_main = 0.0, angleDelta_main = 0.0;
    //double currentAngle_mod = 0.0, angleDelta__mod = 0.0;
    
    //juce::OwnedArray<SineOscillator> oscillators;
    juce::OwnedArray<SawWave> wavetables;
    
    //SawWave& main;
    SineOscillator mod;
    SmoothedValue<float> level = 0.25f;
    SmoothedValue<float> feedback = 0.0f;

	AudioProcessorValueTreeState parameters;
	AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
        return { 
            std::make_unique<AudioParameterFloat>("freq_carrier", "Carrier Frequency", 50.0f, 15000.0f, 440.0f),
			std::make_unique<AudioParameterInt>("fb_block_size", "Block Size", 1, 128, 64),
			std::make_unique<AudioParameterBool>("tanh_clipping", "Tanh Clipping", false)
        };
	}

    void parameterChanged(const juce::String& parameterID, float newValue) override;
private:
    //==============================================================================
    TanhLookupTable tanhLUT;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleSampleFeedbackAudioProcessor)
};


