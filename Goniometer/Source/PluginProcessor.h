/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class GoniometerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    GoniometerAudioProcessor();
    ~GoniometerAudioProcessor() override;

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

    float left_sample, right_sample;
    static constexpr auto fftOrder = 5;
    static constexpr int fftSize = 1 << fftOrder;

    void pushSampleIntoQueue(const float& sample, std::vector<std::complex<float>> queue)
    {
        if (queue == fifoLeft)
        {
            // if the fifo contains enough data, set a flag to say
            // that the next line should now be rendered..
            if (fifoIndexLeft == fftSize)
            {
                if (!nextFFTBlockReadyLeft)
                {
                    std::fill(fftDataLeft.begin(), fftDataLeft.end(), std::complex<float>(0.0f, 0.0f));
                    std::copy(fifoLeft.begin(), fifoLeft.end(), fftDataLeft.begin());
                    nextFFTBlockReadyLeft = true;
                    forwardFftLeft.perform(fifoLeft.data(), fftDataLeft.data(), false);
                }

                fifoIndexLeft = 0;
            }

            fifoLeft[(size_t)fifoIndexLeft++] = std::complex<float>(sample, 0.0f);
        }

        else if (queue == fifoRight)
        {
            if (fifoIndexRight == fftSize)
            {
                if (!nextFFTBlockReadyRight)
                {
                    std::fill(fftDataRight.begin(), fftDataRight.end(), std::complex<float>(0.0f, 0.0f));
                    std::copy(fifoRight.begin(), fifoRight.end(), fftDataRight.begin());
                    nextFFTBlockReadyRight = true;
                    forwardFftRight.perform(fifoRight.data(), fftDataRight.data(), false);
                }

                fifoIndexRight = 0;
            }

            fifoRight[(size_t)fifoIndexRight++] = std::complex<float>(sample, 0.0f);
        }
    };



    bool nextFFTBlockReadyLeft, nextFFTBlockReadyRight = false;  // ready for rendering? 
    // results of FFT calculations
    std::vector<std::complex<float>> fftDataLeft = std::vector<std::complex<float>>(2 * fftSize);
    std::vector<std::complex<float>> fftDataRight = std::vector<std::complex<float>>(2 * fftSize);

private:
    //==============================================================================
    juce::dsp::FFT forwardFftLeft;  // Declare a dsp::FFT object to perform the forward FFT on.
    juce::dsp::FFT forwardFftRight;

    // std::array<float, fftSize> fifoLeft, fifoRight;              // for incoming audio samples
    std::vector<std::complex<float>> fifoLeft = std::vector<std::complex<float>>(fftSize);
    std::vector<std::complex<float>> fifoRight = std::vector<std::complex<float>>(fftSize);
    int fifoIndexLeft = 0;                                      // how many samples are in the queue
    int fifoIndexRight = 0;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GoniometerAudioProcessor)
};
