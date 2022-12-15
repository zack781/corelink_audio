/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Client.cpp"
#include <future>

//==============================================================================
CorelinkvstAudioProcessor::CorelinkvstAudioProcessor()
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

    
    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::udp).set_certificate_path(".\\config\\ca-crt.pem");
    
    info.set_certificate_path(info_temp.client_certificate_path);
    info.set_hostname("corelink.hpc.nyu.edu");
    info.set_username(info_temp.username);
    info.set_password(info_temp.password);
    info.set_port_number(20010);

    if (!client.init_protocols())
    {
        throw corelink::commons::corelink_exception(
            "failed to initialize protocol information. please contact corelink development");
    }

    control_channel_id = client.add_control_channel(
        info.protocol,
        info.hostname,
        info.port_number,
        info.client_certificate_path,
        /*[this](corelink::core::network::channel_id_type host_id, in<std::string> err) { this->on_error(host_id, err); },
        [this](corelink::core::network::channel_id_type host_id) { this->on_channel_init(host_id); },
        [this](corelink::core::network::channel_id_type host_id) { this->on_channel_uninit(host_id); }*/
        std::bind(&Client::on_error, &clientobj, std::placeholders::_1,
            std::placeholders::_2),
        std::bind(&Client::on_channel_init, &clientobj, std::placeholders::_1),
        std::bind(&Client::on_channel_uninit, &clientobj, std::placeholders::_1)
        /*error,
        init,
        uninit*/
    );

    while (!clientobj.get_done());

    clientobj.update_done(false);

    client.request(
        control_channel_id,
        corelink::client::corelink_functions::authenticate,
        std::make_shared<corelink::client::request_response::authenticate_client_request>(info.username, info.password),
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string>,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>>
            )
        {
            DBG("Authentication seems okay...\n");
            clientobj.create_sender(host_id, client);
            //obj.list_streams(host_id, client);
            //obj.create_receiver(host_id, client);
            //obj.breakSender();
        }
    );

    bufferSize = 0;
    sampleRate_ = 0;


}

CorelinkvstAudioProcessor::~CorelinkvstAudioProcessor()
{
    

}

//==============================================================================
const juce::String CorelinkvstAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CorelinkvstAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CorelinkvstAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CorelinkvstAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CorelinkvstAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CorelinkvstAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CorelinkvstAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CorelinkvstAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CorelinkvstAudioProcessor::getProgramName (int index)
{
    return {};
}

void CorelinkvstAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CorelinkvstAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
  
    bufferSize = samplesPerBlock;
    sampleRate_ = sampleRate;

    /*floatBuffer.clear();
    for (int i = 0; i < samplesPerBlock * 2; ++i) {
        floatBuffer.push_back(0.0);
    }*/

    
}

void CorelinkvstAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    client.request(
        control_channel_id,
        corelink::client::corelink_functions::disconnect,
        nullptr,
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string>,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
        {
        }
    );

    client.destroy();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CorelinkvstAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

int CorelinkvstAudioProcessor::getBufferSize()const 
{
    return bufferSize;
}

double CorelinkvstAudioProcessor::getSampleRate()const
{
    return sampleRate_;
}

void CorelinkvstAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

   /*juce::AudioBuffer<float>* ptr = &buffer;

   ostringstream get_the_address;
   get_the_address << ptr;
   string address = get_the_address.str();

   DBG(address);*/

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    /*auto now = std::chrono::steady_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds> (now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::microseconds> (epoch);
    timeStamp = value.count();*/

    //int count = 0;
    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //{
    //    auto* inBuffer = buffer.getReadPointer(channel);
    //    auto* channelData = buffer.getWritePointer (channel);

    //    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
    //    {
    //        /*DBG("channel = " << channel);
    //        DBG(sample);*/
    //        auto val = inBuffer[sample];
    //        DBG("val = " << val);
    //        //floatBuffer[count] = val;
    //        //++count;
    //        channelData[sample] = 0;
    //        
    //    }
    //}
    
    
    /*auto* inBuffer_0 = buffer.getReadPointer(0);
    auto* inBuffer_1 = buffer.getReadPointer(1);

    std::copy(inBuffer_0[0], inBuffer_0[buffer.getNumSamples()], lst_0);
    std::copy(inBuffer_1[0], inBuffer_1[buffer.getNumSamples()], lst_1);

    bufferQueue.push(lst_0);
    bufferQueue.push(lst_1);*/

    if (!clientobj.getLoading() && totalNumInputChannels >= 2)
    {

        //bufferQueue.push(new juce::AudioBuffer<float>(buffer));
        //sendData();
        //m_Futures.push_back(std::async(std::launch::async, &CorelinkvstAudioProcessor::sendData, this, bufferQueue));
        //sendData(bufferQueue);
        //sendData_(bufferQueue);
        //std::async(std::launch::async, &CorelinkvstAudioProcessor::sendData_, this, &buffer);
        std::async(std::launch::async, &CorelinkvstAudioProcessor::sendData_, this, new juce::AudioBuffer<float>(buffer));
        
    }

}

void CorelinkvstAudioProcessor::sendData(std::queue<juce::AudioBuffer<float>*>& bufferQueue)
{

    /*std::string msg = "hello world";
    std::vector<uint8_t> data(msg.begin(), msg.end());

    client.send_data(clientobj.getHostId(), data, corelink::utils::json());*/

    std::vector<uint8_t> data;

    //meta.append("timeStamp", timeStamp);
    meta.append("bufferSize", bufferSize);

    //std::vector<float> temp = { 3.14, 9.81, 5.64, -5.06 };

    auto totalNumInputChannels = getTotalNumInputChannels();

    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        //std::lock_guard<std::mutex> lock(s_QueueMutex);
        auto* inBuffer = bufferQueue.front()->getReadPointer(channel);
        auto* outBuffer = bufferQueue.front()->getWritePointer(channel);
        for (int sample = 0; sample < bufferQueue.front()->getNumSamples(); sample++)
        {
            DBG((inBuffer[sample]));
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
            outBuffer[sample] = 0;
        }
    }

    //DBG("data size = " << data.size());

    //for (float val : bufferQueue.front()) {
    /*for (int i=0; i < bufferSize; ++i){
        int* p = (int*)(&(bufferQueue.front()[i]));
        unsigned byte1 = (*p >> 24);
        unsigned byte2 = (*p >> 16);
        unsigned byte3 = (*p >> 8);
        unsigned byte4 = *p;
        data.push_back(byte1);
        data.push_back(byte2);
        data.push_back(byte3);
        data.push_back(byte4);
    }*/

    bufferQueue.pop();

    client.send_data(clientobj.getHostId(), data, meta);

}


void CorelinkvstAudioProcessor::sendData_(juce::AudioBuffer<float>* buffer)
{
    std::vector<uint8_t> data;

    //meta.append("timeStamp", timeStamp);
    meta.append("bufferSize", bufferSize);

    //std::vector<float> temp = { 3.14, 9.81, 5.64, -5.06 };

    auto totalNumInputChannels = getTotalNumInputChannels();

    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        auto* inBuffer = buffer->getReadPointer(channel);
        auto* channelData = buffer->getWritePointer(channel);
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

    //for (float val : bufferQueue.front())
    //{
    //    //DBG((inBuffer[sample]));
    //    int* p = (int*)(&val);
    //    unsigned byte1 = (*p >> 24);
    //    unsigned byte2 = (*p >> 16);
    //    unsigned byte3 = (*p >> 8);
    //    unsigned byte4 = *p;
    //    data.push_back(byte1);
    //    data.push_back(byte2);
    //    data.push_back(byte3);
    //    data.push_back(byte4);
    //}

    client.send_data(clientobj.getHostId(), data, meta);

}


//==============================================================================
bool CorelinkvstAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CorelinkvstAudioProcessor::createEditor()
{
    return new CorelinkvstAudioProcessorEditor (*this);
}

//==============================================================================
void CorelinkvstAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CorelinkvstAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CorelinkvstAudioProcessor();
}
