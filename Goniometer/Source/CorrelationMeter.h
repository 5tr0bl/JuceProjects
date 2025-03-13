/*
  ==============================================================================

    CorrelationMeter.h
    Created: 25 Jan 2023 11:44:42pm
    Author:  Micha

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <math.h>
#include <complex.h>

#define PI 3.14159265

namespace Gonio
{
    class CorrelationMeter : public Component
    {
    public:
        CorrelationMeter()
        {
            auto bounds = getLocalBounds().toFloat();
            width = static_cast<float>(getWidth());
            height = static_cast<float>(getHeight());
            
            // Draw -1 and 1
            /*
            addAndMakeVisible(minusOne);
            addAndMakeVisible(one);
            minusOne.setFont(juce::Font(8.0f, juce::Font::bold));
            one.setFont(juce::Font(8.0f, juce::Font::bold));
            minusOne.setText("-1", juce::dontSendNotification);
            one.setText("1", juce::dontSendNotification);
            minusOne.setCentrePosition(10, height / 2);
            minusOne.setCentrePosition(width - 10, height / 2);
            */
        }

        void paint(Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            width = static_cast<float>(getWidth());
            height = static_cast<float>(getHeight());
            //g.setColour(Colours::black.withAlpha(0.6f));
            
            ColourGradient green = ColourGradient(startColor, 0, 0,
                endColor, width - 1.f, height / 2.f, false);

            g.setGradientFill(green);
            //g.fillRect(bounds);
            g.fillRoundedRectangle(bounds, 1);

            
            // Draw -1 and 1 Text Labels
            g.setFont(font);
            g.setColour(labelColor);
            g.drawText(minusOne, bounds, juce::Justification::topLeft, true);
            g.drawText(one, bounds, juce::Justification::topRight, true);

            // Draw Slider
            g.setColour(Colours::white);
            auto sliderPos = jmap(correlation, -1.f, 1.f, 0.f, width);
            g.fillRoundedRectangle(sliderPos, 0, 5, height, 5);
        }

        void getCorrelation(std::vector<std::complex<float>>& left, std::vector<std::complex<float>>& right)
        {
            jassert(left[0] == left[0] && right[0] == right[0]);    // NaN check
            jassert(left.size() == right.size());
            int N = left.size();
            phaseCorrelationBins.clear();
            phaseCorrelationBins.resize(N);
            correlation = 0.0f;
            // take the phase per channel for each freq bin --- or just for half of the FFT vector??
            for (int freqBin = 0; freqBin < N / 2; ++freqBin)
            {
                auto* leftBin = &left[freqBin];
                auto* rightBin = &right[freqBin];
                
                // Normalize the magnitudes
                if(abs(*leftBin) > 0.0f) *leftBin /= abs(*leftBin);
                if (abs(*rightBin) > 0.0f) *rightBin /= abs(*rightBin);
                
                // Compute the element-wise product of the two normalized magnitudes
                std::complex<float> product = *leftBin * std::conj(*rightBin);
                /*  The sin function maps the range of the phase angle from [-π, π] radians to [-1, 1]
                    with values near -1 representing large phase differences
                    and values near 1 representing similar phases. */
                phaseCorrelationBins[freqBin] = std::sin(std::arg(product));
                correlation += std::sin(std::arg(product));
            }
            
            // get mean value over all phase correlations in -1...1 range
            correlation /= static_cast<float>(N / 2);

            // take the mean value of the correlation and map from radians to [-1;1]
            // correlation = jmap(correlation / static_cast<float>(N), static_cast<float>(-PI), static_cast<float>(PI), -1.f, 1.f);
        }

    private:        
        float phaseLeft = 0.0f;
        float phaseRight = 0.0f;
        float width, height, correlation = 0.0f;
        std::vector<float> phaseCorrelationBins;
        juce::Colour startColor = juce::Colours::red;
        juce::Colour endColor = juce::Colours::forestgreen;
        juce::Colour labelColor = juce::Colours::darkgrey;
        juce::Font font = juce::Font(18.0f);
        juce::String minusOne = "-1";
        juce::String one = "1";
        //ColourGradient green, red;
    };
}