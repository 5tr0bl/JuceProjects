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
    void updateActiveSheet(BarRangeSheet* newSheet);
    BarRangeSheet* getActiveSheet(int currentBar);
    void addBarRangeSheet(int startBar);
    void removeBarRangeSheet(int index);

    NotepadAudioProcessor& audioProcessor;

    juce::Label barRangeLabel;
    TextEditor textEditor;

	vector<unique_ptr<BarRangeSheet>> barRangeSheets;
    BarRangeSheet* currentlyActiveSheet = nullptr;

    // for manaully scrolling through note sheets
	int manualSheetIndex = 0;
    bool isPlaying = false;
    
    // Buttons
	juce::TextButton addSheetButton{ "Add Sheet" };
	juce::TextButton removeSheetButton{ "Remove Sheet" };
    juce::TextButton leftButton;
    juce::TextButton rightButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotepadAudioProcessorEditor)
};
