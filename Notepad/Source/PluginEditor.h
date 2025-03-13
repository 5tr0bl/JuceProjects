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
class NotepadAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    NotepadAudioProcessorEditor (NotepadAudioProcessor&);
    ~NotepadAudioProcessorEditor() override;

    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NotepadAudioProcessor& audioProcessor;

    TextEditor textEditor;
    AudioPlayHead::CurrentPositionInfo currentPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotepadAudioProcessorEditor)
};
