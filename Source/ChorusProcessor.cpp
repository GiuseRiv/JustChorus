/*
  ==============================================================================
  
    ChorusProcessor.cpp
    Created: 29 Mar 2025 12:34:37pm
    Author:  Giuseppe Rivezzi
  
  ==============================================================================
*/

#include "ChorusProcessor.h"

void ChorusProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    numChannels = spec.numChannels;
    
    // Initialize LFO: sine wave
    lfo.initialise([](float x) { return std::sin(x); });
    lfo.setFrequency(rate);
    lfo.prepare(spec);
    
    // Set up a delay line for each channel.
    // The maximum delay is depth (in ms) converted to samples.
    const float maxDelayInSeconds = (depth * 0.001f) + 0.05f; // add a small margin (e.g., 50 ms)
    const int maxDelaySamples = static_cast<int>(sampleRate * maxDelayInSeconds);
    
    delayLines.clear();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* delayLine = new juce::dsp::DelayLine<float>(static_cast<size_t>(maxDelaySamples));
        delayLine->prepare(spec);
        delayLine->reset();
        delayLines.add(delayLine);
    }
}

void ChorusProcessor::updateParameters(const juce::AudioProcessorValueTreeState& apvts)
{
    setRate(*apvts.getRawParameterValue("rate"));
    setDepth(*apvts.getRawParameterValue("depth"));
    setMix(*apvts.getRawParameterValue("mix"));
    lfo.setFrequency(rate);
}

void ChorusProcessor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();
    const int numSamples = static_cast<int>(block.getNumSamples());

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = block.getChannelPointer(ch);
        
        // Ensure the delay line exists for this channel
        if (ch >= delayLines.size() || delayLines[ch] == nullptr)
            continue;

        auto* delayLine = delayLines[ch];

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Generate LFO value per sample (Sine wave modulation)
            float lfoValue = std::sin(lfoPhase);
            
            // Convert LFO output to delay time in samples
            float delayTimeSamples = (lfoValue * 0.5f + 0.5f) * (depth * sampleRate * 0.001f);

            // Set the delay time dynamically
            delayLine->setDelay(delayTimeSamples);

            // Process the sample through the delay line
            float delayedSample = delayLine->popSample(0, delayTimeSamples);

            // Mix dry and wet signals
            float inputSample = channelData[sample];
            channelData[sample] = inputSample * (1.0f - mix) + delayedSample * mix;

            // Store the current input sample in the delay line
            delayLine->pushSample(0, inputSample);

            // Update LFO phase
            lfoPhase += juce::MathConstants<float>::twoPi * rate / sampleRate;

            // Keep `lfoPhase` in the range [0, 2π]
            if (lfoPhase > juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;
        }
    }
}



void ChorusProcessor::reset()
{
    lfo.reset();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        if (auto* delayLine = delayLines[ch])
            delayLine->reset();
    }
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
