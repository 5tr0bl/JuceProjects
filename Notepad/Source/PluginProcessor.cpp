/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NotepadAudioProcessor::NotepadAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Initialize the member atomics
    barCount.store(0);
	isPlaying.store(false);

    // Create dummy BarRangeSheetData to have a first element
    BarRangeSheetData dummy;
    dummy.startBar = 1;
    dummy.endBar = DEFAULT_END_BAR;
    dummy.text = "Enter your notes here...";
    barRangeSheetData.push_back(dummy);
}

NotepadAudioProcessor::~NotepadAudioProcessor()
{
}

//==============================================================================
const juce::String NotepadAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NotepadAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NotepadAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NotepadAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NotepadAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NotepadAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NotepadAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NotepadAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NotepadAudioProcessor::getProgramName (int index)
{
    return {};
}

void NotepadAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NotepadAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NotepadAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NotepadAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NotepadAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

	// Only call for the transport playhead in processBlock, according to the JUCE documentation
	// In standalone mode, the playhead will be a nullptr
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            auto barCountOpt = position->getBarCount();
            auto ppqPos = position->getPpqPosition();

            // set the isPlaying state for the Editor to fetch it
            isPlaying.store(position->getIsPlaying());

            // Easiest way. Not all DAWs support this. (e.g. Reaper)
            if (barCountOpt.hasValue())
                barCount.store(*barCountOpt);
            else if (ppqPos.hasValue())
            {
				auto timeSig = position->getTimeSignature();
                if (timeSig.hasValue())
                {
                    double beatsPerBar = 4.0 * timeSig->numerator / timeSig->denominator;
                    // PPQ position divided by beats per bar equals the bar number
                    // Add 1 because bars typically start at 1, not 0
                    int calculatedBar = static_cast<int>((*ppqPos / beatsPerBar) + 1);
                    barCount.store(calculatedBar);
                }
            }
            else
                barCount.store(-1);
        }
        else
        {
			DBG("Playhead position not available.");
        }
    }
    
    /*
    auto totalNumInputChannels  = getTotalNumInputChannels();
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    */
}

//==============================================================================
bool NotepadAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NotepadAudioProcessor::createEditor()
{
    return new NotepadAudioProcessorEditor (*this);
}

// Remember that barRangeSheetData is the processors representation of the bar ranges
// the Editor has its own representation with TextEditors
void NotepadAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml("BarRangeSheetData");
    for (const auto& sheet : barRangeSheetData)
    {
        juce::XmlElement* sheetXml = xml.createNewChildElement("Sheet");
        sheetXml->setAttribute("startBar", sheet.startBar);
        sheetXml->setAttribute("endBar", sheet.endBar);
        sheetXml->setAttribute("text", sheet.text);
    }
    copyXmlToBinary(xml, destData);
}

void NotepadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName("BarRangeSheetData"))
    {
        barRangeSheetData.clear();
        forEachXmlChildElement(*xmlState, sheetXml)
        {
            if (sheetXml->hasTagName("Sheet"))
            {
                BarRangeSheetData sheet;
                sheet.startBar = sheetXml->getIntAttribute("startBar");
                sheet.endBar = sheetXml->getIntAttribute("endBar");
                sheet.text = sheetXml->getStringAttribute("text");
                barRangeSheetData.push_back(sheet);
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NotepadAudioProcessor();
}
