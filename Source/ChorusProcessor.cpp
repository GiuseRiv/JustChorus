/*
  ==============================================================================

    ChorusProcessor.cpp
    Created: 29 Mar 2025 12:34:37pm
    Author:  Giuseppe Rivezzi

  ==============================================================================
*/


/*The process() method loops over each channel and sample:

It computes an LFO value using a sine wave, which modulates the delay time.

The delay time (in samples) is calculated from the LFO output scaled by the depth parameter.

A read position is computed (with wrapping) to fetch the delayed sample from the buffer.

The delayed sample is fetched using linear interpolation for smooth transitions.

The processed output is a blend of the original (dry) and delayed (wet) signals based on the mix parameter.

Finally, the current input sample is written into the delay buffer for future processing.
*/

#include "ChorusProcessor.h"

// Prepare the chorus processor (allocate delay buffer, reset states)
void ChorusProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    numChannels = spec.numChannels;
    maxDelaySamples = static_cast<int>(sampleRate * maxDelayTime);
    
    // Set up the delay buffer
    delayBuffer.setSize(numChannels, maxDelaySamples + spec.maximumBlockSize);
    delayBuffer.clear();

    writePosition = 0;
    lfoPhase = 0.0f;
}

// Update chorus parameters from the APVTS
void ChorusProcessor::updateParameters(const juce::AudioProcessorValueTreeState& apvts)
{
    setRate(*apvts.getRawParameterValue("rate"));
    setDepth(*apvts.getRawParameterValue("depth"));
    setMix(*apvts.getRawParameterValue("mix"));
}

// Main processing function
void ChorusProcessor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();
    const int numSamples = static_cast<int>(block.getNumSamples());

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = block.getChannelPointer(channel);
        float* delayData = delayBuffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Compute LFO modulation for delay time
            float lfoValue = std::sin(lfoPhase);
            float currentDelaySamples = (lfoValue * 0.5f + 0.5f) * (depth * sampleRate * 0.001f);

            // Compute read position in delay buffer
            int readPosition = writePosition - static_cast<int>(currentDelaySamples);
            if (readPosition < 0)
                readPosition += delayBuffer.getNumSamples();

            // Get interpolated delayed sample
            float delayedSample = getInterpolatedSample(delayData, delayBuffer.getNumSamples(), readPosition, currentDelaySamples);

            // Mix dry and wet signals
            float inputSample = channelData[sample];
            float outputSample = inputSample * (1.0f - mix) + delayedSample * mix;
            channelData[sample] = outputSample;

            // Store input sample in delay buffer
            delayData[writePosition] = inputSample;

            // Increment write position and update LFO phase
            if (++writePosition >= delayBuffer.getNumSamples())
                writePosition = 0;
            
            lfoPhase += juce::MathConstants<float>::twoPi * rate / sampleRate;
            if (lfoPhase > juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;
        }
    }
}

// Set new LFO rate
void ChorusProcessor::setRate(float newRate)
{
    rate = newRate;
}

// Set new modulation depth
void ChorusProcessor::setDepth(float newDepth)
{
    depth = newDepth;
}

// Set new wet/dry mix
void ChorusProcessor::setMix(float newMix)
{
    mix = newMix;
}

// Reset delay buffer and LFO phase
void ChorusProcessor::reset()
{
    delayBuffer.clear();
    writePosition = 0;
    lfoPhase = 0.0f;
}

// Linear interpolation for smooth delay processing
float ChorusProcessor::getInterpolatedSample(const float* buffer, int bufferSize, int index, float delayOffset)
{
    int index1 = index;
    int index2 = (index1 + 1) % bufferSize;
    float frac = delayOffset - static_cast<int>(delayOffset);
    return buffer[index1] * (1.0f - frac) + buffer[index2] * frac;
}
