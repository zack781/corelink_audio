/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReceiverAudioProcessor::ReceiverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), my_fifo(2048)
#endif
{
    
    
    
    done = false;
    loading = true;
    
        
    cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";
    
    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);
    
    info.set_certificate_path(info_temp.client_certificate_path);
    info.set_hostname("corelink.hpc.nyu.edu");
    info.set_username(info_temp.username);
    info.set_password(info_temp.password);
    info.set_port_number(20010);
    
    
    if(!client.init_protocols())
    {
        throw corelink::commons::corelink_exception("Failed to init protocol!");
    }
    
    control_channel_id = client.add_control_channel(
                                                    info.protocol,
                                                    info.hostname,
                                                    info.port_number,
                                                    info.client_certificate_path,
                                                    std::bind(&ReceiverAudioProcessor::on_error, this, std::placeholders::_1, std::placeholders::_2),
                                                    std::bind(&ReceiverAudioProcessor::on_channel_init, this, std::placeholders::_1),
                                                    std::bind(&ReceiverAudioProcessor::on_channel_uninit, this, std::placeholders::_1));
    
    while(!done);
    done = false;
    
    
    client.request(control_channel_id,
                   corelink::client::corelink_functions::authenticate,
                   std::make_shared<corelink::client::request_response::requests::authenticate_client_request>(info.username, info.password),
                   [&](corelink::core::network::channel_id_type host_id,
                       in<std::string>,
                       in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>>)
                   {
                        create_receiver(host_id, client);
                    }
    );
    
    // Circular Buffering
    
//    for (int i=0; i<1024*12 ; i++)
//    {
//        cBuffer.push_back(0);
//    }

    read_pos = 0;
    write_pos = 0;

    // ------------------
    
//    write1 = true;
//    write2 = false;
//
//    fill1 = false;
//    fill2 = false;
    
    //nBuffer.clear();
    for (int i = 0; i < 1120*4; i++)
    {
        //nBuffer.push_back(0);
        tbbVector->push_back(0);
    }
    
    
    while (tsQueue->size() != 0)
    {
        tsQueue->pop();
    }
    if (tsQueue == nullptr)
    {
        delete tsQueue;
    }
    
    while (dataQueue->size() != 0)
    {
        dataQueue->pop();
    }
    if (dataQueue == nullptr)
    {
        delete dataQueue;
    }
    
    //tbbVector->clear();
    //delete tbbVector;
    
    
//    my_fifo.pop();
    
}

ReceiverAudioProcessor::~ReceiverAudioProcessor()
{
    float sum = 0;
    for (float val : elapsed)
    {
        sum+=val;
    }

    DBG(sum << " / " << counter << " = " << sum/counter);
    
    if (tsQueue != nullptr)
    {
        delete tsQueue;
    }
    
    tbbVector->clear();
    delete tbbVector;
        
}

void ReceiverAudioProcessor::on_error(corelink::core::network::channel_id_type host_id, in<std::string> err) {
    std::cerr << "Error in host id: " << host_id << "\n" << err << "\n";

    done = true;
}

void ReceiverAudioProcessor::on_channel_init(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " connected\n";
    DBG("Host id: " << std::to_string(host_id) << " connected");
    done = true;
}

void ReceiverAudioProcessor::on_channel_uninit(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " disconnected\n";
    DBG("Host id: " << std::to_string(host_id) << " disconnected");
    done = true;
}

void ReceiverAudioProcessor::queuing(tbb::detail::d1::concurrent_vector<float>*& tbbVector, ThreadsafeQueue<float>*& tsQueue, std::atomic<int>& write_pos, int bufferSize_, std::atomic<bool>& loading, std::atomic<bool>& receiverInit)
{
    while (true)
    {
//        DBG("queuing");
        
        if (!loading && receiverInit)
        {
//            DBG("nBuffer.size() = " << nBuffer.size());
//            DBG("myQueue.size() = " << myQueue.size());
//            DBG("myQueue.size() = " << myQueue.size());
//            DBG("cBuffer.size() = " << cBuffer.size());
            if (tsQueue->size() > bufferSize_ * 4)
            {
                //                if (write_flag == 0 && write)
                //                {
                //                    queueMutex.lock();
                //                    for (int i = 0; i < bufferSize_ * 2; i++)
                //                    {
                //
                //                        nBuffer[i] = myQueue.front();
                //                        myQueue.pop();
                //
                //                    }
                //                    queueMutex.unlock();
                //
                //                    writeMutex.lock();
                //                    write = false;
                //                    writeMutex.unlock();
                //                }
                //                else if (write_flag == 1 && write)
                //                {
                //                    queueMutex.lock();
                //                    for (int i = bufferSize_ * 2; i < bufferSize_ * 4; i++)
                //                    {
                //
                //                        nBuffer[i] = myQueue.front();
                //                        myQueue.pop();
                //
                //                    }
                //                    queueMutex.unlock();
                //
                //                    writeMutex.lock();
                //                    write = false;
                //                    writeMutex.unlock();
                //                }
                //            }
//                std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                
//                bufferMutex.lock();
//                cBuffer[write_pos] = cQueue.front();
//                cQueue.pop();
                auto res = *std::move(tsQueue->pop());
                //DBG("pop = " << res);
                //(*tbbVector)[write_pos] = res;
                writeMutex.lock();
                (*tbbVector)[write_pos] = res;
                writeMutex.unlock();
                write_pos += 1;
                write_pos = write_pos % tbbVector->size();
                
                
//                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                
//                float write_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                
//                bufferMutex.unlock();
                
//                if (read_time != 0 && write_time < read_time)
//                {
//                    std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(end - start));
//                    //std::this_thread::sleep_for(std::chrono::microseconds(1));
//                }
//
//                if (write_pos > bufferSize_ * 2)
//                {
//                    init = true;
//                }
            }
        }
    }
}

void ReceiverAudioProcessor::asyncCallerProcessing(AsyncCaller<farbot::fifo_options::concurrency::single>& update_queue, ThreadsafeQueue<std::vector<uint8_t>>*& dataQueue, ThreadsafeQueue<float>*& tsQueue, int bufferSize_, std::mutex& bufferMutex, std::atomic<bool>& loading, std::atomic<bool>& receiverInit)
{
    while (true)
    {
        if (!loading && receiverInit)
        {
            update_queue.process(std::ref(*dataQueue), std::ref(*tsQueue), bufferSize_, std::ref(bufferMutex));
        }
    }
}

void ReceiverAudioProcessor::create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
                     out<corelink::client::corelink_classic_client> client)
{
        // we are creating a TCP sender
        auto request =
            std::make_shared<corelink::client::request_response::requests::modify_receiver_stream_request>(
                ns_cl_core::network::constants::protocols::udp);

        // only websockets
        request->
            client_certificate_path = cert_path;
        request->
            alert = true;
        request->
            echo = true;
        request->
            workspace = "ZackAudio";
        request->
            stream_type = "['audiotesting']\n";

        request->
            on_init = [this](ns_cl_core::network::channel_id_type /*host_id*/)
        {
            std::cout << "Receiver init\n";
            
//            while (this->loading);
//
            std::thread queue_thread(&ReceiverAudioProcessor::queuing, this, std::ref(this->tbbVector), std::ref(this->tsQueue), std::ref(this->write_pos), 1024, std::ref(this->loading), std::ref(this->receiverInit));
            queue_thread.detach();

            std::thread AsyncCaller_thread(&ReceiverAudioProcessor::asyncCallerProcessing, this, std::ref(this->update_queue), std::ref(this->dataQueue), std::ref(this->tsQueue), 1024, std::ref(this->bufferMutex), std::ref(this->loading), std::ref(this->receiverInit));
            AsyncCaller_thread.detach();
        };
        request->
            on_uninit = [](ns_cl_core::network::channel_id_type /*host_id*/)
        {
            std::cout << "Receiver uninit\n";
            DBG("Receiver uninit");
        };
        request->
            on_error = [](corelink::core::network::channel_id_type /*host_id*/, in<std::string> err)
        {
            std::cerr << "Error while receiving data on the data channel: " << err << "\n";
            /*DBG("Error while receiving data on the data channel: " << err);*/
        };
        request->
            on_receive = [this](ns_cl_core::network::channel_id_type host_id,
                in<corelink::client::constants::corelink_stream_id_type> stream_id,
                in<corelink::utils::json> headers,
                std::vector<uint8_t> data)
        {

            if (!data.empty())
            {
                //bufferSize_ = headers.get_int("bufferSize");
                if (!loading && receiverInit)
                {
//                    std::vector<uint8_t> tmp;
//                    swapMove(tmp, data);
                    
                    std::thread tmp_thread(&ReceiverAudioProcessor::buffer_processing, this, std::move(data), 1024);
                    tmp_thread.detach();
//
//                    update_queue.callAsync([] () { DBG("Hello World"); });
                    
//                    for (int i = 0; i < bufferSize_ * 8; i += 4) {
//
//                        //DBG("sample = " << sample);
//                        unsigned b1 = (unsigned)((data)[i]);
//                        unsigned b2 = (unsigned)((data)[i + 1]);
//                        unsigned b3 = (unsigned)((data)[i + 2]);
//                        unsigned b4 = (unsigned)((data)[i + 3]);
//
//                        b1 <<= 24;
//                        b2 <<= 24;
//                        b2 >>= 8;
//                        b3 <<= 24;
//                        b3 >>= 16;
//
//                        unsigned res = b1 + b2 + b3 + b4;
//                        float* p_ = (float*)(&res);
//                        // Circular Buffer Implementation
//                        cBuffer[write_pos] = *p_;
//                        //DBG("cBuffer[write_pos] = " << cBuffer[write_pos]);
//                        write_pos = (write_pos + 1) % cBuffer.size();
//                    }
                }
            }
            

        };
        client.request(
                control_channel_id,
                ns_cl_client::corelink_functions::create_receiver,
                request,
                [&](corelink::core::network::channel_id_type /*host_id*/,
                    in<std::string> /*msg*/,
                    in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response)
                {
                    //std::cout << "Created Receiver\n";
                    receiverInit = true;
                    //loading = false;
                    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                }
        );
}


template<class T>
void ReceiverAudioProcessor::swapMove(T& a, T& b)
{
    T tmp{ std::move(a) };
    a = std::move(b);
    b = std::move(tmp);
}

void ReceiverAudioProcessor::buffer_processing(std::vector<uint8_t> data, int bufferSize)
{
    
    //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//    for (int i = 0; i < bufferSize * 8; i += 4) {
//        unsigned b1 = (unsigned)((*data)[i]);
//        unsigned b2 = (unsigned)((*data)[i + 1]);
//        unsigned b3 = (unsigned)((*data)[i + 2]);
//        unsigned b4 = (unsigned)((*data)[i + 3]);
//        b1 <<= 24;
//        b2 <<= 24;
//        b2 >>= 8;
//        b3 <<= 24;
//        b3 >>= 16;
//        unsigned res = b1 + b2 + b3 + b4;
//        float* p_ = (float*)(&res);
//        // Circular Buffer Implementation
//        tsQueue->push(*p_);
//        //DBG("cBuffer[write_pos] = " << cBuffer[write_pos]);
//        write_pos = (write_pos + 1) % tsQueue->size();
//    }
//
    this->bufferMutex.lock();
    this->dataQueue->push(std::move(data));
    this->bufferMutex.unlock();
//
    this->update_queue.callAsync([this] (ThreadsafeQueue<std::vector<uint8_t>>& dataQueue, ThreadsafeQueue<float>& tsQueue, int bufferSize_, std::mutex& bufferMutex) {
        
        bufferMutex.lock();
        std::vector<uint8_t> tmp = *std::move(dataQueue.pop());
        bufferMutex.unlock();
        
        for (int i = 0; i < bufferSize_ * 8; i += 4) {
            unsigned b1 = (unsigned)(tmp[i]);
            unsigned b2 = (unsigned)(tmp[i + 1]);
            unsigned b3 = (unsigned)(tmp[i + 2]);
            unsigned b4 = (unsigned)(tmp[i + 3]);

            b1 <<= 24;
            b2 <<= 24;
            b2 >>= 8;
            b3 <<= 24;
            b3 >>= 16;

            unsigned res = b1 + b2 + b3 + b4;
            float* p_ = (float*)(&res);

            tsQueue.push(std::move(*p_));
            //this->write_pos = (this->write_pos + 1) % this->tsQueue->size();
        }
    });
}

//==============================================================================
const juce::String ReceiverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReceiverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReceiverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReceiverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReceiverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReceiverAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReceiverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReceiverAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReceiverAudioProcessor::getProgramName (int index)
{
    return {};
}

void ReceiverAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ReceiverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    bufferSize_ = samplesPerBlock;

    // Circular Buffering
    //cBuffer.clear();
    tbbVector->clear();

    while (tsQueue->size() != 0)
    {
        tsQueue->pop();
    }


    while (dataQueue->size() != 0)
    {
        dataQueue->pop();
    }


    for (int i=0; i<samplesPerBlock*4; i++)
    {
        //cBuffer.push_back(0);
        tbbVector->push_back(0);

    }

    read_pos = 0;
    write_pos = 0;
    loading = false;

    DBG("loading is false");
    // ------------------
    
    
    // Double Buffering
//    buffer1.clear();
//    buffer2.clear();
//
//    for (int i=0; i < samplesPerBlock; i++)
//    {
//        buffer1.push_back(0);
//        buffer2.push_back(0);
//    }
    // ----------------
    
    
    // Queuing + 2-N Buffer
//    nBuffer.clear();
//    for (int i = 0; i < samplesPerBlock*4; i++)
//    {
//        nBuffer.push_back(0);
//    }
//
    // --------------------
    
}

void ReceiverAudioProcessor::releaseResources()
{
    
//    float sum = 0;
//    for (float val : elapsed)
//    {
//        sum+=val;
//    }
//
//    DBG(sum << " / " << counter << " = " << sum/counter);
    while (tsQueue->size() != 0)
    {
        tsQueue->pop();
    }
    if (tsQueue->size() == 0)
    {
        delete tsQueue;
    }
    
    while (dataQueue->size() != 0)
    {
        dataQueue->pop();
    }
    if (dataQueue->size() == 0)
    {
        delete dataQueue;
    }
    
    tbbVector->clear();
    delete tbbVector;
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReceiverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ReceiverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
//    DBG("processBlock");
    
    if (!loading && receiverInit)
    {
//        DBG("tsQueue->size = " << tsQueue->size());
        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
//        DBG("#input channels = " << totalNumInputChannels);
//        DBG("#output channels = " << totalNumOutputChannels);
//        DBG("read_pos = " << read_pos);
//        DBG("write_pos = " << write_pos);
        
        for (auto channel = 0; channel < getTotalNumOutputChannels(); channel++)
        {
            auto out = buffer.getWritePointer(channel);
            
//            bufferMutex.lock();
            for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
            {
//                DBG("read_pos = " << read_pos);
//                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                // Circular Buffering
                //out[sample] = (*tbbVector)[read_pos];
                writeMutex.lock();
                auto res = (*tbbVector)[read_pos];
                writeMutex.unlock();
                
                out[sample] = res;
                read_pos += 1;
                read_pos = read_pos % tbbVector->size();
                // --------------------
                
//                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                
//                read_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
//
//                // Double Buffering
//                if (!write1 && fill1)
//                {
//                    out[sample] = buffer1[sample];
//                }
//                else {
//                    if (!write2 && fill2)
//                    {
//                        out[sample] = buffer2[sample];
//                    }
//                }
//
//
//                // ----------------
                
                // Queuing and 2-N Buffer
                
//                DBG("queue size = " << myQueue.size());
//                DBG("write_flag = " << write_flag);
                
//                if (write_flag == 0)
//                {
////                    DBG("sample = " << nBuffer[sample + bufferSize_ + bufferSize_ * channel]);
//                    out[sample] = nBuffer[sample + bufferSize_ + bufferSize_ * channel];
//                }
//                else {
////                    DBG("sample = " << nBuffer[sample + bufferSize_ * channel]);
//                    out[sample] = nBuffer[sample + bufferSize_ * channel];
//                }
                
                // ----------------------
                
            }
//            bufferMutex.unlock();
        }
//        if (!write1)
//        {
//            fill1 = false;
//        }
//
//        if (!write2)
//        {
//            fill2 = false;
//        }
//            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//
//            elapsed.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
//            counter++;
//        if (write_flag == 0)
//        {
//            writeMutex.lock();
//            write_flag = 1;
//            write = true;
//            writeMutex.unlock();
//        }
//        else {
//            writeMutex.lock();
//            write_flag = 0;
//            write = true;
//            writeMutex.unlock();
//        }
        
    }
    

}

//==============================================================================
bool ReceiverAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReceiverAudioProcessor::createEditor()
{
    juce::AudioProcessorEditor* editor;
//    return new ReceiverAudioProcessorEditor (*this);
    return editor;
}

//==============================================================================
void ReceiverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ReceiverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReceiverAudioProcessor();
}
