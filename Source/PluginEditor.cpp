/*
  ==============================================================================
  
    PluginEditor.cpp
    Created: [date]
    Author:  Giuseppe Rivezzi
  
  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

// Helper function to configure a slider and attach a label.
void IChorusAudioProcessorEditor::configureSlider(juce::Slider& slider, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::saddlebrown);
    addAndMakeVisible(slider);
    
    // Create and attach a label to the slider.
    auto* label = new juce::Label();
    label->setText(labelText, juce::dontSendNotification);
    label->attachToComponent(&slider, false); // false = below the slider
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible(label);
    
    // Save label to manage its lifetime.
    sliderLabels.add(label);
}

//==============================================================================
IChorusAudioProcessorEditor::IChorusAudioProcessorEditor (IChorusAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the size of the editor (increased for a more spacious layout).
    setSize (500, 400);
    
    // Title Label setup.
    titleLabel.setText("IChorus", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions("Courier New", 28.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible(titleLabel);
    
    // Configure and add sliders with labels.
    configureSlider(rateSlider, "Rate");
    configureSlider(depthSlider, "Depth");
    configureSlider(mixSlider, "Mix");
    
    // Attach sliders to the corresponding parameters in the APVTS.
    rateAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                          audioProcessor.getAPVTS(), "rate", rateSlider);
    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                          audioProcessor.getAPVTS(), "depth", depthSlider);
    mixAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                          audioProcessor.getAPVTS(), "mix", mixSlider);
}

IChorusAudioProcessorEditor::~IChorusAudioProcessorEditor()
{
}

//==============================================================================
void IChorusAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Draw a professional gradient background.
    auto bounds = getLocalBounds();
    juce::ColourGradient gradient (juce::Colours::white, 0, 0,
                                   juce::Colours::grey, 0, static_cast<float>(bounds.getHeight()), true);
    g.setGradientFill(gradient);
    g.fillAll();
}

void IChorusAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Position the title at the top.
    titleLabel.setBounds(area.removeFromTop(50));
    
    // Divide remaining area equally for the sliders.
    auto slidersArea = area.removeFromTop(area.getHeight() / 2);
    int sliderWidth = slidersArea.getWidth() / 3;
    
    rateSlider.setBounds(slidersArea.removeFromLeft(sliderWidth).reduced(10));
    depthSlider.setBounds(slidersArea.removeFromLeft(sliderWidth).reduced(10));
    mixSlider.setBounds(slidersArea.reduced(10));
}; 