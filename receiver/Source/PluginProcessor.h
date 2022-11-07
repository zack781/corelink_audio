/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#pragma comment (lib, "libssl")
#pragma comment (lib, "libcrypto")
#pragma comment (lib, "websockets")
#pragma comment (lib, "CorelinkClient")

#include <JuceHeader.h>
#include <iostream>
#include "Client.h"

//==============================================================================
/**
*/
class AudioreceiverAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioreceiverAudioProcessor();
    ~AudioreceiverAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    int getBufferSize()const;
    double getSampleRate()const;

private:

    juce::AudioDeviceManager deviceManager;

    Client obj;
    juce::Time timeobj;

    int bufferSize;
    double sampleRate_;


    corelink::client::corelink_classic_client client;
    corelink::client::corelink_client_connection_info info;
    corelink::core::network::channel_id_type control_channel_id;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioreceiverAudioProcessor)
};
