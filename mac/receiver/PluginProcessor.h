/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#pragma comment (dylib, "libwebsockets")
#pragma comment (dylib, "libssl")
#pragma comment (dylib, "libcrypto")


#define CORELINK_USE_CONCURRENT_QUEUE
#include "corelink_all.hpp"

#include <iostream>
#include <future>
#include <mutex>
#include <pthread.h>

#include <JuceHeader.h>

namespace ns_cl_client = corelink::client;
namespace ns_cl_core = corelink::core;
namespace ns_cl_req_resp = ns_cl_client::request_response;
template<typename t> using in = corelink::in<t>;
template<typename t> using out = corelink::out<t>;

//==============================================================================
/**
*/
class AudioreceiverAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
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
    
    void on_error(corelink::core::network::channel_id_type host_id, in<std::string> err);
    
    void on_channel_init(corelink::core::network::channel_id_type host_id);
    
    void on_channel_uninit(corelink::core::network::channel_id_type host_id);
    
    void create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
                         out<corelink::client::corelink_classic_client> client);
    
//    void process_packet(in<std::vector<float>> data, std::queue<std::vector<float>*>* bufferQueue, int bufferSize);
    
    //void buffer_processing(std::vector<uint8_t>& data, int bufferSize);
    void buffer_processing(std::vector<uint8_t>* data, int bufferSize);
        
    template<class T>
    void swapMove(T& a, T& b);
    
    bool isMuted()const;
    void setMute(bool val);
    
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioreceiverAudioProcessor)
    
    corelink::client::corelink_classic_client client;
    corelink::client::corelink_client_connection_info info;
    corelink::core::network::channel_id_type control_channel_id;
    
    std::string cert_path;
    
    bool done;
    bool loading;
    
    bool receiverInit = false;
    
    int bufferSize_;
    
    std::vector<float> buffer_;
    std::queue<std::vector<float>*> bufferQueue;
    
    
    // Circular Buffer
    std::vector<float> myBuffer;
    int read_pos;
    int write_pos;
    
    std::mutex bufferMutext;
    
    std::vector<std::future<void>> m_Futures;
    
    // -------------
    
    std::queue<juce::AudioBuffer<float>> juceBuffers;
    
    bool mute = false;
    int multiplier = 1;
    
};
