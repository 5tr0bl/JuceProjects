/*
  ==============================================================================

    BarRangeSheet.h
    Created: 22 Aug 2025 9:32:13pm
    Author:  Micha

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class BarRangeSheet : public juce::Component
{
public:
    BarRangeSheet() : startBar(0), endBar(0)
    {
        textEditor.setMultiLine(true);
        textEditor.setReturnKeyStartsNewLine(true);
        textEditor.setCaretVisible(true);
        textEditor.setScrollbarsShown(true);
    }

    BarRangeSheet(int startBar, int endBar, const String text)
    {
		this->startBar = startBar;
		this->endBar = endBar;

        textEditor.setMultiLine(true);
        textEditor.setReturnKeyStartsNewLine(true);
        textEditor.setCaretVisible(true);
        textEditor.setScrollbarsShown(true);
        textEditor.setText(text);
    }
    
    ~BarRangeSheet() override {}
    
    void paint(juce::Graphics& g) override
    {

    }
    
    void resized() override
    {

    }

    int startBar;
    int endBar;
    juce::TextEditor textEditor;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BarRangeSheet)
};