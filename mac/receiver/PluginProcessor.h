/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

//#define CORELINK_USE_WEBSOCKET
#define CORELINK_USE_CONCURRENT_QUEUE
#define CORELINK_ENABLE_STRING_UTIL_FUNCTIONS

#define CONNECT_TO_LOCAL

#ifdef CONNECT_TO_LOCAL
#define CORELINK_WEBSOCKET_CONNECT_LOCAL_SERVER
#endif

#include <JuceHeader.h>
#include "corelink_all.hpp"
#include <iostream>
#include <future>
//#include <atomic>
#include <queue>
#include "tbb.h"
//#include "queue.cpp"
#include "ThreadsafeQueue.h"
#include "fifo.hpp"

//#include "AsyncCaller.hpp"
#include <functional>
#include "AsyncCaller_juce.h"

namespace ns_cl_client = corelink::client;
namespace ns_cl_core = corelink::core;
namespace ns_cl_req_resp = ns_cl_client::request_response;
template<typename t> using in = corelink::in<t>;
template<typename t> using out = corelink::out<t>;


//==============================================================================
/**
*/
class ReceiverAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ReceiverAudioProcessor();
    ~ReceiverAudioProcessor() override;

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
    
    void buffer_processing(std::vector<uint8_t> data, int bufferSize);
        
    template<class T>
    void swapMove(T& a, T& b);
    
    void queuing(tbb::detail::d1::concurrent_vector<float>*& tbbVector, ThreadsafeQueue<float>*& tsQueue, std::atomic<int>& write_pos, int bufferSize_, std::atomic<bool>& loading, std::atomic<bool>& receiverInit);
    
    void asyncCallerProcessing(AsyncCaller<farbot::fifo_options::concurrency::single>& update_queue, ThreadsafeQueue<std::vector<uint8_t>>*& dataQueue, ThreadsafeQueue<float>*& tsQueue, int bufferSize_, std::mutex& bufferMutex, std::atomic<bool>& loading, std::atomic<bool>& receiverInit);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReceiverAudioProcessor)
    
    corelink::client::corelink_classic_client client;
    std::string cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";
    
    corelink::client::corelink_client_connection_info info = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);
    
    corelink::core::network::channel_id_type control_channel_id;
        
    std::atomic<bool> done;
    std::atomic<bool> loading;
    std::atomic<bool> receiverInit = false;
    std::atomic<int> bufferSize_;
    
    std::queue<std::vector<float>> slowQueue;
    std::vector<float> tmpVector;
   
//    std::atomic<bool> to_write = false;
    
    //float sampleRate;
    
    ThreadsafeQueue<float>* tsQueue = new ThreadsafeQueue<float>();
    ThreadsafeQueue<std::vector<uint8_t>>* dataQueue = new ThreadsafeQueue<std::vector<uint8_t>>();
    
    tbb::detail::d1::concurrent_vector<float>* tbbVector = new tbb::detail::d1::concurrent_vector<float>();
    
    farbot::fifo<float,
                 farbot::fifo_options::concurrency::single,
                 farbot::fifo_options::concurrency::single,
                 farbot::fifo_options::full_empty_failure_mode::overwrite_or_return_default,
                 farbot::fifo_options::full_empty_failure_mode::overwrite_or_return_default> my_fifo;
    
    
    AsyncCaller<farbot::fifo_options::concurrency::single> update_queue;
    //tbb::detail::d2::concurrent_queue<float> cQueue;
    
    // Circular Buffer
    std::atomic<int> read_pos;
    std::atomic<int> write_pos;
    
//    std::vector<float> cBuffer;
    
    //ThreadsafeQueue* dataQueue = new ThreadsafeQueue();
    //std::queue<std::vector<uint8_t>> dataQueue;
    
    std::mutex bufferMutex;
    std::mutex writeMutex;
    
    float read_time = 0;
    
    bool init = false;
    
    std::vector<uint8_t> v8;
    
    // ---------------
    
    // Double Buffering
    
    std::vector<float> buffer1;
    std::vector<float> buffer2;
    
    bool write1;
    bool write2;
    
    bool fill1;
    bool fill2;
    
    // ----------------
    
    // Metrics
    std::vector<float> elapsed;
    int counter = 0;
    
    // -------
    
    // Queuing + 2-N Buffer
    
    std::queue<float> myQueue;
    std::mutex queueMutex;
    
    std::vector<float> nBuffer;
    
    int write_flag = 0;
    bool write = true;
    //std::mutex writeMutex;
    // --------------------
    
    
};
