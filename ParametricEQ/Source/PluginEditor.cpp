/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // bandwidth slider
	addAndMakeVisible(qSlider);
    qSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    qSliderAttachment.reset(new juce::SliderParameterAttachment(*apvts.getParameter("q"), qSlider));
	addAndMakeVisible(qSliderLabel);
    qSliderLabel.setText("Q", juce::dontSendNotification);

	addAndMakeVisible(centerFrequencySlider);
	centerFrequencySlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    centerFrequencySliderAttachment.reset(new juce::SliderParameterAttachment(*apvts.getParameter("centerFrequency"), centerFrequencySlider));
	addAndMakeVisible(centerFrequencySliderLabel);
	centerFrequencySliderLabel.setText("Center Frequency", juce::dontSendNotification);

	addAndMakeVisible(filterTypeComboBox);
    filterTypeComboBox.addItem("Low Pass", 1);
    filterTypeComboBox.addItem("High Pass", 2);
    filterTypeComboBox.addItem("Band Pass", 3);
    filterTypeComboBox.addItem("Notch", 4);
    filterTypeComboBox.addItem("Peak", 5);
    filterTypeComboBox.addItem("Low Shelf", 6);
    filterTypeComboBox.addItem("High Shelf", 7);
    filterTypeComboBox.setSelectedId(0);
	filterTypeComboBoxAttachment.reset(new juce::ComboBoxParameterAttachment(*apvts.getParameter("filterType"), filterTypeComboBox));
	addAndMakeVisible(filterTypeComboBoxLabel);
	filterTypeComboBoxLabel.setText("Filter Type", juce::dontSendNotification);

	addAndMakeVisible(gainSlider);
	gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	gainSliderAttachment.reset(new juce::SliderParameterAttachment(*apvts.getParameter("gain"), gainSlider));

    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(2.0);
    setSize (500, 250);
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void ParametricEQAudioProcessorEditor::resized()
{
	auto width = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();

    centerFrequencySlider.setBounds(width * 0.1, height * 0.1, width * 0.4, height * 0.2);
    //centerFrequencySliderLabel.setBounds(centerFrequencySlider.getX(), centerFrequencySlider.getY() + centerFrequencySlider.getHeight(),
    //    centerFrequencySlider.getWidth() / 2, centerFrequencySlider.getWidth() / 2);
	
    qSlider.setBounds(width * 0.1, height * 0.4, width * 0.2, height * 0.2);
	//qSliderLabel.setBounds(qSlider.getX(), qSlider.getY() + qSlider.getHeight(),
    //    qSlider.getWidth() / 2, qSlider.getWidth() / 2);

    filterTypeComboBox.setBounds(width * 0.6, height * 0.1, width * 0.2, height * 0.2);
	filterTypeComboBoxLabel.setBounds(filterTypeComboBox.getX(), filterTypeComboBox.getY() + filterTypeComboBox.getHeight(),
		filterTypeComboBox.getWidth() / 2, filterTypeComboBox.getWidth() / 2);

    gainSlider.setBounds(width * 0.3, height * 0.4, width * 0.2, height * 0.2);
}
