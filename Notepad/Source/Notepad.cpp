/*
  ==============================================================================

    Notepad.cpp
    Created: 11 Feb 2026 9:24:06pm
    Author:  mst

  ==============================================================================
*/

#include "Notepad.h"

Notepad::Notepad(NotepadAudioProcessor & p) : audioProcessor(p)
{
    addAndMakeVisible(barRangeLabel);
    barRangeLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(addButton);
    addAndMakeVisible(removeButton);
    addAndMakeVisible(leftButton);
    addAndMakeVisible(rightButton);

    addButton.setButtonText("Add Sheet");
    removeButton.setButtonText("Remove");
    leftButton.setButtonText("<");
    rightButton.setButtonText(">");

    addButton.setTooltip("Add a new Note starting from the current bar");
    removeButton.setTooltip("Remove the currently active Note");
    leftButton.setTooltip("Go to previous Note");
    rightButton.setTooltip("Go to next Note.");

    addButton.onClick = [this]() { addSheetAt(audioProcessor.getBarCount()); };
    removeButton.onClick = [this]()
        {
            if (currentlyActiveSheet)
            {
                auto it = std::find_if(sheets.begin(), sheets.end(),
                    [this](auto& s) { return s.get() == currentlyActiveSheet; });
                if (it != sheets.end())
                    removeSheetAt(static_cast<int>(std::distance(sheets.begin(), it)));
            }
        };

    // start timer to poll playhead
    startTimerHz(30);

    loadFromProcessor();
}

Notepad::~Notepad() = default;

void Notepad::loadFromProcessor()
{
    sheets.clear();
    for (const auto& d : audioProcessor.barRangeSheetData)
    {
        auto s = std::make_unique<BarRangeSheet>(d.startBar, d.endBar);
        s->textEditor.setText(d.text);
        s->textEditor.addListener(this);

        // Add the sheet component itself; the sheet owns and shows its TextEditor
        addAndMakeVisible(*s);

        sheets.push_back(std::move(s));
    }

    // Choose an initial active sheet: prefer the playhead sheet, otherwise first sheet
    auto* active = getActiveSheetForBar(audioProcessor.getBarCount());
    if (!active && !sheets.empty())
        active = sheets.front().get();

    updateActiveSheet(active);

	// also try here to fix the issue that sheets are not visible after adding them
	resized(); // force layout update
}

// find sheet with largest startBar <= currentBar
BarRangeSheet* Notepad::getActiveSheetForBar(int currentBar)
{
    for (int i = static_cast<int>(sheets.size()) - 1; i >= 0; --i)
    {
        auto& s = sheets[static_cast<size_t>(i)];
        if (currentBar >= s->startBar)
        {
            if (!s->endBar.has_value() || currentBar <= s->endBar.value())
                return s.get();
        }
    }
    return nullptr;
}

void Notepad::updateActiveSheet(BarRangeSheet* newSheet)
{
	currentlyActiveSheet = newSheet;

	// show only the active sheet component (sheet manages its editor internally)
	for (auto& s : sheets)
		s->setVisible(s.get() == newSheet);

	if (newSheet)
	{
		barRangeLabel.setText("Bars " + juce::String(newSheet->startBar) + " to " + newSheet->getBarRangeEndText(),
			juce::dontSendNotification);
		barRangeLabel.setVisible(true);
	}
	else
	{
		barRangeLabel.setVisible(false);
	}
}

void Notepad::timerCallback()
{
    const bool isPlaying = audioProcessor.getIsPlaying();
    const int currentBar = audioProcessor.getBarCount();

    auto* active = getActiveSheetForBar(currentBar);
    if (active != currentlyActiveSheet)
    {
        //if (active)
        //{
            updateActiveSheet(active); // this is new now, from oct 2026, commee
        //}
    }

    leftButton.setEnabled(!isPlaying);
    rightButton.setEnabled(!isPlaying);
}

void Notepad::textEditorTextChanged(juce::TextEditor& editor)
{
	int sheetsSize = static_cast<int>(sheets.size());
    for (int i = 0; i < sheetsSize; ++i)
    {
        // find which sheet this editor belongs to, and update commit
        if (&sheets[i]->textEditor == &editor)
        {
			if (i >= 0 && i < static_cast<int>(audioProcessor.barRangeSheetData.size()))
                audioProcessor.barRangeSheetData[i].text = editor.getText();
            return;
        }
    }
}

void Notepad::addSheetAt(int startBar)
{
	// Find insertion index (keep sheets sorted by startBar)
	int insertIndex = 0;
	while (insertIndex < static_cast<int>(audioProcessor.barRangeSheetData.size()) && audioProcessor.barRangeSheetData[insertIndex].startBar < startBar)
		++insertIndex;

	// Prevent duplicate startBar
	if (!audioProcessor.barRangeSheetData.empty())
	{
		if (insertIndex > 0 && audioProcessor.barRangeSheetData[insertIndex - 1].startBar == startBar)
			return;
		if (insertIndex < static_cast<int>(audioProcessor.barRangeSheetData.size()) && audioProcessor.barRangeSheetData[insertIndex].startBar == startBar)
			return;
	}

	// Determine endBar for the new sheet (if inserting before another sheet)
	std::optional<int> endBar = std::nullopt;
	if (insertIndex < static_cast<int>(audioProcessor.barRangeSheetData.size()))
		endBar = audioProcessor.barRangeSheetData[insertIndex].startBar - 1;

	//// Create UI sheet
	//auto newSheet = std::make_unique<BarRangeSheet>(startBar, endBar);
	//newSheet->textEditor.setText(BarRangeSheet::getDefaultTextEditorText());
	//newSheet->textEditor.addListener(this);
	//
	//// Add the sheet component (sheet owns its TextEditor)
	//addAndMakeVisible(*newSheet);

	// Insert into UI vector
	//sheets.insert(sheets.begin() + insertIndex, std::move(newSheet));

	// Create and insert into processor-side vector
	NotepadAudioProcessor::BarRangeSheetData sheetData;
	sheetData.startBar = startBar;
	sheetData.endBar = endBar;
	sheetData.text = BarRangeSheet::getDefaultTextEditorText();
	audioProcessor.barRangeSheetData.insert(audioProcessor.barRangeSheetData.begin() + insertIndex, sheetData);

	// Update previous sheet's endBar if there is a previous sheet
	if (insertIndex > 0)
	{
		//sheets[insertIndex - 1]->endBar = startBar - 1;
		audioProcessor.barRangeSheetData[insertIndex - 1].endBar = startBar - 1;
	}

	// Make the newly inserted sheet active and visible
	//updateActiveSheet(audioProcessor.barRangeSheetData[insertIndex].get());

    // Try to fix the issue that sheets are not visible after adding the,m
	//resized(); // force layout update
    
    // After insertion:
    //commitToProcessor();
    loadFromProcessor(); // or better: mutate the in-memory sheets and add children
}

void Notepad::removeSheetAt(int index)
{
    if (index < 0 || index >= static_cast<int>(audioProcessor.barRangeSheetData.size()))
        return;
    
    audioProcessor.barRangeSheetData.erase(audioProcessor.barRangeSheetData.begin() + index);

    // Update previous sheets' endBars
    if (index > 0 && index <= static_cast<int>(audioProcessor.barRangeSheetData.size()))
    {
        // did we remove the last sheet?
        if (index == static_cast<int>(audioProcessor.barRangeSheetData.size()))
        {
			//sheets[index - 1]->endBar = std::nullopt;
			audioProcessor.barRangeSheetData[index - 1].endBar = std::nullopt;
        }
        else
        {
			//sheets[index - 1]->endBar = sheets[index]->startBar - 1;
			audioProcessor.barRangeSheetData[index - 1].endBar = audioProcessor.barRangeSheetData[index].startBar - 1;
        }
    }

	// the removed sheet is still "the active one" after removal, so update the active sheet to the one that now contains the current bar
    //updateActiveSheet(getActiveSheetForBar(audioProcessor.getBarCount()));

    loadFromProcessor();
}

void Notepad::paint(juce::Graphics& g)
{
    // fill with default background so components inside look correct
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

// resized: layout barRangeLabel, buttons and the active sheet's textEditor
void Notepad::resized()
{
    auto area = getLocalBounds();
    barRangeLabel.setBounds(area.removeFromTop(24));

    auto buttonArea = area.removeFromBottom(32);
    leftButton.setBounds(buttonArea.removeFromLeft(40).reduced(4));
    rightButton.setBounds(buttonArea.removeFromRight(40).reduced(4));
    addButton.setBounds(buttonArea.removeFromLeft(80).reduced(4));
    removeButton.setBounds(buttonArea.removeFromLeft(80).reduced(4));

    // layout each sheet component full remaining area (sheet will layout its editor)
    for (auto& s : sheets)
        s->setBounds(area.reduced(6));
	//s->textEditor.setBounds(area.reduced(6));

}