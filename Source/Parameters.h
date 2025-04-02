/*
  ==============================================================================

    Parameters.h
    Created: 29 Mar 2025 12:42:21pm
    Author:  Giuseppe Rivezzi

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// Returns a ParameterLayout containing all the plugin parameters.
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
