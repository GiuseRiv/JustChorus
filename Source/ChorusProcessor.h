/*
  ==============================================================================

    ChorusProcessor.h
    Created: 29 Mar 2025 12:34:37pm
    Author:  Giuseppe Rivezzi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ChorusProcessor
{
public:
    ChorusProcessor() = default;

    // Prepare the processor with the given specifications
    void prepare(const juce::dsp::ProcessSpec& spec);

    // Process a block of audio
    void process(juce::dsp::ProcessContextReplacing<float>& context);

    // Reset internal processing state
    void reset();

    // Update internal parameters from the APVTS (AudioProcessorValueTreeState)
    void updateParameters(const juce::AudioProcessorValueTreeState& apvts);

    // Parameter setters
    void setRate(float newRate);
    void setDepth(float newDepth);
    void setMix(float newMix);

private:
    // Helper function for interpolated sample fetching
    float getInterpolatedSample(const float* buffer, int bufferSize, int index, float delayOffset);

    // DSP and state variables
    float sampleRate { 44100.0f };
    int numChannels { 2 };
    int maxDelaySamples { 0 };
    float maxDelayTime { 0.05f }; // Maximum delay time in seconds (50ms)

    // Chorus parameters
    float rate { 0.25f };   // LFO rate in Hz
    float depth { 10.0f };  // Modulation depth in milliseconds
    float mix { 0.5f };     // Wet/dry mix (0.0 to 1.0)

    // Internal processing state
    juce::AudioBuffer<float> delayBuffer;
    int writePosition { 0 };
    float lfoPhase { 0.0f };
};
