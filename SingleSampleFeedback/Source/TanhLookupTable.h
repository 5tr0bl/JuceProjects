/*
  ==============================================================================

    1.	Define the Range and Resolution:
	tanh is typically used for values in the range [-3, 3] because values outside this range are almost saturated (tanh(x) approaches -1 or 1).
•	Choose a resolution (number of steps) for the table. For example, 1024 steps provide a good balance between accuracy and memory usage.
    
    2.	Precompute the Values:
•	Use a loop to calculate tanh for evenly spaced values in the range and store them in an array.
    
    3.	Access the Table:
•	Normalize the input value to the range of the table.
•	Use the normalized value to index into the table.
    
    4.	Interpolate for Higher Accuracy:
•	If the input value falls between two indices, use linear interpolation to improve accuracy.

  ==============================================================================
*/

#pragma once
#include <vector>
#include <cmath>
#include <cassert>

class TanhLookupTable
{
public:
    TanhLookupTable(float minValue, float maxValue, int resolution)
        : minValue(minValue), maxValue(maxValue), resolution(resolution)
    {
        assert(resolution > 1);
        stepSize = (maxValue - minValue) / (resolution - 1);
        table.resize(resolution);

        // Precompute tanh values
        for (int i = 0; i < resolution; ++i)
        {
            float x = minValue + i * stepSize;
            table[i] = std::tanh(x);
        }
    }

    float getValue(float x) const
    {
        // Clamp input to the range
        if (x <= minValue) return table.front();
        if (x >= maxValue) return table.back();

        // Normalize input to the table range
        float normalized = (x - minValue) / stepSize;
        int index = static_cast<int>(normalized);
        float frac = normalized - index;

        // Linear interpolation
        return table[index] + frac * (table[index + 1] - table[index]);
    }

private:
    float minValue, maxValue, stepSize;
    int resolution;
    std::vector<float> table;
};