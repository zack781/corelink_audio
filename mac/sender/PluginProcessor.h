/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#pragma comment (dylib, "libssl")
#pragma comment (dylib, "libcrypto")

//#define CORELINK_USE_WEBSOCKET                         // Enable usage of websockets

#define CORELINK_USE_CONCURRENT_COUNTER                // Enable usage of concurrent counter class
#define CORELINK_USE_CONCURRENT_QUEUE                  // Enable usage of concurrent queue class
#define CORELINK_USE_CONCURRENT_MAP                    // Enable usage of concurrent map class
#define CORELINK_ENABLE_STRING_UTIL_FUNCTIONS


#include "corelink_all.hpp"

#include <iostream>
#include <future>

#include <JuceHeader.h>

namespace ns_cl_client = corelink::client;
namespace ns_cl_core = corelink::core;
namespace ns_cl_req_resp = ns_cl_client::request_response;
template<typename t> using in = corelink::in<t>;
template<typename t> using out = corelink::out<t>;

//==============================================================================
/**
*/
class AudiosenderAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AudiosenderAudioProcessor();
    ~AudiosenderAudioProcessor() override;

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
    
    void on_channel_init(corelink::core::network::channel_id_type host_id);
    void on_channel_uninit(corelink::core::network::channel_id_type host_id);
    void on_error(corelink::core::network::channel_id_type host_id, in<std::string> err);
    void create_sender(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);
    
    template<class T>
    void swapMove(T& a, T& b);
    
    void sendData(juce::AudioBuffer<float>* buffer, int bufferSize);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudiosenderAudioProcessor)
    
    corelink::client::corelink_classic_client client;
    corelink::client::corelink_client_connection_info info;
    corelink::core::network::channel_id_type control_channel_id;
    
    std::vector<float> floatBuffer;
    corelink::utils::json meta;
    int bufferSize;
    double sampleRate_;
    
    
    std::string cert_path;
    
    bool done;
    bool loading;
    
    corelink::core::network::channel_id_type hostId;
    
    
};
