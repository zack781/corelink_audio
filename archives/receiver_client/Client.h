#pragma once

#define CORELINK_USE_CONCURRENT_QUEUE

//#pragma comment (lib, "libssl")
//#pragma comment (lib, "libcrypto")
//#pragma comment (lib, "websockets")

#include <iostream>


#include "corelink_all.hpp"
#include <JuceHeader.h>
#define CORELINK_USE_WEBSOCKET

namespace ns_cl_client = corelink::client;
namespace ns_cl_core = corelink::core;
namespace ns_cl_req_resp = ns_cl_client::request_response;
template<typename t> using in = corelink::in<t>;
template<typename t> using out = corelink::out<t>;

class Client {
public:
    // Corelink

    Client();
    ~Client();

    void on_error(corelink::core::network::channel_id_type host_id, in<std::string> err);
    std::string on_channel_init(corelink::core::network::channel_id_type host_id);
    void on_channel_uninit(corelink::core::network::channel_id_type host_id);
    std::string add_on_update_handler(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);
    void add_on_subscribed_handler(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);
    void create_sender(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);
    void create_receiver(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);

    void send_timed_data(ns_cl_core::network::channel_id_type data_channel_id, out<corelink::client::corelink_classic_client> client);

    void list_streams(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);

    bool get_done()const;

    bool update_done(bool update);

    corelink::ptr_to_const_val<char> getCertPath() const;

    std::string getMsg()const;
    std::string updateMsg(std::string update);

    void breakSender();

    void init();

    std::vector<float> getBuffer()const;
    void clearBuffer();
    int getBufferSize();
    double getSampleRate();
    int getNumChannels();

    bool getBufferGuard()const;

    void setBufferGuard(bool val);

    /*std::queue<std::vector<float>*>& getBufferQueue();*/
    std::queue<std::vector<float>*>& getBufferQueue();

    std::vector<std::vector<float>*>& getBufferVector();

    std::queue<long long>& getTimeStamp();

    bool receiverCreated();

    //--------------------
private:
    //Corelink
    bool done;
    bool break_sender;
    std::string msg;

    std::string cert_path;

    //------------------------
    std::vector<float> buffer_;
    long time;
    
    std::queue<std::vector<float>*> bufferQueue;
    std::queue<std::queue<float>*> bufferQueue_;
    std::queue<long long> timeQueue;

    std::vector<std::vector<float>*> bufferVector;

    std::queue<long long> timeStamp;

    bool receiverInit;
    //------------------------

    bool bufferGuard = true;

    int bufferSize_;
    double sampleRate;
    int numChannels_ = 2;

    int Disk;
    int sample = 0;
    
    //std::vector<long long> timeAtSent;

    /*corelink::client::corelink_classic_client client;

    corelink::client::corelink_client_connection_info info;

    corelink::core::network::channel_id_type control_channel_id;*/

    //--------------
};