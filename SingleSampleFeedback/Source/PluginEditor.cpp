/*
  ==============================================================================

    Note that createEditor() in PluginProcessor.cpp has been extended to also pass down
    the parameters/AudioProcessorValueTreeState.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SingleSampleFeedbackAudioProcessorEditor::SingleSampleFeedbackAudioProcessorEditor (SingleSampleFeedbackAudioProcessor& p, AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 400);
	setResizable(true, true);

    // Carrier Frequency Slider
    addAndMakeVisible(freqCarrierSlider);
    freqCarrierSlider.setRange(50.0, 15000.0);
    freqCarrierSlider.setSkewFactorFromMidPoint(5000.0);
	freqCarrierSlider.setValue(440.0);
    freqCarrierSlider.setTextValueSuffix(" Hz");
    freqCarrierSliderAttachment.reset(new SliderParameterAttachment(*apvts.getParameter("freq_carrier"), freqCarrierSlider));

	// Feedback Block Size
	addAndMakeVisible(feebackBlockSizeSlider);
	feebackBlockSizeSlider.setRange(1, 128, 1);
	feebackBlockSizeSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	feebackBlockSizeSlider.setValue(64);
	feedbackBlockSizeAttachment.reset(new SliderParameterAttachment(*apvts.getParameter("fb_block_size"), feebackBlockSizeSlider));

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

    modFreqSlider.setRange(-2.0, 5.0);
    modFreq2Slider.setRange(0.0, 1.0);
    ampSlider.setRange(0.0, 0.5);
    feedbackAmount.setRange(0.0, 3.0);
    //ampSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    //feedbackAmount.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    modFreqSlider.setSkewFactorFromMidPoint(0.0);
    modFreqSlider.setTextValueSuffix(" Hz");

    // remove these Sliders
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
    auto lineHeight = static_cast<int>(area.getHeight() / 10);

    pitchTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    modFreqTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    modFreqTextResultButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    ampTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
    feedbackTextButton.setBounds(sidebarArea.removeFromTop(lineHeight));
	feebackBlockSizeSlider.setBounds(sidebarArea.removeFromTop(lineHeight));

    freqCarrierSlider.setBounds(area.removeFromTop(lineHeight));
    modFreqSlider.setBounds(area.removeFromTop(lineHeight));
    modFreq2Slider.setBounds(area.removeFromTop(lineHeight));
    ampSlider.setBounds(area.removeFromTop(lineHeight));
    feedbackAmount.setBounds(area.removeFromTop(lineHeight));
}
