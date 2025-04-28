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
//DEPRECATED
float ChorusProcessor::getInterpolatedSample(const float* buffer, int bufferSize, int index, float delayOffset)
{
    int index1 = index;
    int index2 = (index1 + 1) % bufferSize;
    float frac = delayOffset - std::floor(delayOffset);
    return buffer[index1] * (1.0f - frac) + buffer[index2] * frac;
}

// Cubic interpolation using four samples.
//DEPRECATED
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

float ChorusProcessor::getBandLimitedInterpolatedSample(const float* buffer, int bufferSize, float delayIndex)
{
    const int kernelRadius = 8; // Half-width of the interpolation kernel
    float result = 0.0f;
    float sum = 0.0f;
    
    int baseIndex = static_cast<int>(std::floor(delayIndex));
    float frac = delayIndex - baseIndex;
    
    // Loop over the kernel window
    for (int i = -kernelRadius; i <= kernelRadius; i++)
    {
        // Compute the sample index (wrap around if necessary)
        int sampleIndex = baseIndex + i;
        while (sampleIndex < 0)
            sampleIndex += bufferSize;
        while (sampleIndex >= bufferSize)
            sampleIndex -= bufferSize;
        
        // x is the distance from the actual delay position
        float x = static_cast<float>(i) - frac;
        
        // Compute the sinc function; handle x==0 to avoid division by zero
        float sincValue = (std::abs(x) < 1e-6f) ? 1.0f : std::sin(M_PI * x) / (M_PI * x);
        
        // Apply a Hann window to taper the sinc function.
        // The Hann window is defined over [-kernelRadius, kernelRadius].
        float window = 0.5f * (1.0f + std::cos((M_PI * x) / kernelRadius));
        
        float weight = sincValue * window;
        result += buffer[sampleIndex] * weight;
        sum += weight;
    }
    
    // Normalize to preserve amplitude
    return result / sum;
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
    
    //per oversampling
    int factor = 4; // 4x oversampling 
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        numChannels,
        static_cast<int>(std::log2(factor)),
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true
    );
    oversampler->reset();
    oversampler->initProcessing(static_cast<size_t>(spec.maximumBlockSize));
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        delayBuffers[ch].assign(maxDelaySamples, 0.0f);
    }
}

void ChorusProcessor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    auto oversampledBlock = oversampler->processSamplesUp(context.getOutputBlock());

    juce::dsp::AudioBlock<float> block(oversampledBlock);
    auto sampleRateOS = sampleRate * oversampler->getOversamplingFactor();
    auto numSamples = static_cast<int>(block.getNumSamples());

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = block.getChannelPointer(ch);
        auto& buffer = delayBuffers[ch];
        int& writePos = writePositions[ch];

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float rawLfo = std::sin(lfoPhase);
            smoothedLfoValue = lfoSmoothCoeff * rawLfo + (1.0f - lfoSmoothCoeff) * smoothedLfoValue;

            float modDepthFactor = 0.4f;
            float baseDelaySamples = depth * sampleRateOS * 0.001f;
            float modulator = ((smoothedLfoValue * 0.5f) + 0.5f) * modDepthFactor;
            float delayTimeSamples = baseDelaySamples * modulator;

            float readPos = static_cast<float>(writePos) - delayTimeSamples;
            if (readPos < 0)
                readPos += maxDelaySamples;

            float delayedSample = getBandLimitedInterpolatedSample(buffer.data(), maxDelaySamples, readPos);

            float dryMix = 1.0f - mix * 0.8f;
            float wetMix = mix;
            float inputSample = channelData[sample];
            channelData[sample] = inputSample * dryMix + delayedSample * wetMix;

            buffer[writePos] = inputSample;
            writePos = (writePos + 1) % maxDelaySamples;

            lfoPhase += juce::MathConstants<float>::twoPi * rate / sampleRateOS;
            if (lfoPhase > juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;
        }
    }

    oversampler->processSamplesDown(context.getOutputBlock());
}


void ChorusProcessor::reset()
{
    if (oversampler)
        oversampler->reset();

    lowPassFilter.reset();
    lfo.reset();

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
    // Optional clamp or scale
    depth = std::clamp(newDepth, 0.5f, 10.0f); // For example, 0.5ms to 10ms range
}


void ChorusProcessor::setMix(float newMix)
{
    mix = newMix;
}
