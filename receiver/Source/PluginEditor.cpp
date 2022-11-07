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
    setSize (400, 300);
}

AudioreceiverAudioProcessorEditor::~AudioreceiverAudioProcessorEditor()
{
}

//==============================================================================
void AudioreceiverAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont(15.0f);
    g.drawText("Buffer Size = " + juce::String(audioProcessor.getBufferSize()), 50, 0, 200, 100, juce::Justification::centred, 1);
    g.drawText("Sampling Rate = " + juce::String(audioProcessor.getSampleRate()), 50, 50, 200, 100, juce::Justification::centred, 1);
}

void AudioreceiverAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
