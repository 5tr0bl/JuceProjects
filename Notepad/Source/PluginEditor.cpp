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

    // Header containing the bar range
    addAndMakeVisible(barRangeLabel);
    barRangeLabel.setJustificationType(juce::Justification::centred);
    barRangeLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(textEditor);
    textEditor.addListener(this);
    
	// Copy the BarRangeSheetData from the processor to the editor's barRangeSheets
    for(const auto& sheetData : audioProcessor.barRangeSheetData)
    {
        auto sheet = std::make_unique<BarRangeSheet>();
		sheet->startBar = sheetData.startBar;
        sheet->endBar = sheetData.endBar;
        sheet->textEditor.setText(sheetData.text);
        sheet->textEditor.addListener(this);
		barRangeSheets.push_back(std::move(sheet));
    }

    // Buttons for adding/removing note sheets
	addAndMakeVisible(addSheetButton);
    addAndMakeVisible(removeSheetButton);

	addSheetButton.setButtonText("Add Sheet");
	removeSheetButton.setButtonText("Remove Sheet");

	addSheetButton.setTooltip("Add a new Note starting from the current bar");
	removeSheetButton.setTooltip("Remove the currently active Note");

    addSheetButton.onClick = [this]()
        {
            // Add a new BarRangeSheet starting from the current bar
            int nextBar = audioProcessor.getBarCount();
			addBarRangeSheet(nextBar);
        };
    removeSheetButton.onClick = [this]()
        {
            // Remove the currently active sheet if it exists
            if (currentlyActiveSheet)
            {
                auto it = std::find_if(barRangeSheets.begin(), barRangeSheets.end(),
                                       [this](const std::unique_ptr<BarRangeSheet>& sheet) {
                                           return sheet.get() == currentlyActiveSheet;
                                       });
                if (it != barRangeSheets.end())
                {
                    int index = std::distance(barRangeSheets.begin(), it);
                    removeBarRangeSheet(index);
                }
            }
		};

    // Buttons for manually sccrolling through note sheets
    addAndMakeVisible(leftButton);
    addAndMakeVisible(rightButton);

    leftButton.setButtonText("<");
    rightButton.setButtonText(">");

	leftButton.setTooltip("Go to previous Note");
    rightButton.setTooltip("Go to next Note.");
    
    leftButton.onClick = [this]()
        {
            if (!barRangeSheets.empty())
            {
                // Cycle through the sheets with wrap-around and take the previous one
                manualSheetIndex = (manualSheetIndex - 1 + barRangeSheets.size()) % barRangeSheets.size();
				updateActiveSheet(barRangeSheets[manualSheetIndex].get());
            }
        };
    rightButton.onClick = [this]()
        {
            if (!barRangeSheets.empty())
            {
				// Cycle through the sheets with wrap-around and take the next one
                manualSheetIndex = (manualSheetIndex + 1 + barRangeSheets.size()) % barRangeSheets.size();
                updateActiveSheet(barRangeSheets[manualSheetIndex].get());
            }
        };

    // Start Timer and play around with the frequency maybe
    startTimerHz(30);
}

NotepadAudioProcessorEditor::~NotepadAudioProcessorEditor()
{
}

void NotepadAudioProcessorEditor::textEditorTextChanged(juce::TextEditor& editor)
{
    // Check if the modified TextEditor is the one from the Editor instance
	// Check if an active sheet is currently being used
    if (&editor == &textEditor && currentlyActiveSheet)
    {
		// Copy the Editor's text into the currently active sheet
        currentlyActiveSheet->textEditor.setText(editor.getText());
    }
}

void NotepadAudioProcessorEditor::timerCallback()
{
    BarRangeSheet* activeSheet;
    const bool isPlaying = audioProcessor.getIsPlaying();
    
    if (isPlaying)
    {
		// Fetch currently active sheet via the AudioProcessor's Playhead
        const int currentBar = audioProcessor.getBarCount();
        activeSheet = getActiveSheet(currentBar);

        // Check if active sheet has changed
        if (activeSheet != currentlyActiveSheet)
        {
            currentlyActiveSheet = activeSheet;

            if (activeSheet)
            {
                // Update the bar range label
                const juce::String startBarString = juce::String(activeSheet->startBar);
                const juce::String endBarString = audioProcessor.getEndBarText(activeSheet->endBar);
                barRangeLabel.setText(
                    "Bars " + startBarString + " to " + endBarString,
                    juce::dontSendNotification);
                // Update the main editor with the content from the active sheet
                textEditor.setText(activeSheet->textEditor.getText());

                barRangeLabel.setVisible(true);
                textEditor.setVisible(true);
            }
        }
        
		// If playing, disable manual navigation buttons
        leftButton.setEnabled(false);
        rightButton.setEnabled(false);
    }
    else
    {
        leftButton.setEnabled(true);
		rightButton.setEnabled(true);
    }
    

    
        
    if(!activeSheet)
    {
        // No active sheet for this bar range
        barRangeLabel.setVisible(false);
        textEditor.setVisible(false);
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
    barRangeLabel.setBounds(area.removeFromTop(24));
    textEditor.setBounds(area.removeFromTop(area.getHeight() - 40));

    auto buttonArea = area;
    leftButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 4).reduced(5));
    addSheetButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 3).reduced(5));
    removeSheetButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2).reduced(5));
    rightButton.setBounds(buttonArea.reduced(5));
}

// This gets called either after clicking manually through the note sheets
// or when the playhead enters a different bar range
void NotepadAudioProcessorEditor::updateActiveSheet(BarRangeSheet* newSheet)
{
    if (newSheet)
    {
        currentlyActiveSheet = newSheet;

        juce::String endBarText = audioProcessor.getEndBarText(newSheet->endBar);

        barRangeLabel.setText(
            "Bars " + juce::String(newSheet->startBar) + " to " + juce::String(newSheet->endBar.value()),
            juce::dontSendNotification);
        textEditor.setText(newSheet->textEditor.getText());
        
        barRangeLabel.setVisible(true);
        textEditor.setVisible(true);
    }
        
}

BarRangeSheet* NotepadAudioProcessorEditor::getActiveSheet(int currentBar)
{
    for (auto& sheet : barRangeSheets)
    { 
        if (currentBar >= sheet->startBar)  // current bar is after this sheet's start bar
        {
            if (!sheet->endBar.has_value() ||                               // endBar = nullopt -> this is the last sheet
                (sheet->endBar.has_value() && currentBar <= sheet->endBar)) // current bar is before this sheet's end bar
                {
                    return sheet.get();
                }
        }
    }
    
    return nullptr;
}

void NotepadAudioProcessorEditor::addBarRangeSheet(int startBar)
{
    // Find out where to insert the new sheet
    int insertIndex = 0;
    while (insertIndex < barRangeSheets.size() && barRangeSheets[insertIndex]->startBar < startBar)
        ++insertIndex;

	// Check for duplicates by not allowing two sheets with the same startBar
	if (!barRangeSheets.empty())
	{
		// Check previous sheet if exists
		if (insertIndex > 0 &&
            insertIndex <= barRangeSheets.size() && 
            barRangeSheets[insertIndex - 1]->startBar == startBar)
			return;
			
		// Check next sheet if exists
		if (insertIndex < barRangeSheets.size() && 
            barRangeSheets[insertIndex]->startBar == startBar)
			return;
	}

    // Determine the end bar of new Sheet
    bool check = insertIndex < barRangeSheets.size();
    std::optional<int> endBar = std::nullopt;

    // Do we insert between two Sheets?
    if (insertIndex < barRangeSheets.size())
    {
        // Set end bar of new Sheet depending of next Sheet's start bar
        endBar = barRangeSheets[insertIndex]->startBar -1;
    }
        

    // Create BarRangeSheet and insert into the Editor's vector
    auto sheet = std::make_unique<BarRangeSheet>(startBar, endBar);
	barRangeSheets.insert(barRangeSheets.begin() + insertIndex, std::move(sheet));

    // Create BarRangeSheetData and insert into the Processor's vector
	NotepadAudioProcessor::BarRangeSheetData sheetData;
    sheetData.startBar = startBar;
    sheetData.endBar = endBar;
    sheetData.text = BarRangeSheet::getDefaultTextEditorText();
	audioProcessor.barRangeSheetData.insert(audioProcessor.barRangeSheetData.begin() + insertIndex, sheetData);

    // Update previous sheet's endBar
    if (insertIndex > 0)
    {
        barRangeSheets[insertIndex - 1]->endBar = startBar - 1;
        audioProcessor.barRangeSheetData[insertIndex - 1].endBar = startBar - 1;
    }

    // Update currently active sheet index
}

void NotepadAudioProcessorEditor::removeBarRangeSheet(int index)
{
    if (index < 0 || index > barRangeSheets.size())
        return;

	// Remove the sheet from the Editor's vector
    barRangeSheets.erase(barRangeSheets.begin() + index);
	// Remove the corresponding data from the Processor's vector
    audioProcessor.barRangeSheetData.erase(audioProcessor.barRangeSheetData.begin() + index);

    // Update previous sheets' endBars
    if (index > 0 && index < barRangeSheets.size())
    {
        barRangeSheets[index - 1]->endBar = barRangeSheets[index]->startBar - 1;
        audioProcessor.barRangeSheetData[index - 1].endBar = barRangeSheets[index]->startBar - 1;
    }
}
