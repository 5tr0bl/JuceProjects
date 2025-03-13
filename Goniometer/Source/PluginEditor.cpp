/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GoniometerAudioProcessorEditor::GoniometerAudioProcessorEditor (GoniometerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    addAndMakeVisible(lissajous);
    addAndMakeVisible(corr);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 500);

    startTimerHz(24);
}

GoniometerAudioProcessorEditor::~GoniometerAudioProcessorEditor()
{
}

void GoniometerAudioProcessorEditor::timerCallback()
{
    lissajous.setLeft(audioProcessor.left_sample);
    lissajous.setRight(audioProcessor.right_sample);
    lissajous.repaint();
    //if (audioProcessor.nextFFTBlockReadyLeft) {/* Draw Right */ }
    if (audioProcessor.nextFFTBlockReadyRight) 
    {
        corr.getCorrelation(audioProcessor.fftDataLeft, audioProcessor.fftDataRight);
        audioProcessor.nextFFTBlockReadyLeft = false;
        audioProcessor.nextFFTBlockReadyRight = false;
        corr.repaint();
    }
}

//==============================================================================
void GoniometerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void GoniometerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    lissajous.setBounds(area.removeFromTop( getWidth() ));
    corr.setBounds(area.removeFromTop( getHeight() - getWidth() ));
}
