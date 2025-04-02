/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class IChorusAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    IChorusAudioProcessorEditor (IChorusAudioProcessor&);
    ~IChorusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    IChorusAudioProcessor& audioProcessor;
    
    juce::Slider rateSlider;
    juce::Slider depthSlider;
    juce::Slider mixSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IChorusAudioProcessorEditor)
};
