/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SingleSampleFeedbackAudioProcessorEditor::SingleSampleFeedbackAudioProcessorEditor (SingleSampleFeedbackAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 400);
    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(modFreqSlider);
    addAndMakeVisible(modFreq2Slider);
    addAndMakeVisible(ampSlider);
    addAndMakeVisible(feedbackAmount);

    pitchTextButton.setButtonText("Carrier Pitch");
    addAndMakeVisible(pitchTextButton); 
    modFreqTextButton.setButtonText("Mod. Freq.");
    addAndMakeVisible(modFreqTextButton);
    modFreqTextResultButton.setButtonText("0 Hz");
    addAndMakeVisible(modFreqTextResultButton);
    ampTextButton.setButtonText("Amp");
    addAndMakeVisible(ampTextButton);
    feedbackTextButton.setButtonText("Feedback");
    addAndMakeVisible(feedbackTextButton);

    frequencySlider.setRange(50.0, 5000.0);
    modFreqSlider.setRange(-2.0, 5.0);
    modFreq2Slider.setRange(0.0, 1.0);
    ampSlider.setRange(0.0, 0.5);
    feedbackAmount.setRange(0.0, 3.0);
    //ampSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    //feedbackAmount.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    frequencySlider.setSkewFactorFromMidPoint(500.0);
    modFreqSlider.setSkewFactorFromMidPoint(0.0);
    frequencySlider.setTextValueSuffix(" Hz");
    modFreqSlider.setTextValueSuffix(" Hz");
    frequencySlider.addListener(this);
    modFreqSlider.addListener(this);
    modFreq2Slider.addListener(this);
    ampSlider.addListener(this);
    feedbackAmount.addListener(this);


}

SingleSampleFeedbackAudioProcessorEditor::~SingleSampleFeedbackAudioProcessorEditor()
{
}

//==============================================================================
void SingleSampleFeedbackAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    /*
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    */
}

void SingleSampleFeedbackAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto sidebarArea = area.removeFromLeft(area.getWidth() / 5);
    auto lineHeight = 40;

    pitchTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    modFreqTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    modFreqTextResultButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    ampTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    feedbackTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));

    frequencySlider.setBounds(area.removeFromTop(lineHeight));
    modFreqSlider.setBounds(area.removeFromTop(lineHeight));
    modFreq2Slider.setBounds(area.removeFromTop(lineHeight));
    ampSlider.setBounds(area.removeFromTop(lineHeight));
    feedbackAmount.setBounds(area.removeFromTop(lineHeight));
}
