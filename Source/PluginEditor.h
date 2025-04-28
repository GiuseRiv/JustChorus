/*
  ==============================================================================
  
    PluginEditor.h
    Created: [date]
    Author:  Giuseppe Rivezzi
  
  ==============================================================================
*/

#pragma once

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class IChorusAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    IChorusAudioProcessorEditor (IChorusAudioProcessor&);
    ~IChorusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void configureSlider(juce::Slider& slider, const juce::String& labelText);

    IChorusAudioProcessor& audioProcessor;

    // UI Elements
    juce::Label titleLabel;

    juce::Slider rateSlider;
    juce::Slider depthSlider;
    juce::Slider mixSlider;

    // Attachments for APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // Keep track of labels for memory management
    juce::OwnedArray<juce::Label> sliderLabels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IChorusAudioProcessorEditor)
};
