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
    BarRangeSheet() : startBar(0), endBar(std::nullopt)
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

	BarRangeSheet(int startBar, std::optional<int> endBar) : BarRangeSheet()
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
    std::optional<int> endBar;
    juce::TextEditor textEditor;
    
	// Returns the default text for a new BarRangeSheet's TextEditor
    static juce::String getDefaultTextEditorText()
    {
        return "Enter your notes here...";
    }

    // Returns a text for the end bar displayed in a BarRangeLabel
	juce::String getBarRangeEndText() const
	{
		return endBar.has_value() ? juce::String(endBar.value()) : "End";
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BarRangeSheet)
};