# JustChorus

Just a chorus plugin

## TODO

As of right now, the DSP BE is garbage, still i want to test it.

I have a new implementation that uses JUCE’s orc class to handle the LFO wave computation (as of right now we manually handle phase and sine computation per sample) and dsp::DelayLine<float> to handle the delay computation
