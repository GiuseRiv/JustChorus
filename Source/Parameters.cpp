/*
  ==============================================================================
  
    Parameters.cpp
    Created: 29 Mar 2025 12:42:28pm
    Author:  Giuseppe Rivezzi
  
  ==============================================================================
*/

#include "Parameters.h"

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    // Create a vector to hold unique pointers to your ranged parameters.
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Define the 'rate' parameter: controls the LFO frequency (in Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "rate",   // Parameter ID
        "Rate",   // Parameter Name
        0.1f,     // Minimum value
        5.0f,     // Maximum value
        1.0f      // Default value
    ));

    // Define the 'depth' parameter: controls the modulation depth
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "depth",
        "Depth",
        0.0f,
        1.0f,
        0.5f
    ));

    // Define the 'mix' parameter: controls the wet/dry mix of the effect
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix",
        "Mix",
        0.0f,
        1.0f,
        0.5f
    ));

    // Return the ParameterLayout constructed from the vector of parameters.
    return { params.begin(), params.end() };
}

