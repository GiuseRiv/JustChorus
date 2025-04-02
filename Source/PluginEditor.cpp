/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
IChorusAudioProcessorEditor::IChorusAudioProcessorEditor (IChorusAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the size of the editor.
    setSize (400, 300);
    
    // Configure the Rate knob.
    rateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    rateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::saddlebrown);
    addAndMakeVisible(rateSlider);
    
    // Configure the Depth knob.
    depthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    depthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::saddlebrown);
    addAndMakeVisible(depthSlider);
    
    // Configure the Mix knob.
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    mixSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::saddlebrown);
    addAndMakeVisible(mixSlider);
    
    // Attach sliders to the corresponding parameters in the APVTS.
    rateAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "rate", rateSlider);
    depthAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "depth", depthSlider);
    mixAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "mix", mixSlider);
}

IChorusAudioProcessorEditor::~IChorusAudioProcessorEditor()
{
}

//==============================================================================
void IChorusAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Create a vintage feel using a gradient background.
    auto bounds = getLocalBounds();
    juce::ColourGradient gradient (juce::Colours::darkolivegreen, 0, 0,
                                   juce::Colours::black, 0, static_cast<float>(bounds.getHeight()), true);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Draw a vintage-style title at the top.
    g.setColour(juce::Colours::antiquewhite);
    g.setFont(juce::FontOptions("Courier New", 24.0f, juce::Font::bold));
    g.drawFittedText("IChorus", bounds.removeFromTop(40), juce::Justification::centred, 1);
}

void IChorusAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Divide the area for the three knobs.
    auto sliderArea = area.removeFromTop(area.getHeight() / 2);
    int sliderWidth = sliderArea.getWidth() / 3;
    
    rateSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(10));
    depthSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(10));
    mixSlider.setBounds(sliderArea.reduced(10));
}
