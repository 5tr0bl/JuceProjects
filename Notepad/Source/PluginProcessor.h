/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

using namespace std;

//==============================================================================
/**
*/
class NotepadAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NotepadAudioProcessor();
    ~NotepadAudioProcessor() override;

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

    // const always better for getters
    // noexcept safe here, as atomic operation will not throw exception
	int getBarCount() const noexcept { return barCount.load(); }

    struct BarRangeSheetData {
        int startBar;
        int endBar;
        juce::String text;
    };
    std::vector<BarRangeSheetData> barRangeSheetData;

	const int DEFAULT_END_BAR = 1000; // Default end bar for new sheets

private:
    //==============================================================================
    atomic<int> barCount;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotepadAudioProcessor)
};
