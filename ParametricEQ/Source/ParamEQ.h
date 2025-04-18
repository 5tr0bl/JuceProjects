/*
  ==============================================================================

    ParamEQ.h
    Created: 19 Mar 2025 11:42:26pm
    Author:  Micha

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>

constexpr float PI = 3.14159265358979323846f;

enum FilterType
{
	LowPass,
	HighPass,
	BandPass,
	Notch,
	Peak,
	LowShelf,
	HighShelf
};
FilterType getFilterTypeFromString(const juce::String& filterTypeString);

class ParamEQ
{
public:

	void processBlock(juce::AudioBuffer<float>& buffer);

	void setBandwidth();
	void setCenterFrequency(float newCenterFrequency);
	void setFilterType(FilterType newFilterType);
	void setGain(float newGain);
	void setQ(float newQ);
	void setSampleRate(double newSampleRate);
	void setShelvingSlope();
	void update2ndOrderBuffer(int channel, float inputSample, float outputSample);

private:
	std::atomic<float> ampFactor; // dependent on gain
	float bandWidth; // for bandpass and notch filters
	std::atomic<float> centerFrequency;
	FilterType filterType;
	std::atomic<float> gain;
	std::atomic<float> q;
	double sampleRate;
	std::atomic<float> shelvingSlope; // for shelving EQs; dependent on Q

	std::vector<float> dnBuffer; // for 1st order allpass filter

	// for second order allpass filtered samples
	struct Ap2ndOrdBuffer
	{
		float x_1 = 0.f;
		float x_2 = 0.f;
		float y_1 = 0.f;
		float y_2 = 0.f;
	};
	std::vector<Ap2ndOrdBuffer> secondOrderBuffer;
	//std::mutex secondOrderBufferMutex; // mutex for thread safety
};