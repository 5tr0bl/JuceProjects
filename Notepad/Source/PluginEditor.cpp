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

    // Buttons
	addAndMakeVisible(addSheetButton);
    addAndMakeVisible(removeSheetButton);

	addSheetButton.setButtonText("Add Sheet");
	removeSheetButton.setButtonText("Remove Sheet");

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

    startTimerHz(30);
}

NotepadAudioProcessorEditor::~NotepadAudioProcessorEditor()
{
}

void NotepadAudioProcessorEditor::textEditorTextChanged(juce::TextEditor& editor)
{
    // Check if the modified TextEditor is the main TextEditor
	// Check if an active sheet is currently being used
    if (&editor == &textEditor && currentlyActiveSheet)
    {
		// Update the text in the currently active sheet
        currentlyActiveSheet->textEditor.setText(editor.getText());
    }
}

void NotepadAudioProcessorEditor::timerCallback()
{
	// Fetch currently active sheet
    const int currentBar = audioProcessor.getBarCount();
    BarRangeSheet* activeSheet = getActiveSheet(currentBar);

    // Check if active sheet has changed
	if (activeSheet != currentlyActiveSheet)
    {
        currentlyActiveSheet = activeSheet;

        if (activeSheet)
        {
            // Update the main editor with the content from the active sheet
            textEditor.setText(activeSheet->textEditor.getText());
            textEditor.setVisible(true);
        }
        else
        {
            // No active sheet for this bar range
            textEditor.setVisible(false);
        }
	}

    //textEditor.setText("Bar: " + String(currentBar));
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
    textEditor.setBounds(area.removeFromTop(area.getHeight() - 40));
    addSheetButton.setBounds(area.removeFromLeft(area.getWidth() / 2).reduced(5));
    removeSheetButton.setBounds(area.reduced(5));
}

BarRangeSheet* NotepadAudioProcessorEditor::getActiveSheet(int currentBar)
{
    for (auto& sheet : barRangeSheets)
    { 
        if (currentBar >= sheet->startBar && currentBar <= sheet->endBar)
            return sheet.get();
    }
    
    return nullptr;
}

void NotepadAudioProcessorEditor::addBarRangeSheet(int startBar)
{
    // Find out where to insert the new sheet
    int insertIndex = 0;
    while (insertIndex < barRangeSheets.size() && barRangeSheets[insertIndex]->startBar < startBar)
        ++insertIndex;

    // Determine the end bar
    int endBar = insertIndex > barRangeSheets.size()
        ? barRangeSheets[++insertIndex]->startBar - 1
        : audioProcessor.DEFAULT_END_BAR;

	// Create dummy text for the new sheet
	String dummyText = "Sheet for bars " + String(startBar) + " to " + String(endBar);

    // Create BarRangeSheet and insert ino the Editor's vector
    auto sheet = std::make_unique<BarRangeSheet>();
    sheet->startBar = startBar;
    sheet->endBar = endBar;
    sheet->textEditor.setText(dummyText);
    //sheet->textEditor.addListener(this);
	barRangeSheets.insert(barRangeSheets.begin() + insertIndex, std::move(sheet));

    // Create BarRangeSheetData and insert ino the Processor's vector
	NotepadAudioProcessor::BarRangeSheetData sheetData;
    sheetData.startBar = startBar;
    sheetData.endBar = endBar;
    sheetData.text = dummyText;
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
