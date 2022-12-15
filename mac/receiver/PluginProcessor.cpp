/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/



#include "PluginProcessor.h"
#include "PluginEditor.h"
#define print_delim std::cout << "-------------------------------------------\n"


//==============================================================================
AudioreceiverAudioProcessor::AudioreceiverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    done = false;
    loading = true;

    cert_path = ".\\config\\ca-crt.pem";
    
    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);
    
    info.set_certificate_path(info_temp.client_certificate_path);
    info.set_hostname("corelink.hpc.nyu.edu");
    info.set_username(info_temp.username);
    info.set_password(info_temp.password);
    info.set_port_number(20010);
    
    if(!client.init_protocols())
    {
        throw corelink::commons::corelink_exception("Failed to ijnit protocol info!");
    }
    
    control_channel_id = client.add_control_channel(
                                                    info.protocol,
                                                    info.hostname,
                                                    info.port_number,
                                                    info.client_certificate_path,
                                                    std::bind(&AudioreceiverAudioProcessor::on_error, this, std::placeholders::_1, std::placeholders::_2),
                                                    std::bind(&AudioreceiverAudioProcessor::on_channel_init, this, std::placeholders::_1),
                                                    std::bind(&AudioreceiverAudioProcessor::on_channel_uninit, this, std::placeholders::_1));
    
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
    
    for (int i=0; i<1024*12 ; i++)
    {
        myBuffer.push_back(0);
    }
    
    read_pos = 0;
    write_pos = 0;
    
}

AudioreceiverAudioProcessor::~AudioreceiverAudioProcessor()
{
}

void AudioreceiverAudioProcessor::on_error(corelink::core::network::channel_id_type host_id, in<std::string> err) {
    std::cerr << "Error in host id: " << host_id << "\n" << err << "\n";

    done = true;
}

void AudioreceiverAudioProcessor::on_channel_init(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " connected\n";
    DBG("Host id: " << std::to_string(host_id) << " connected");
    print_delim;
    done = true;
}

void AudioreceiverAudioProcessor::on_channel_uninit(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " disconnected\n";
    DBG("Host id: " << std::to_string(host_id) << " disconnected");
    print_delim;
    done = true;
}

//void AudioreceiverAudioProcessor::process_packet(in<std::vector<float>> data, std::queue<std::vector<float>*>* bufferQueue, int bufferSize)
//{
//    std::vector<float> buffer_;
//    
//    for (int i = 0; bufferSize * 8; i += 4) {
//         
//            //DBG("sample = " << sample);
//            unsigned b1 = (unsigned)data[i];
//            unsigned b2 = (unsigned)data[i + 1];
//            unsigned b3 = (unsigned)data[i + 2];
//            unsigned b4 = (unsigned)data[i + 3];
//
//            b1 <<= 24;
//            b2 <<= 24;
//            b2 >>= 8;
//            b3 <<= 24;
//            b3 >>= 16;
//
//            unsigned res = b1 + b2 + b3 + b4;
//            float* p_ = (float*)(&res);
//            buffer_.push_back(*p_);
//    }
//    bufferQueue->push(new std::vector<float>(buffer_));
//}

void AudioreceiverAudioProcessor::create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
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
            on_init = [](ns_cl_core::network::channel_id_type /*host_id*/)
        {
            std::cout << "Receiver init\n";
            DBG("Receiver init");
            print_delim;
        };
        request->
            on_uninit = [](ns_cl_core::network::channel_id_type /*host_id*/)
        {
            std::cout << "Receiver uninit\n";
            DBG("Receiver uninit");
            print_delim;
        };
        request->
            on_error = [](corelink::core::network::channel_id_type /*host_id*/, in<std::string> err)
        {
            std::cerr << "Error while receiving data on the data channel: " << err << "\n";
            /*DBG("Error while receiving data on the data channel: " << err);*/
            print_delim;
        };
        request->
            on_receive = [this](ns_cl_core::network::channel_id_type host_id,
                in<corelink::client::constants::corelink_stream_id_type> stream_id,
                in<corelink::utils::json> headers,
                std::vector<uint8_t> data)
        {
           /* std::cout << "Received some data from data channel: " << host_id << " corresponding to Corelink Stream ID: "
                << stream_id << '\n';*/
            //DBG("Received some data from data channel: " << std::to_string(host_id) << " corresponding to Corelink Stream ID: " << std::to_string(stream_id));
            
            //DBG("vector size = " << data.size());
            int counter = 0;
            if (!data.

                empty()

                )
            {
                        //buffer_.clear();
//                        for (int i = 0; i < headers.get_int("bufferSize") * 8; i += 4) {
//
//                            //DBG("sample = " << sample);
//                            unsigned b1 = (unsigned)data[i];
//                            unsigned b2 = (unsigned)data[i + 1];
//                            unsigned b3 = (unsigned)data[i + 2];
//                            unsigned b4 = (unsigned)data[i + 3];
//
//                            b1 <<= 24;
//                            b2 <<= 24;
//                            b2 >>= 8;
//                            b3 <<= 24;
//                            b3 >>= 16;
//
//                            unsigned res = b1 + b2 + b3 + b4;
//                            float* p_ = (float*)(&res);
////                          DBG("counter = " << counter);
////                          counter++;
////                          DBG("val " << i << " = " << *p_);
//                            //buffer_.push_back(*p_);
//
//                            if (write_pos >= myBuffer.size())
//                            {
//                                write_pos = 0;
//                            }
//                            myBuffer[write_pos] = *p_;
//                            ++write_pos;
//                        }
                        //bufferQueue.push(new std::vector<float>(buffer_));
                        //bufferQueue.push(new std::vector<float>(buffer_));
                        //bufferQueue.push(new std::queue<float>(buffer_));
                        //bufferVector.push_back(new std::vector<float>(buffer_));
                    
                    std::vector<uint8_t> tmp;
                    swapMove(tmp, data);

//                    m_Futures.push_back(std::async(std::launch::async, &AudioreceiverAudioProcessor::buffer_processing, this, &tmp, headers.get_int("bufferSize")));
//
                std::async(std::launch::async, &AudioreceiverAudioProcessor::buffer_processing, this, &tmp, headers.get_int("bufferSize"));
                
            }
            
            if (!data.empty())
            {
                //std::cout << "Headers: " << headers.to_string(true) << '\n';
                //DBG("Headers: " << headers.to_string(true));

                //bufferSize_ = headers.get_int("bufferSize");
                ////DBG("headers bufferSize = " << headers.get_int("bufferSize"));
                //numChannels_ = headers.get_int("numChannels");

                /*timeAtSent.push_back(headers.get_int64("timeAtSent"));*/
                bufferSize_ = headers.get_int("bufferSize");
                //sampleRate = headers.get_double("sampleRate");
                //timeStamp.push(headers.get_int64("timeStamp"));
            }
            else
            {
                //std::cout << "Header is empty!!!!!!!\n";
            }
            //        done = true;
        };
        client.
            request(
                control_channel_id,
                ns_cl_client::corelink_functions::create_receiver,
                request,
                [&](corelink::core::network::channel_id_type /*host_id*/,
                    in<std::string> /*msg*/,
                    in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response)
                {
                    std::cout << "Created Receiver\n";
                    receiverInit = true;
                    loading = false;
                    print_delim;
                }
        );
}

template<class T>
void AudioreceiverAudioProcessor::swapMove(T& a, T& b)
{
    T tmp{ std::move(a) };
    a = std::move(b);
    b = std::move(tmp);
}

void AudioreceiverAudioProcessor::buffer_processing(std::vector<uint8_t>* data, int bufferSize)
{
    
//    juce::AudioBuffer<float> newBuffer;
//
//    newBuffer.setSize(2, bufferSize);
//
//    int counter = 0;
//    int channel = 0;
//
//    auto* outBuffer = newBuffer.getWritePointer(channel);
    
    for (int i = 0; i < bufferSize * 8; i += 4) {

        //DBG("sample = " << sample);
        unsigned b1 = (unsigned)((*data)[i]);
        unsigned b2 = (unsigned)((*data)[i + 1]);
        unsigned b3 = (unsigned)((*data)[i + 2]);
        unsigned b4 = (unsigned)((*data)[i + 3]);

        b1 <<= 24;
        b2 <<= 24;
        b2 >>= 8;
        b3 <<= 24;
        b3 >>= 16;
        
        unsigned res = b1 + b2 + b3 + b4;
        float* p_ = (float*)(&res);
//                          DBG("counter = " << counter);
//                          counter++;
//                          DBG("val " << i << " = " << *p_);
        //buffer_.push_back(*p_);
        
//        if (write_pos >= myBuffer.size())
//        {
//            write_pos = 0;
//        }
//        myBuffer[write_pos] = *p_;
//        ++write_pos;
        
//        outBuffer[counter] = *p_;
//        counter++;
//        if (counter > bufferSize-1)
//        {
//            channel++;
//        }
        
        //while ((write_pos + 1) % myBuffer.size() == read_pos);
        //std::lock_guard<std::mutex> lock(bufferMutext);
        
        //while ((write_pos) % myBuffer.size() == read_pos);
        bufferMutext.lock();
        myBuffer[write_pos] = *p_;
        write_pos = (write_pos + 1) % myBuffer.size();
        bufferMutext.unlock();
        
        
    }
    
//    bufferMutext.lock();
//    juceBuffers.push(newBuffer);
//    bufferMutext.unlock();
}

//==============================================================================
const juce::String AudioreceiverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioreceiverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioreceiverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioreceiverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioreceiverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioreceiverAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioreceiverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioreceiverAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioreceiverAudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioreceiverAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioreceiverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    myBuffer.clear();
    
    for (int i=0; i<samplesPerBlock*12 ; i++)
    {
        myBuffer.push_back(0);
    }
    
    read_pos = 0;
    write_pos = 0;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

void AudioreceiverAudioProcessor::releaseResources()
{
    client.destroy();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioreceiverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AudioreceiverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
        if (mute)
        {
            multiplier = 0;
        } else {
            multiplier = 1;
        }
    
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        //DBG("bufferQueue size = " << bufferQueue.size());
//        if (bufferQueue.size() > 1) {
//
//            for (auto channel = 0; channel < 2; ++channel) {
//                auto* outBuffer = buffer.getWritePointer(channel);
//                for (int sample = 0; sample < bufferSize_; sample++) {
//
//                    outBuffer[sample] = (*(bufferQueue.front()))[sample + (bufferSize_ * channel)] ;
//                }
//            }
//
//
//            bufferQueue.pop();
//        }
//        else {
//            for (auto channel = 0; channel < totalNumOutputChannels; ++channel) {
//                auto* outBuffer = buffer.getWritePointer(channel);
//
//                for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
//                    outBuffer[sample] = 0;
//                }
//            }
//        }
    
//    DBG("myBuffer size = " << myBuffer.size());
//    DBG("read_pos = " << read_pos);
//    DBG("write_pos = " << write_pos);
        
//    auto ptr = &buffer;
//    std::ostringstream get_the_address;
//    get_the_address << ptr;
//    std::string address = get_the_address.str();
//
//
//    DBG(address);
    //while (write_pos == read_pos)

            for (auto channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                auto* outBuffer = buffer.getWritePointer(channel);
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    //            if (read_pos >= myBuffer.size())
                    //            {
                    //                read_pos = 0;
                    //            }
                    //            outBuffer[sample] = myBuffer[read_pos];
                    //            ++read_pos;

                    //bufferMutext.lock();
                    //while(write_pos == read_pos);
                    //while(write_pos == read_pos);
                    outBuffer[sample] = myBuffer[read_pos];
                    read_pos = (read_pos + 1) % myBuffer.size();
                    //bufferMutext.unlock();
                }
            }
    
    //DBG("juceBuffers = " << juceBuffers.size());
    
//    if (juceBuffers.size() > 100)
//    {
//
//        swapMove(buffer, juceBuffers.front());
//
//        for (auto channel = 0; channel < totalNumOutputChannels; ++channel)
//        {
//            auto* outBuffer = juceBuffers.front().getWritePointer(channel);
//            auto* inBuffer = buffer.getReadPointer(channel);
//            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
//            {
//                outBuffer[sample] = inBuffer[sample];
//            }
//        }
//
//        juceBuffers.pop();
//    }
//

    

}

//==============================================================================
bool AudioreceiverAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioreceiverAudioProcessor::createEditor()
{
    return new AudioreceiverAudioProcessorEditor (*this);
}

//==============================================================================
void AudioreceiverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AudioreceiverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

bool AudioreceiverAudioProcessor::isMuted() const
{
    return mute;
}

void AudioreceiverAudioProcessor::setMute(bool val)
{
    mute = val;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioreceiverAudioProcessor();
}
