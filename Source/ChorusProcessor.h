/*
  ==============================================================================
  
    ChorusProcessor.h
    Created: 29 Mar 2025 12:34:37pm
    Author:  Giuseppe Rivezzi
  
  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include <vector>

class ChorusProcessor
{
public:
    ChorusProcessor() = default;
    
    // Prepare the processor with the given specifications.
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    // Process a block of audio.
    void process(juce::dsp::ProcessContextReplacing<float>& context);
    
    // Reset internal state.
    void reset();
    
    // Update internal parameters from APVTS.
    void updateParameters(const juce::AudioProcessorValueTreeState& apvts);
    
    // Parameter setters.
    void setRate(float newRate);
    void setDepth(float newDepth);
    void setMix(float newMix);
    
    // Linear interpolation method (for reference). (DEPRECATED)
    float getInterpolatedSample(const float* buffer, int bufferSize, int index, float delayOffset);
    
    // Cubic interpolation method. (DEPRECATED)
    float getCubicInterpolatedSample(const float* buffer, int bufferSize, float delayIndex);
    
    // Band-limited interpolation method
    float getBandLimitedInterpolatedSample(const float* buffer, int bufferSize, float delayIndex);
    
private:
    // DSP variables.
    float sampleRate { 44100.0f };
    int numChannels { 2 };
    
    // Chorus parameters.
    float rate { 0.25f };    // LFO rate in Hz.
    float depth { 10.0f };   // Modulation depth in milliseconds.
    float mix { 0.5f };      // Wet/dry mix (0.0 to 1.0).
    float lfoPhase { 0.0f };  // LFO phase for chorus modulation.
    float smoothedLfoValue { 0.0f };
    float lfoSmoothingFactor { 0.05f }; // Controls the smoothness of LFO transitions.
    
    // Raw circular delay buffers – one per channel.
    //i did not use JUCE's delayline because it does not expose methods to access pointes in the buffer
    std::vector<std::vector<float>> delayBuffers;
    std::vector<int> writePositions;
    int maxDelaySamples { 0 };
    
    // JUCE DSP oscillator as the LFO.
    juce::dsp::Oscillator<float> lfo;
    
    // JUCE DSP low pass filter (to apply before the chorus algorithm).
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> lowPassFilter;
};
