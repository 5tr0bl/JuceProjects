/*
  ==============================================================================

    WaveTable.h
    
	The Sine and Saw Types are more or less taken from the JUCE tutorials.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class WaveTableBase : public juce::AudioProcessorValueTreeState::Listener
{
public:
	WaveTableBase() = default;
	virtual ~WaveTableBase() = default;
	virtual void prepareToPlay(double sampleRate, int samplesPerBlock) = 0;
	virtual void releaseResources();
	virtual float getNextSample() = 0;
	virtual void setFrequency(float frequency, double sampleRate) = 0;
	virtual void updateAngle();
	virtual float polyBLEP(float t); 
	// AudioProcessorValueTreeState::Listener
	void parameterChanged(const juce::String& parameterID, float newValue) override {}
};

class SineWaveTable : public WaveTableBase
{
public:
	SineWaveTable() : currentAngle(0.0f), angleDelta(0.0f) {}
	void prepareToPlay(double sampleRate, int samplesPerBlock) override {}
	void releaseResources() override {}
	float getNextSample() override;
	void setFrequency(float frequency, double sampleRate) override;
	void updateAngle() override;
private:
	float currentAngle;
	float angleDelta;
};

class SawWaveTable : public WaveTableBase
{
public:
	SawWaveTable(unsigned int tableSizeExt);
	void fillWavetable();
	void prepareToPlay(double sampleRate, int samplesPerBlock) override {}
	void releaseResources() override {}
	float getNextSample() override;
	float polyBLEP(float t) override;
	void setFrequency(float frequency, double sampleRate) override;
private:
	unsigned int tableSizeInternal = 0;
	AudioBuffer<float> wavetable;
	float currentIndex = 0.0f;
	float tableDelta = 0.0f;
};