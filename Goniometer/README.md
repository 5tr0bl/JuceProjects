A simple JUCE project for self-tuition in terms of audio material visualisation

Consists of a Goniometer (or more common: Vector Scope) and a Correlation Meter

The Lissajous figure maps the sample values of the left/right channel to the rotated X- & Y-Axis

The Correlation Meter visualizes the phase difference between the two channels by 
- calculating the respective, complex FFTs
- taking the respective phase 
- and computing the correlation via arctan in the range of -1...0...1

Implemented and tested on Windows (Visual Studio 2022) with the aid of JUCE's AudioPluginHost
Open .jucer file and export to your IDE of choice

TO-DOs:
- Add a visual trace to the Goniometer's Dot to actually resemble a Lissajous figure
- Improve the way that AffineTransforms are used to orientate the Goniometer
- Add the Middle (or "Mono" Axis)
- Color Gradient on Correlation Meter
- Little Labels for "L", "R", "1", "1" ...
- Maybe come up with something for a multi-channel correlation meter?