/*
  ==============================================================================
  
    ChorusProcessor.cpp
    Created: 29 Mar 2025 12:34:37pm
    Author:  Giuseppe Rivezzi
  
  ==============================================================================
*/
#include "ChorusProcessor.h"
#include <cmath>

// Linear interpolation (for reference)
float ChorusProcessor::getInterpolatedSample(const float* buffer, int bufferSize, int index, float delayOffset)
{
    int index1 = index;
    int index2 = (index1 + 1) % bufferSize;
    float frac = delayOffset - std::floor(delayOffset);
    return buffer[index1] * (1.0f - frac) + buffer[index2] * frac;
}

// Cubic interpolation using four samples.
float ChorusProcessor::getCubicInterpolatedSample(const float* buffer, int bufferSize, float delayIndex)
{
    int index = static_cast<int>(std::floor(delayIndex));
    float frac = delayIndex - index;

    // A lambda to safely wrap around the buffer indices.
    auto getWrappedSample = [buffer, bufferSize](int i) -> float {
        while (i < 0)
            i += bufferSize;
        while (i >= bufferSize)
            i -= bufferSize;
        return buffer[i];
    };

    float y0 = getWrappedSample(index - 1);
    float y1 = getWrappedSample(index);
    float y2 = getWrappedSample(index + 1);
    float y3 = getWrappedSample(index + 2);

    // Cubic interpolation (Catmull-Rom spline variant).
    float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float a2 = -0.5f * y0 + 0.5f * y2;
    float a3 = y1;

    return ((a0 * frac + a1) * frac + a2) * frac + a3;
}

void ChorusProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    // Store sample rate and channel count.
    sampleRate = spec.sampleRate;
    numChannels = spec.numChannels;
    
    // --- Low Pass Filter Setup ---
    const float cutoffFrequency = 5000.0f;
    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffFrequency);
    *lowPassFilter.state = *coefficients;
    lowPassFilter.prepare(spec);
    
    // --- LFO Setup ---
    lfoPhase = 0.0f;
    lfo.initialise([](float x) { return std::sin(x); });
    lfo.setFrequency(rate);
    lfo.prepare(spec);
    
    // --- Delay Buffer Setup ---
    // Calculate maximum delay in samples (depth in ms plus a margin).
    maxDelaySamples = static_cast<int>((depth * 0.001f + 0.05f) * sampleRate);
    
    // Resize and initialize raw delay buffers and write positions.
    delayBuffers.clear();
    delayBuffers.resize(numChannels);
    writePositions.clear();
    writePositions.resize(numChannels, 0);
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        delayBuffers[ch].assign(maxDelaySamples, 0.0f);
    }
}

void ChorusProcessor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    // First, process the signal through the low pass filter.
    lowPassFilter.process(context);
    
    auto block = context.getOutputBlock();
    int numSamples = static_cast<int>(block.getNumSamples());
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = block.getChannelPointer(ch);
        auto& buffer = delayBuffers[ch];
        int& writePos = writePositions[ch];
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Generate the LFO value and smooth it.
            float lfoValue = std::sin(lfoPhase);
            smoothedLfoValue += (lfoValue - smoothedLfoValue) * lfoSmoothingFactor;
            
            // Calculate the delay time in samples.
            float delayTimeSamples = (smoothedLfoValue * 0.5f + 0.5f) * (depth * sampleRate * 0.001f);
            
            // Calculate the read position (taking the circular buffer wrap into account).
            float readPos = static_cast<float>(writePos) - delayTimeSamples;
            if (readPos < 0)
                readPos += maxDelaySamples;
            
            // Retrieve delayed sample using cubic interpolation.
            float delayedSample = getCubicInterpolatedSample(buffer.data(), maxDelaySamples, readPos);
            
            // Mix the dry and wet signals.
            float inputSample = channelData[sample];
            channelData[sample] = inputSample * (1.0f - mix) + delayedSample * mix;
            
            // Write the current sample into the delay buffer.
            buffer[writePos] = inputSample;
            writePos = (writePos + 1) % maxDelaySamples;
            
            // Update the LFO phase.
            lfoPhase += juce::MathConstants<float>::twoPi * rate / sampleRate;
            if (lfoPhase > juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;
        }
    }
}

void ChorusProcessor::reset()
{
    // Reset filter and LFO states.
    lowPassFilter.reset();
    lfo.reset();
    
    // Clear delay buffers and reset write positions.
    for (int ch = 0; ch < numChannels; ++ch)
    {
        std::fill(delayBuffers[ch].begin(), delayBuffers[ch].end(), 0.0f);
        writePositions[ch] = 0;
    }
}

void ChorusProcessor::updateParameters(const juce::AudioProcessorValueTreeState& apvts)
{
    setRate(*apvts.getRawParameterValue("rate"));
    setDepth(*apvts.getRawParameterValue("depth"));
    setMix(*apvts.getRawParameterValue("mix"));
    lfo.setFrequency(rate);
}

void ChorusProcessor::setRate(float newRate)
{
    rate = newRate;
}

void ChorusProcessor::setDepth(float newDepth)
{
    depth = newDepth;
}

void ChorusProcessor::setMix(float newMix)
{
    mix = newMix;
}
