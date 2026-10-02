/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NotepadAudioProcessorEditor::NotepadAudioProcessorEditor (NotepadAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), tabOverview(p)
{
    setSize (400, 300);

    // Header containing the bar range
    //addAndMakeVisible(barRangeLabel);
    //barRangeLabel.setJustificationType(juce::Justification::centred);
    //barRangeLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(tabOverview);
    
    //addAndMakeVisible(textEditor);
    //textEditor.addListener(this);
    
	// Copy the BarRangeSheetData from the processor to the editor's barRangeSheets
    /*for(const auto& sheetData : audioProcessor.barRangeSheetData)
    {
        auto sheet = std::make_unique<BarRangeSheet>();
		sheet->startBar = sheetData.startBar;
        sheet->endBar = sheetData.endBar;
        sheet->textEditor.setText(sheetData.text);
        sheet->textEditor.addListener(this);
		barRangeSheets.push_back(std::move(sheet));
    }*/

    // Buttons for adding/removing note sheets
	//addAndMakeVisible(addSheetButton);
    //addAndMakeVisible(removeSheetButton);

	//addSheetButton.setButtonText("Add Sheet");
	//removeSheetButton.setButtonText("Remove Sheet");

	
    /*
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
    */

    // Buttons for manually sccrolling through note sheets
    //addAndMakeVisible(leftButton);
    //addAndMakeVisible(rightButton);

    //leftButton.setButtonText("<");
    //rightButton.setButtonText(">");

	
    /*
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

    */
}

NotepadAudioProcessorEditor::~NotepadAudioProcessorEditor()
{
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
    //barRangeLabel.setBounds(area.removeFromTop(24));
    //textEditor.setBounds(area.removeFromTop(area.getHeight() - 40));

    tabOverview.setBounds(area);
    
    // This left some space on the top
    //tabOverview.setBounds(area.removeFromTop(area.getHeight() - 40));
}

//void NotepadAudioProcessorEditor::addBarRangeSheet(int startBar)
//{
//    // Find out where to insert the new sheet
//    int insertIndex = 0;
//    while (insertIndex < barRangeSheets.size() && barRangeSheets[insertIndex]->startBar < startBar)
//        ++insertIndex;
//
//	// Check for duplicates by not allowing two sheets with the same startBar
//	if (!barRangeSheets.empty())
//	{
//		// Check previous sheet if exists
//		if (insertIndex > 0 &&
//            insertIndex <= barRangeSheets.size() && 
//            barRangeSheets[insertIndex - 1]->startBar == startBar)
//			return;
//			
//		// Check next sheet if exists
//		if (insertIndex < barRangeSheets.size() && 
//            barRangeSheets[insertIndex]->startBar == startBar)
//			return;
//	}
//
//    // Determine the end bar of new Sheet
//    bool check = insertIndex < barRangeSheets.size();
//    std::optional<int> endBar = std::nullopt;
//
//    // Do we insert between two Sheets?
//    if (insertIndex < barRangeSheets.size())
//    {
//        // Set end bar of new Sheet depending of next Sheet's start bar
//        endBar = barRangeSheets[insertIndex]->startBar -1;
//    }
//        
//
//    // Create BarRangeSheet and insert into the Editor's vector
//    auto sheet = std::make_unique<BarRangeSheet>(startBar, endBar);
//	barRangeSheets.insert(barRangeSheets.begin() + insertIndex, std::move(sheet));
//
//    // Create BarRangeSheetData and insert into the Processor's vector
//	NotepadAudioProcessor::BarRangeSheetData sheetData;
//    sheetData.startBar = startBar;
//    sheetData.endBar = endBar;
//    sheetData.text = BarRangeSheet::getDefaultTextEditorText();
//	audioProcessor.barRangeSheetData.insert(audioProcessor.barRangeSheetData.begin() + insertIndex, sheetData);
//
//    // Update previous sheet's endBar
//    if (insertIndex > 0)
//    {
//        barRangeSheets[insertIndex - 1]->endBar = startBar - 1;
//        audioProcessor.barRangeSheetData[insertIndex - 1].endBar = startBar - 1;
//    }
//
//    // Update currently active sheet index
//}
//
//void NotepadAudioProcessorEditor::removeBarRangeSheet(int index)
//{
//    if (index < 0 || index > barRangeSheets.size())
//        return;
//
//	// Remove the sheet from the Editor's vector
//    barRangeSheets.erase(barRangeSheets.begin() + index);
//	// Remove the corresponding data from the Processor's vector
//    audioProcessor.barRangeSheetData.erase(audioProcessor.barRangeSheetData.begin() + index);
//
//    // Update previous sheets' endBars
//    if (index > 0 && index < barRangeSheets.size())
//    {
//        barRangeSheets[index - 1]->endBar = barRangeSheets[index]->startBar - 1;
//        audioProcessor.barRangeSheetData[index - 1].endBar = barRangeSheets[index]->startBar - 1;
//    }
//}
