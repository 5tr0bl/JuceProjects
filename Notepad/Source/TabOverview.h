/*
  ==============================================================================

    TabView.h
    Created: 30 Dec 2025 2:36:42pm
    Author:  Micha Strobl

    A helper to integrate JUCE's TabbedComponent into the project

    Add Tabs like this:
    addTab("BarRangeSheet", juce::Colours...,
            new BarRangeSheet(),
            true);
    This creates only one Sheet though, we need a collection of those!

    Access tabs like this later:
    auto* barRangeTab = dynamic_cast<BarRangeSheet*>(getTabContentComponent(0));

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TabulatureView.h"
#include "Notepad.h"

class TabOverview : public juce::TabbedComponent
{
public:
    TabOverview::TabOverview(NotepadAudioProcessor& p) : juce::TabbedComponent(TabbedButtonBar::Orientation::TabsAtLeft)
    {
        // Add the tab for the plain text notes..
        addTab("BarSheets", juce::Colours::transparentWhite,
                new Notepad(p),
                true, 0);

        // Add the tab for the guitar tab notes..
        addTab("Git. Tab", juce::Colours::transparentBlack,
                new TabulatureView(6),
                true/*, -1 */);
    };

    /*
    void resized() override
    {
        return;
    }
    */

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabOverview)
};