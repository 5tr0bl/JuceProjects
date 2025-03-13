/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Lissajous.h"
#include "CorrelationMeter.h"

//==============================================================================
/**
*/
class GoniometerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    GoniometerAudioProcessorEditor (GoniometerAudioProcessor&);
    ~GoniometerAudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GoniometerAudioProcessor& audioProcessor;

    Gui::Lissajous lissajous;
    Gonio::CorrelationMeter corr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GoniometerAudioProcessorEditor)
};
