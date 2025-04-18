/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ParametricEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor&, juce::AudioProcessorValueTreeState& apvts);
    ~ParametricEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ParametricEQAudioProcessor& audioProcessor;

	juce::Slider centerFrequencySlider;
    juce::Label centerFrequencySliderLabel;

    juce::ComboBox filterTypeComboBox;
	juce::Label filterTypeComboBoxLabel;

	juce::Slider gainSlider;
	juce::Label gainSliderLabel;

    juce::Slider qSlider;
    juce::Label qSliderLabel;

    std::unique_ptr<juce::SliderParameterAttachment> centerFrequencySliderAttachment;
    std::unique_ptr<juce::ComboBoxParameterAttachment> filterTypeComboBoxAttachment;
	std::unique_ptr<juce::SliderParameterAttachment> gainSliderAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> qSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessorEditor)
};
