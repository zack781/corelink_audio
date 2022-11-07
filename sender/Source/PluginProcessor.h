/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#pragma comment (lib, "libssl")
#pragma comment (lib, "libcrypto")
#pragma comment (lib, "websockets")
#pragma comment (lib, "CorelinkClientApp")

#define CORELINK_USE_CONCURRENT_QUEUE

#include <JuceHeader.h>

#include "corelink_all.hpp"
#include <iostream>
#include "Client.h"
#include <future>
//==============================================================================
/**
*/
class CorelinkvstAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CorelinkvstAudioProcessor();
    ~CorelinkvstAudioProcessor() override;

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

    Client getClientObj()const {
        return clientobj;
    }
    
    bool getConnectState()const {
        return connect;
    }

    bool setConnectState(bool state) {
        connect = state;
        return connect;
    }

    //void sendData_(std::queue<juce::AudioBuffer<float>*>& buffer);
    void sendData_(juce::AudioBuffer<float>* buffer);
    void sendData(std::queue<juce::AudioBuffer<float>*>& bufferQueue);

private:

    Client clientobj;

    corelink::client::corelink_classic_client client;
    corelink::client::corelink_client_connection_info info;
    corelink::core::network::channel_id_type control_channel_id;
    
    std::vector<float> floatBuffer;

    std::queue<juce::AudioBuffer<float>*> bufferQueue;

    std::mutex s_QueueMutex;

    std::vector<std::future<void>> m_Futures;

    float* lst_0;
    float* lst_1;

    corelink::utils::json meta;
    int bufferSize;
    double sampleRate_;
    long long timeStamp;


    bool connect = false;

 
 
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CorelinkvstAudioProcessor)
};
