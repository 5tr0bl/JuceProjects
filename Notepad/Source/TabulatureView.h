/*
  ==============================================================================

    TabulatureView.h
    Created: 30 Dec 2025 3:30:40pm
    Author:  mst

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TabulatureView : public juce::Component
{
public:
    // Default constructor which calls the explicit coinstructor below
    TabulatureView() : TabulatureView(6) {}

    TabulatureView(int numLines) : lineCount(numLines)
    {
        // User-editable text editor
        tabulatureEditor.setMultiLine(true);
        tabulatureEditor.setReturnKeyStartsNewLine(false);
        tabulatureEditor.setCaretVisible(true);
        tabulatureEditor.setScrollbarsShown(true);
        tabulatureEditor.setText(getDefaultTextEditorText(lineCount));
    }

    ~TabulatureView() override = default;

    void paint(juce::Graphics& g) override
    {
        // Paint implementation (if needed)
    }
    void resized() override
    {
        // Resized implementation (if needed)
	}

    // Returns the default text for a new BarRangeSheet's TextEditor
    static juce::String getDefaultTextEditorText(int numLines)
    {
        juce::String text = "|---|";

        for (int i = 1; i < numLines; i++)
            text.append("\n|---|", 8);

        return text;
    }

private:
	int lineCount;

	juce::TextEditor tabulatureEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabulatureView)
};