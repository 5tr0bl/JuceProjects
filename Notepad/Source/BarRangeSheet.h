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
        // User-editable text editor
        textEditor.setMultiLine(true);
        textEditor.setReturnKeyStartsNewLine(true);
        textEditor.setCaretVisible(true);
        textEditor.setScrollbarsShown(true);
        textEditor.setText(getDefaultTextEditorText());
    }

    BarRangeSheet(int startBar, int endBar) : BarRangeSheet()
    {
        this->startBar = startBar;
        this->endBar = endBar;
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
    
    static juce::String getDefaultTextEditorText()
    {
        return "Enter your notes here...";
    }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BarRangeSheet)
};