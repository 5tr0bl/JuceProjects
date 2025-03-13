/*
  ==============================================================================

    Lissajous.h
    Created: 21 Jan 2023 12:12:21am
    Author:  Micha

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <tuple>

namespace Gui 
{
    class Lissajous : public Component 
    {
    public:
        Lissajous()
        {
            // glow.setGlowProperties(16.f, Colours::fuchsia, Point(0, 0));
            // setComponentEffect(&glow); // very heavy on CPU right now ...
        }
        void paint(Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            g.setColour(Colours::darkblue.withAlpha(0.6f));
            g.fillEllipse(bounds);

            juce::AffineTransform toZero = juce::AffineTransform().translation(static_cast<float>(getWidth() / -2.f), static_cast<float>(getHeight() / -2.f));
            //juce::AffineTransform fromZero = toZero.inverted(); // might be too much computational effort
            juce::AffineTransform fromZero = juce::AffineTransform().translation(static_cast<float>(getWidth() / 2.f), static_cast<float>(getHeight() / 2.f));
            g.addTransform(fromZero);
            g.addTransform(rotation);
            g.addTransform(toZero);

            g.setColour(Colours::whitesmoke);
            auto [xAxis, yAxis] = setPaths(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
            g.fillPath(xAxis);
            g.fillPath(yAxis);

            g.setColour(Colours::orangered);
            //valueL = jmap(valueL, -1.f, 1.f, 0.f, static_cast<float>(getHeight() - 1.f));
            valueL = jmap(valueL, -1.f, 1.f, static_cast<float>(getHeight() - 1.f), 0.f);
            valueR = jmap(valueR, -1.f, 1.f, 0.f, static_cast<float>(getWidth() - 1.f));

            juce::Rectangle<float> pointArea(8, 8);
            pointArea.setCentre(valueR, valueL);
            g.fillEllipse(pointArea);
        }

        void setLeft(const float value) { valueL = value; }
        void setRight(const float value) { valueR = value; }
        std::tuple<Path, Path> setPaths(float width, float height)
        {
            juce::Path xAxis, yAxis;
            juce::Point<float> xStart = juce::Point<float>(0.f, (height / 2.f - 1.f));
            juce::Point<float> xEnd = juce::Point<float>((width - 1.f), (height / 2.f - 1.f));
            juce::Point<float> yStart = juce::Point<float>((width / 2.f - 1.f), (height - 1.f));
            juce::Point<float> yEnd = juce::Point<float>((width / 2.f - 1.f), 0.f);
            xAxis.startNewSubPath(xStart);
            xAxis.addArrow(juce::Line(xStart, xEnd), 2.f, 15.f, 10.f);
            xAxis.closeSubPath();
            yAxis.startNewSubPath(yStart);
            yAxis.addArrow(juce::Line(yStart, yEnd), 2.f, 15.f, 10.f);
            yAxis.closeSubPath();
            return { xAxis, yAxis };
        }

    private:
        float valueL = 0.0f;
        float valueR = 0.0f;
        juce::AffineTransform rotation = juce::AffineTransform().rotated(juce::degreesToRadians(315.f));
        juce::GlowEffect glow;
    };
}
