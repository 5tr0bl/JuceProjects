/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <math.h>

//==============================================================================
/**
*/
class SingleSampleFeedbackAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                  public juce::Slider::Listener
{
public:
    SingleSampleFeedbackAudioProcessorEditor (SingleSampleFeedbackAudioProcessor&);
    ~SingleSampleFeedbackAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == &frequencySlider)
        {
            audioProcessor.frequency_main = frequencySlider.getValue();
            audioProcessor.wavetables[0]->setFrequency(frequencySlider.getValue(), audioProcessor.current_samplerate);
        }
        else if (slider == &modFreqSlider)
        {
            audioProcessor.frequency_mod = modFreqSlider.getValue() * powf(10, modFreq2Slider.getValue());
            audioProcessor.mod.setFrequency(audioProcessor.frequency_mod, audioProcessor.current_samplerate);
            modFreqTextResultButton.setButtonText(juce::String(audioProcessor.frequency_mod) + " HZ");
        }
        else if (slider == &modFreq2Slider)
        {
            audioProcessor.frequency_mod = modFreqSlider.getValue() * powf(10, modFreq2Slider.getValue());
            audioProcessor.mod.setFrequency(audioProcessor.frequency_mod, audioProcessor.current_samplerate);
            modFreqTextResultButton.setButtonText(juce::String(audioProcessor.frequency_mod) + " HZ");
        }
        else if (slider == &ampSlider)
        {
            audioProcessor.level = ampSlider.getValue();
        }
        else if (slider == &feedbackAmount)
        {
            audioProcessor.feedback = feedbackAmount.getValue();
        }
    }

private:
    SingleSampleFeedbackAudioProcessor& audioProcessor;
    juce::Slider frequencySlider;
    juce::Slider modFreqSlider;
    juce::Slider modFreq2Slider;
    juce::Slider ampSlider;
    juce::Slider feedbackAmount;

    juce::TextButton pitchTextButton;
    juce::TextButton modFreqTextButton;
    juce::TextButton modFreqTextResultButton;
    juce::TextButton ampTextButton;
    juce::TextButton feedbackTextButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleSampleFeedbackAudioProcessorEditor)
};
