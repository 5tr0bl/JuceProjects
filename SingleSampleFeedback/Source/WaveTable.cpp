/*
  ==============================================================================

    WaveTable.cpp
    Created: 16 Jul 2025 1:07:53am
    Author:  Micha

  ==============================================================================
*/

#include "WaveTable.h"

// Default definitions, as these are not pure virtual functions 
// and need not be defined in all derived classes
void WaveTableBase::releaseResources() {}
void WaveTableBase::updateAngle() {}
float WaveTableBase::polyBLEP(float) { return 0.0f; }

void SineWaveTable::setFrequency(float frequency, double sampleRate)
{
    auto cyclesPerSample = frequency / sampleRate;
    angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
}

void SineWaveTable::updateAngle()
{
    currentAngle += angleDelta;
    if (currentAngle >= juce::MathConstants<float>::twoPi)
        currentAngle -= juce::MathConstants<float>::twoPi;
}

float SineWaveTable::getNextSample()
{
    auto currentSample = std::sin(currentAngle);
    updateAngle();
    return currentSample;
}

SawWaveTable::SawWaveTable(unsigned int tableSizeExt)
{
    tableSizeInternal = (int)tableSizeExt - 1;
    wavetable.setSize(1, (int)tableSizeExt);
    fillWavetable();
}

void SawWaveTable::fillWavetable()
{
    auto* samples = wavetable.getWritePointer(0);
    auto angleDelta = juce::MathConstants<double>::twoPi / (double)(tableSizeInternal - 1);
    auto currentAngle = 0.0;

    for (unsigned int i = 0; i < tableSizeInternal; ++i)
    {
        // This one was weirdly proposed by ChatGPT
        //auto sample = 2.f * (float(i) / float(tableSizeInternal) - 0.5f); // Saw
        auto sample = 2.f * (float(i / tableSizeInternal) - floorf((1.f / 2.f) + (float(i) / float(tableSizeInternal)))); // Saw
        samples[i] = (float)sample;
        currentAngle += angleDelta;
    }
    samples[tableSizeInternal] = samples[0]; // Value of first & last sample identical
}

float SawWaveTable::getNextSample()
{
    auto* table = wavetable.getReadPointer(0);

    int index0 = static_cast<int>(currentIndex);
    int index1 = (index0 + 1) % tableSizeInternal;
    float frac = currentIndex - static_cast<float>(index0);
    float value0 = table[index0];
    float value1 = table[index1];

    // note that index will be increased AFTER this computation
    float currentSample = value0 + frac * (value1 - value0); // linear Interpolation

    float previousIndex = currentIndex;
    currentIndex += tableDelta;
    
    // increase index with wrap around table
    bool wrapped;
    if (currentIndex > static_cast<float>(tableSizeInternal))
    {
        currentIndex -= static_cast<float>(tableSizeInternal);
        wrapped = true;
    }

    if (wrapped)
    {
        float t = (previousIndex + tableDelta - (float)tableSizeInternal) / tableDelta; // normalized [0,1)
        currentSample -= polyBLEP(t); // subtract BLEP step to smooth transition
    }
    
    return currentSample;
}

/*
    PolyBLEP = Polynomial Band-Limited Step.
    Bandlimited polynomial correction as an attempt to avoid aliasing.
    Will be called when the index is near a discontinuity to smoothen the transition.
	So for a saw wave table this will happen when the index wraps around from the 
    last sample back to the first sample.
*/
float SawWaveTable::polyBLEP(float t)
{
    // t is the normalized time within the discontinuity window
    // t must be in [0, 1)
    if (t < 1.0f)
    {
        return t - t * t * 0.5f;
    }
    return 0.0f;
}

void SawWaveTable::setFrequency(float frequency, double sampleRate)
{
    auto tableSizeOverSampleRate = (float)tableSizeInternal / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}
