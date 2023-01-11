

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SenderAudioProcessor::SenderAudioProcessor()
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
    
    cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";
    
    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::websocket).set_certificate_path(cert_path);
    
    info.set_certificate_path(info_temp.client_certificate_path);
    info.set_hostname("corelink.hpc.nyu.edu");
    info.set_username(info_temp.username);
    info.set_password(info_temp.password);
    info.set_port_number(20012);
    
    if(!client.init_protocols())
    {
        throw corelink::commons::corelink_exception("Failed to init protocol info!");
    }
    
    control_channel_id = client.add_control_channel(
                                                    info.protocol,
                                                    info.hostname,
                                                    info.port_number,
                                                    info.client_certificate_path,
                                                    std::bind(&SenderAudioProcessor::on_error, this, std::placeholders::_1, std::placeholders::_2),
                                                    std::bind(&SenderAudioProcessor::on_channel_init, this, std::placeholders::_1),
                                                    std::bind(&SenderAudioProcessor::on_channel_uninit, this, std::placeholders::_1));
    
    while(!done);
    done = false;
    
    client.request(control_channel_id,
                   corelink::client::corelink_functions::authenticate,
                   std::make_shared<corelink::client::request_response::requests::authenticate_client_request>(info.username, info.password),
                   [&](corelink::core::network::channel_id_type host_id,
                       in<std::string>,
                       in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>>)
                   {
                        create_sender(host_id, client);
                    }
    );
    
}

SenderAudioProcessor::~SenderAudioProcessor()
{
}

void SenderAudioProcessor::on_channel_init(corelink::core::network::channel_id_type host_id)
{
    done = true;
}

void SenderAudioProcessor::on_channel_uninit(corelink::core::network::channel_id_type host_id)
{
    done = true;
}

void SenderAudioProcessor::on_error(corelink::core::network::channel_id_type host_id, in<std::string> err)
{
    DBG("Error in host id: " << host_id);
    done = true;
}

void SenderAudioProcessor::create_sender(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client)
{
    // we are creating a TCP sender
    auto request =
        std::make_shared<corelink::client::request_response::requests::modify_sender_stream_request>(ns_cl_core::network::constants::protocols::udp);

    // only websockets
    request->client_certificate_path = cert_path;
    request->alert = true;
    request->echo = true;
    request->workspace = "ZackAudio";
    request->stream_type = "audiotesting";
    
    request->on_error = [](
        corelink::core::network::channel_id_type host_id,
        in<std::string> err)
    {
        DBG("Error while sending data on the data channel: " << err);
    };
    request->
        on_send = [](ns_cl_core::network::channel_id_type host_id, size_t bytes_sent)
    {
        //std::cout << "Sent out [" << bytes_sent << "] bytes on channel id" << host_id << "\n";
    };
    request->
        on_init = [&](ns_cl_core::network::channel_id_type host_id)
    {
        /*std::thread sender_thread(std::bind(&Client::send_timed_data, this, std::placeholders::_1,
        std::placeholders::_2), host_id, std::ref(client));
        sender_thread.detach();*/

    };
    client.
        request(
        control_channel_id,
        ns_cl_client::corelink_functions::create_sender,
        request,
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string> /*msg*/,
            in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response)
            {
               
                hostId = host_id;
                loading = false;

//                DBG("Created sender");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            }
        );
}
    

//==============================================================================
const juce::String SenderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SenderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SenderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SenderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SenderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SenderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SenderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SenderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SenderAudioProcessor::getProgramName (int index)
{
    return {};
}

void SenderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SenderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    bufferSize = samplesPerBlock;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void SenderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SenderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SenderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (!loading && totalNumInputChannels >= 2)
    {
        juce::AudioBuffer<float> newBuffer;
        swapMove(newBuffer, buffer);
        
        std::async(std::launch::async, &SenderAudioProcessor::sendData, this, &newBuffer, newBuffer.getNumSamples());
    }
    
}

template<class T>
void SenderAudioProcessor::swapMove(T &a, T &b)
{
    T tmp{ std::move(a) };
    a = std::move(b);
    b = std::move(tmp);
}

void SenderAudioProcessor::sendData(juce::AudioBuffer<float>* buffer, int bufferSize)
{
    //DBG("sending data");
    std::vector<uint8_t> data;
    
    meta.append("bufferSize", bufferSize);
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    
    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        auto* inBuffer = buffer->getReadPointer(channel);
        //auto* channelData = buffer->getWritePointer(channel);
        for (int sample = 0; sample < buffer->getNumSamples(); sample++)
        {
            float val = inBuffer[sample];
            int* p = (int*)(&val);
            unsigned byte1 = (*p >> 24);
            unsigned byte2 = (*p >> 16);
            unsigned byte3 = (*p >> 8);
            unsigned byte4 = *p;
            data.push_back(byte1);
            data.push_back(byte2);
            data.push_back(byte3);
            data.push_back(byte4);
        }
    }
    client.send_data(hostId, std::move(data), std::move(meta));
}

//==============================================================================
bool SenderAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SenderAudioProcessor::createEditor()
{
    juce::AudioProcessorEditor* editor;
//    return new SenderAudioProcessorEditor (*this);
    return editor;
}

//==============================================================================
void SenderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SenderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//=========================X=====================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SenderAudioProcessor();
}
