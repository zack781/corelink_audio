/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class AudioreceiverAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioreceiverAudioProcessorEditor (AudioreceiverAudioProcessor&);
    ~AudioreceiverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void buttonHandler();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioreceiverAudioProcessor& audioProcessor;
    
    juce::TextButton button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioreceiverAudioProcessorEditor)
};
