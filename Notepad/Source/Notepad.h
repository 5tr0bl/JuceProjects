/*
  ==============================================================================

    Notepad.h
    Created: 11 Feb 2026 9:24:06pm
    Author:  mst

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BarRangeSheet.h"

class Notepad : public juce::Component,
    private juce::Timer,
    private juce::TextEditor::Listener
{
public:
    explicit Notepad(NotepadAudioProcessor& p);
    ~Notepad() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // public API
    void loadFromProcessor();
    void commitToProcessor();
    void addSheetAt(int startBar);
    void removeSheetAt(int index);

private:
    NotepadAudioProcessor& audioProcessor;

    juce::Label barRangeLabel;
    juce::TextButton addButton, removeButton, leftButton, rightButton;

    std::vector<std::unique_ptr<BarRangeSheet>> sheets;
    BarRangeSheet* currentlyActiveSheet = nullptr;
    int manualSheetIndex = 0;

    // Formerly in Editor
    void updateActiveSheet(BarRangeSheet* newSheet);

    // Timer polls audioProcessor atomics (safe)
    void timerCallback() override;

    // TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor& editor) override;

    BarRangeSheet* getActiveSheetForBar(int currentBar);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Notepad)
};