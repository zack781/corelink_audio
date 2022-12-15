/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioreceiverAudioProcessorEditor::AudioreceiverAudioProcessorEditor (AudioreceiverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible(button);
    button.setButtonText("Mute");
    
    button.onClick = [this]() { buttonHandler(); };
    button.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    
    setSize (400, 300);
}

AudioreceiverAudioProcessorEditor::~AudioreceiverAudioProcessorEditor()
{
}

void AudioreceiverAudioProcessorEditor::buttonHandler()
{
    audioProcessor.setMute(!audioProcessor.isMuted());
    if (audioProcessor.isMuted())
    {
        button.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    }
    else {
        button.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    }
}

//==============================================================================
void AudioreceiverAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Audio Receiver", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioreceiverAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    button.setBounds(150, 175, 100, 30);
}
