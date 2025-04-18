/*
  ==============================================================================

    ParamEQ.cpp
    Created: 19 Mar 2025 11:42:26pm
    Author:  Micha

    LP and HP filters are implemented as 1st Order Allpass IIR filters.
	Bandpass and Notch filters are implemented as 2nd Order Allpass IIR filters.

  ==============================================================================
*/

#include "ParamEQ.h"

FilterType getFilterTypeFromString(const juce::String& filterTypeString)
{
    if (filterTypeString == "Low Pass") return FilterType::LowPass;
    if (filterTypeString == "High Pass") return FilterType::HighPass;
    if (filterTypeString == "Band Pass") return FilterType::BandPass;
    if (filterTypeString == "Notch") return FilterType::Notch;
    if (filterTypeString == "Peak") return FilterType::Peak;
    if (filterTypeString == "Low Shelf") return FilterType::LowShelf;
    if (filterTypeString == "High Shelf") return FilterType::HighShelf;

    // Default case
    return FilterType::LowPass;
}

void ParamEQ::processBlock(juce::AudioBuffer<float>& buffer)
{
    dnBuffer.resize(buffer.getNumChannels(), 0.f);
    secondOrderBuffer.resize(buffer.getNumChannels());

    // Auxiliaries, also consider making copies of parameters which might be changed by another thread
    // during the execution of this processBlock run-through!
	const auto centerFrequencyLocal = centerFrequency.load(std::memory_order_relaxed);
    const auto omega = 2.f * PI * centerFrequencyLocal / sampleRate; // center frequency in rad/s
	const auto ampFactorLocal = ampFactor.load(std::memory_order_relaxed);
	const auto qLocal = q.load(std::memory_order_relaxed);

    switch (filterType)
    {
        case FilterType::LowPass:
        {
            const auto sign = 1.f;
            const auto tan = std::tan(PI * centerFrequencyLocal / sampleRate);
            const auto a1 = (tan - 1.f) / (1.f + tan); // allpass component

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const auto x_n = channelData[sample];
                    const auto allpassFilteredSample = a1 * x_n + dnBuffer[channel];
                    dnBuffer[channel] = -a1 * allpassFilteredSample + x_n;

                    const auto filteredSample = 0.5f * (x_n + sign * allpassFilteredSample);
                    channelData[sample] = filteredSample;
                }
            }
            break;
        }
		case FilterType::HighPass:
		{
            const auto sign = -1.f;
			const auto tan = std::tan(PI * centerFrequencyLocal / sampleRate);
			const auto a1 = (tan - 1.f) / (1.f + tan); // allpass component

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
				auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const auto x_n = channelData[sample];
					const auto allpassFilteredSample = a1 * x_n + dnBuffer[channel];
					dnBuffer[channel] = -a1 * allpassFilteredSample + x_n;
                        
					const auto filteredSample = 0.5f * (x_n + sign * allpassFilteredSample);
					channelData[sample] = filteredSample;
				}
            }
            break;
		}
        case FilterType::BandPass:
        {
			// save tangens result to variable 
            auto tan = std::tan(PI * bandWidth / (2.f * sampleRate));
            // c only depends on the bandwidth
            auto c = (tan - 1.f) / (tan + 1.f);
            // d only depends on the center frequency
			auto d = -1.f * (std::cos(2.f * PI * centerFrequencyLocal / sampleRate));

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
					const auto x_n = channelData[sample]; // current input sample

                    const auto allpassFilteredSample = (-1.f * c * x_n) // b_0
                        + d * (1.f - c) * secondOrderBuffer[channel].x_1 // b_1
                        + 1.f * secondOrderBuffer[channel].x_2 // b_2
                        - d * (1.f - c) * secondOrderBuffer[channel].y_1 // a_1
                        - (-1.f * c) * secondOrderBuffer[channel].y_2; // a_2

                    const auto filteredSample = 0.5f * (x_n - allpassFilteredSample);
                    channelData[sample] = filteredSample;

                    // push current input/output samples to ap 2nd order buffer
					secondOrderBuffer[channel].x_2 = secondOrderBuffer[channel].x_1;
					secondOrderBuffer[channel].x_1 = x_n;
					secondOrderBuffer[channel].y_2 = secondOrderBuffer[channel].y_1;
					secondOrderBuffer[channel].y_1 = allpassFilteredSample;
                }
            }
            break;
        }
		case FilterType::Notch:
		{
            // save tangens result to variable 
            auto tan = std::tan(PI * bandWidth / (2.f * sampleRate));
            // c only depends on the bandwidth
            auto c = (tan - 1.f) / (tan + 1.f);
            // d only depends on the center frequency
            auto d = -1.f * (std::cos(2.f * PI * centerFrequencyLocal / sampleRate));

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const auto x_n = channelData[sample]; // current input sample

                    const auto allpassFilteredSample = (-1.f * c * x_n) // b_0
                        + d * (1.f - c) * secondOrderBuffer[channel].x_1 // b_1
                        + 1.f * secondOrderBuffer[channel].x_2 // b_2
                        - d * (1.f - c) * secondOrderBuffer[channel].y_1 // a_1
                        - (-1.f * c) * secondOrderBuffer[channel].y_2; // a_2

					// -PI phase cancellation at center freq by adding AP-filtered sample to input sample
                    const auto filteredSample = 0.5f * (x_n + allpassFilteredSample);
                    channelData[sample] = filteredSample;

                    // push current input/output samples to 2nd order buffer
                    secondOrderBuffer[channel].x_2 = secondOrderBuffer[channel].x_1;
                    secondOrderBuffer[channel].x_1 = x_n;
                    secondOrderBuffer[channel].y_2 = secondOrderBuffer[channel].y_1;
                    secondOrderBuffer[channel].y_1 = allpassFilteredSample;
                }
            }
            break;
		}
        case FilterType::Peak:
        {
			const auto alpha = std::sin(omega) / (2.f * q.load(std::memory_order_relaxed));
            const auto a0 = 1.f + alpha / ampFactorLocal; // will be used for normalization
            const auto a1 = -2.f * std::cos(omega);
            const auto a2 = 1.f - alpha / ampFactorLocal;
            const auto b0 = 1.f + alpha * ampFactorLocal;
            const auto b1 = -2.f * std::cos(omega);
            const auto b2 = 1.f - alpha * ampFactorLocal;

            for (auto channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
                {
					const auto x_n = channelData[sample]; // current input sample

                    const auto filteredSample = (b0 / a0) * x_n
                        + (b1 / a0) * secondOrderBuffer[channel].x_1
                        + (b2 / a0) * secondOrderBuffer[channel].x_2
                        - (a1 / a0) * secondOrderBuffer[channel].y_1
                        - (a2 / a0) * secondOrderBuffer[channel].y_2;

                    channelData[sample] = filteredSample;

                    // push current input/output samples to 2nd order buffer
                    secondOrderBuffer[channel].x_2 = secondOrderBuffer[channel].x_1;
                    secondOrderBuffer[channel].x_1 = x_n;
                    secondOrderBuffer[channel].y_2 = secondOrderBuffer[channel].y_1;
                    secondOrderBuffer[channel].y_1 = filteredSample;
                }
            }
            break;
        }
		case FilterType::LowShelf:
		{
            /*
            // Original approach from the cookbook using the S variable
            const auto alpha = sin(omega) / 2 
                * sqrt((ampFactorLocal + 1 / ampFactorLocal) * (1 / shelvingSlope - 1) + 2);
            */
			const auto alpha = std::sin(omega) / (2.f * qLocal);
			const auto cosOmega = std::cos(omega);
            const auto auxOne = 2 * sqrt(ampFactorLocal) * alpha;

            const auto b0 = ampFactorLocal * ((ampFactorLocal + 1) - (ampFactorLocal - 1) * cosOmega + auxOne);
            const auto b1 = 2 * ampFactorLocal * ((ampFactorLocal - 1) - (ampFactorLocal + 1) * cosOmega);
            const auto b2 = ampFactorLocal * ((ampFactorLocal + 1) - (ampFactorLocal - 1) * cosOmega - auxOne);
            const auto a0 = (ampFactorLocal + 1) + (ampFactorLocal - 1) * cosOmega + auxOne;
            const auto a1 = -2 * ((ampFactorLocal - 1) + (ampFactorLocal + 1) * cosOmega);
            const auto a2 = (ampFactorLocal + 1) + (ampFactorLocal - 1) * cosOmega - auxOne;

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const auto x_n = channelData[sample]; // current input sample

                    const auto filteredSample = (b0 / a0) * x_n
                        + (b1 / a0) * secondOrderBuffer[channel].x_1
                        + (b2 / a0) * secondOrderBuffer[channel].x_2
                        - (a1 / a0) * secondOrderBuffer[channel].y_1
                        - (a2 / a0) * secondOrderBuffer[channel].y_2;

                    channelData[sample] = filteredSample;

                    // push current input/output samples to 2nd order buffer
                    secondOrderBuffer[channel].x_2 = secondOrderBuffer[channel].x_1;
                    secondOrderBuffer[channel].x_1 = x_n;
                    secondOrderBuffer[channel].y_2 = secondOrderBuffer[channel].y_1;
                    secondOrderBuffer[channel].y_1 = filteredSample;
                }
            }
            break;
		}
		case FilterType::HighShelf:
        {
            /*
            // Original approach from the cookbook using the S variable
            const auto alpha = sin(omega) / 2
                * sqrt((ampFactorLocal + 1 / ampFactorLocal) * (1 / shelvingSlope - 1) + 2);
            */
            const auto alpha = std::sin(omega) / (2.f * qLocal);
            const auto cosOmega = std::cos(omega);
            const auto auxOne = 2 * sqrt(ampFactorLocal) * alpha;
			
            const auto b0 = ampFactorLocal * ((ampFactorLocal + 1) + (ampFactorLocal - 1) * cosOmega + auxOne);
            const auto b1 = -2 * ampFactorLocal * ((ampFactorLocal - 1) + (ampFactorLocal + 1) * cosOmega);
            const auto b2 = ampFactorLocal * ((ampFactorLocal + 1) + (ampFactorLocal - 1) * cosOmega - auxOne);
            const auto a0 = (ampFactorLocal + 1) - (ampFactorLocal - 1) * cosOmega + auxOne;
            const auto a1 = 2 * ((ampFactorLocal - 1) - (ampFactorLocal + 1) * cosOmega);
            const auto a2 = (ampFactorLocal + 1) - (ampFactorLocal - 1) * cosOmega - auxOne;

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const auto x_n = channelData[sample]; // current input sample
                    
                    const auto filteredSample = (b0 / a0) * x_n
                        + (b1 / a0) * secondOrderBuffer[channel].x_1
                        + (b2 / a0) * secondOrderBuffer[channel].x_2
                        - (a1 / a0) * secondOrderBuffer[channel].y_1
                        - (a2 / a0) * secondOrderBuffer[channel].y_2;

                    channelData[sample] = filteredSample;

					update2ndOrderBuffer(channel, x_n, filteredSample);
                }
            }            
            break;
        }
    }
}

/* For Bandpass and Bandstop Filters */
void ParamEQ::setBandwidth()
{
    bandWidth = centerFrequency.load(std::memory_order_relaxed) / q.load(std::memory_order_relaxed);
}

void ParamEQ::setCenterFrequency(float newCenterFrequency)
{
    centerFrequency.store(newCenterFrequency, std::memory_order_relaxed);
}

void ParamEQ::setFilterType(FilterType newFilterType)
{
    /*
	    stop playback? update state, coeffs?
    */
	//secondOrderBuffer.clear();
    filterType = newFilterType;
}

void ParamEQ::setGain(float newGain)
{
    gain.store(newGain, std::memory_order_relaxed);
    ampFactor.store(std::pow(10.f, newGain / 40.f), std::memory_order_relaxed); // A = sqrt(10^(gain/20))
}

/**
*   BW and S are re-calculated when Q is changed
*/
void ParamEQ::setQ(float newQ)
{
    q.store(newQ, std::memory_order_relaxed);
    setBandwidth();
    setShelvingSlope();
}

void ParamEQ::setSampleRate(double newSampleRate)
{
	sampleRate = newSampleRate;
}

/**
*   The relationship between shelf slope S and Q is
*   1 / Q = sqrt((A + 1 / A) * (1 / S - 1) + 2)
*/
void ParamEQ::setShelvingSlope()
{
	const auto ampFactorLocal = ampFactor.load(std::memory_order_relaxed);
    float ampSum = ampFactorLocal + 1.0f / ampFactorLocal;
    float denominator = ampSum + 1.0f / (q.load() * q.load()) - 2.0f;

	if (denominator != 0.0f)
	{
        shelvingSlope.store(ampSum / denominator, std::memory_order_relaxed);
	}
	else
	{
        shelvingSlope.store(0.0f, std::memory_order_relaxed); // Avoid division by zero
	}
}

/**
*   
*/
void ParamEQ::update2ndOrderBuffer(int channel, float inputSample, float outputSample)
{
    //std::lock_guard<std::mutex> lock(secondOrderBufferMutex); // try locking 

    // push current input/output samples to 2nd order buffer
    secondOrderBuffer[channel].x_2 = secondOrderBuffer[channel].x_1;
    secondOrderBuffer[channel].x_1 = inputSample;
    secondOrderBuffer[channel].y_2 = secondOrderBuffer[channel].y_1;
    secondOrderBuffer[channel].y_1 = outputSample;
}
