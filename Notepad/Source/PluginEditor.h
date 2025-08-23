/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BarRangeSheet.h"

using namespace std;

//==============================================================================
/**
*/
class NotepadAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     public juce::Timer,
	                                 public juce::TextEditor::Listener
{
public:
    NotepadAudioProcessorEditor (NotepadAudioProcessor&);
    ~NotepadAudioProcessorEditor() override;

    void textEditorTextChanged(juce::TextEditor& editor) override;

    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BarRangeSheet* getActiveSheet(int currentBar);
    void addBarRangeSheet(int startBar);
    void removeBarRangeSheet(int index);

    NotepadAudioProcessor& audioProcessor;

    TextEditor textEditor;

	vector<unique_ptr<BarRangeSheet>> barRangeSheets;
    BarRangeSheet* currentlyActiveSheet = nullptr;

    // Buttons
	juce::TextButton addSheetButton{ "Add Sheet" };
	juce::TextButton removeSheetButton{ "Remove Sheet" };

    // Buttons' Listeners

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotepadAudioProcessorEditor)
};
