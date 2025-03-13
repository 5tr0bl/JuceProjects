/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NotepadAudioProcessorEditor::NotepadAudioProcessorEditor (NotepadAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    setSize (400, 300);
    addAndMakeVisible(textEditor);
    textEditor.setMultiLine(true);
    textEditor.setReturnKeyStartsNewLine(true);
    textEditor.setCaretVisible(true);
    textEditor.setScrollbarsShown(true);

    startTimerHz(30);
}

NotepadAudioProcessorEditor::~NotepadAudioProcessorEditor()
{
}

void NotepadAudioProcessorEditor::timerCallback()
{
    AudioPlayHead* playHead = audioProcessor.getPlayHead();
    if (playHead != nullptr)
    {
        playHead->getPosition(currentPositionInfo);
    }
}

//==============================================================================
void NotepadAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void NotepadAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    textEditor.setBounds(area);
}
