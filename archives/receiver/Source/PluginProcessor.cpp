/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Client.cpp"

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

    deviceManager.initialise(2, 2, nullptr, false);

    auto currentSetup = deviceManager.getAudioDeviceSetup();

    currentSetup.bufferSize = 480;
    deviceManager.setAudioDeviceSetup(currentSetup, false);

    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::udp).set_certificate_path("C:\\Users\\Zack\\Desktop\\compSci\\projects\\corelink-server\\config\\ca-crt.pem");

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
        std::bind(&Client::on_error, &obj, std::placeholders::_1,
            std::placeholders::_2),
        std::bind(&Client::on_channel_init, &obj, std::placeholders::_1),
        std::bind(&Client::on_channel_uninit, &obj, std::placeholders::_1)
        /*error,
        init,
        uninit*/
    );

    while (!obj.get_done());

    obj.update_done(false);

    client.request(
        control_channel_id,
        corelink::client::corelink_functions::authenticate,
        std::make_shared<corelink::client::request_response::authenticate_client_request>(info.username, info.password),
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string>,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>>
            )
        {
            //obj.create_sender(host_id, client);
            //obj.list_streams(host_id, client);
            obj.create_receiver(host_id, client);
            //obj.breakSender();
        }
    );
}

AudioreceiverAudioProcessor::~AudioreceiverAudioProcessor()
{
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
    /*this->setPlayConfigDetails(this->getTotalNumInputChannels(), this->getTotalNumOutputChannels(), 4800, 512);*/
    /*DBG("bufferSize = " << samplesPerBlock);
    DBG("sampleRate = " << sampleRate);*/

    bufferSize = samplesPerBlock;
    sampleRate_ = sampleRate;
}

void AudioreceiverAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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

int AudioreceiverAudioProcessor::getBufferSize()const
{
    return bufferSize;
}

double AudioreceiverAudioProcessor::getSampleRate()const
{
    return sampleRate_;
}


void AudioreceiverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto* device = deviceManager.getCurrentAudioDevice();

    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();

    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    if (obj.getBufferQueue().size() > 0) {

        for (auto channel = 0; channel < 2; ++channel) {
            auto* outBuffer = buffer.getWritePointer(channel);
            for (int sample = 0; sample < obj.getBufferSize(); sample++) {
                //outBuffer[sample] = (*(obj.getBufferQueue().front()))[sample + (obj.getBufferSize() * channel)];
                //DBG((*obj.getBufferVector()[0])[sample + (obj.getBufferSize() * channel)]);
                //DBG((*obj.getBufferVector()[0])[sample + (obj.getBufferSize() * channel)]);
                /*BG("channel = " << channel);
                DBG("sample = " << sample);
                DBG("this buffer size = " << buffer.getNumSamples());
                DBG("bufferSize = " << obj.getBufferSize());
                DBG("vectorbuffersize = " << (*(obj.getBufferQueue().front())).size());*/
               
                outBuffer[sample] = (*(obj.getBufferQueue().front()))[sample + (obj.getBufferSize() * channel)];
            }
        }


        obj.getBufferQueue().pop();
    }
    else {
        for (auto channel = 0; channel < maxOutputChannels; ++channel) {
            auto* outBuffer = buffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                outBuffer[sample] = 0;
            }
        }
    }
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioreceiverAudioProcessor();
}
