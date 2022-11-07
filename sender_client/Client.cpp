
#include "Client.h"

#define print_delim std::cout << "-------------------------------------------\n"

using namespace std;

Client::Client() {
    DBG("constructor called!");

    done = false;
    break_sender = false;

    cert_path = "C:\\Users\\Zack\\Desktop\\compSci\\projects\\corelink-server\\config\\ca-crt.pem";

    

}



void Client::on_error(corelink::core::network::channel_id_type host_id, in<std::string> err) {
    std::cerr << "Error in host id: " << host_id << "\n" << err << "\n";
    print_delim;
    done = true;
}

std::string Client::on_channel_init(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " connected\n";
    DBG("Host id: " << std::to_string(host_id) << " connected");
    print_delim;
    done = true;
    return ("Host id: " + std::to_string(host_id) + " connected\n");
}

void Client::on_channel_uninit(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " disconnected\n";
    DBG("Host id: " << std::to_string(host_id) << " disconnected");
    print_delim;
    done = true;
}

void Client::send_timed_data(
    ns_cl_core::network::channel_id_type data_channel_id,
    out<corelink::client::corelink_classic_client> client)
{
    std::string msg = "Hello world";
    while (!break_sender)
    {
        std::vector<uint8_t> data(msg.begin(), msg.end());
        client.
            send_data(
                data_channel_id,
                data,
                corelink::utils::json()
            );
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)
        );
    }
}

void Client::create_sender(ns_cl_core::network::channel_id_type control_channel_id,
    out<corelink::client::corelink_classic_client> client)
{
    // we are creating a TCP sender
    auto request =
        std::make_shared<corelink::client::request_response::modify_sender_stream_request>(
            ns_cl_core::network::constants::protocols::udp);

    // only websockets
    request->client_certificate_path = cert_path;
    request->alert = true;
    request->echo = true;
    request->workspace = "ZackAudio";
    request->stream_type = "audiotesting";
    request->meta = "Some information describing the stream";

    request->on_error = [](
        corelink::core::network::channel_id_type host_id,
        in<std::string> err)
    {
        std::cerr << "Error while sending data on the data channel: " << err << "\n";
        DBG("Error while sending data on the data channel: " << err);
        print_delim;
    };
    request->
        on_send = [](ns_cl_core::network::channel_id_type host_id, size_t bytes_sent)
    {
        //std::cout << "Sent out [" << bytes_sent << "] bytes on channel id" << host_id << "\n";
        print_delim;
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
                in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
            {

                if (response != nullptr) {
                    DBG("create_sender request returned sth!!");
                }
                else {
                    DBG("create_sender request returned a nullptr!!");
                }

                hostId = host_id;
                loading = false;

                std::cout << "Created sender\n";
                DBG("Created sender");

                print_delim;
            }
    );
}

void Client::list_streams(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client) {
    auto request = std::make_shared<corelink::client::request_response::list_streams_request>();
    request->workspaces = { "Holodeck" };

    DBG("list_stream");

    client.request(control_channel_id,
        corelink::client::corelink_functions::list_streams,
        request,
        [](
            corelink::core::network::channel_id_type /*host_id*/,
            in<std::string> /*msg*/,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
        {
            auto stream_list = std::static_pointer_cast<corelink::client::request_response::corelink_server_response_json>(
                response);
            DBG(stream_list->response.to_string());
        });

}

void Client::create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
    out<corelink::client::corelink_classic_client> client)
{
    DBG("receiver called");
    // we are creating a TCP sender
    auto request =
        std::make_shared<corelink::client::request_response::modify_receiver_stream_request>(
            ns_cl_core::network::constants::protocols::tcp);

    // only websockets
    request->
        client_certificate_path = cert_path;
    request->
        alert = true;
    request->
        echo = true;
    request->
        workspace = "Chalktalk";
    request->
        stream_type = "['sarthak_test_1']\n";

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
        DBG("Error while receiving data on the data channel: " << err);
        print_delim;
    };
    request->
        on_receive = [](ns_cl_core::network::channel_id_type host_id,
            in<corelink::client::constants::corelink_stream_id_type> stream_id,
            in<corelink::utils::json> headers,
            in<std::vector<uint8_t >> data)
    {
        std::cout << "Received some data from data channel: " << host_id << " corresponding to Corelink Stream ID: "
            << stream_id << '\n';
        DBG("Received some data from data channel: " << std::to_string(host_id) << " corresponding to Corelink Stream ID: " << std::to_string(stream_id));

        if (!data.

            empty()

            )
        {
            /*std::string s(data.begin(), data.end());
            std::cout << "Data: " << s << "\n";
            DBG("Data: " << s);*/

            std::vector<float> buffer_;
            for (int i = 0; i < data.size(); i += 4) {
                unsigned b1 = (unsigned)data[i];
                unsigned b2 = (unsigned)data[i + 1];
                unsigned b3 = (unsigned)data[i + 2];
                unsigned b4 = (unsigned)data[i + 3];

                b1 <<= 24;
                b2 <<= 24;
                b2 >>= 8;
                b3 <<= 24;
                b3 >>= 16;

                unsigned res = b1 + b2 + b3 + b4;
                float* p_ = (float*)(&res);
                buffer_.push_back(*p_);
            }

            for (const float& val : buffer_) {
                DBG(val);
            }

        }
        else
        {
            std::cout << "Data is empty!!!!!!!\n";
            DBG("Data is empty!!!");
        }
        if (!

            headers()

            .

            ObjectEmpty()

            )
        {
            std::cout << "Headers: " << headers.to_string(true) << '\n';
            DBG("Headers: " << headers.to_string(true));
        }
        else
        {
            std::cout << "Header is empty!!!!!!!\n";
            DBG("Header is empty!!!");
        }
        print_delim;
        //        done = true;
    };
    client.
        request(
            control_channel_id,
            ns_cl_client::corelink_functions::create_receiver,
            request,
            [&](corelink::core::network::channel_id_type /*host_id*/,
                in<std::string> /*msg*/,
                in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
            {
                std::cout << "Created Receiver\n";
                DBG("Created Receiver");
                print_delim;
            }
    );
}


std::string Client::add_on_update_handler(
    ns_cl_core::network::channel_id_type control_channel_id,
    out<corelink::client::corelink_classic_client> client)
{

    DBG("on_update_handler");

    client.request(
        control_channel_id,
        corelink::client::corelink_functions::server_callback_on_update,
        nullptr,
        [&](
            corelink::core::network::channel_id_type /*host_id*/,
            in<std::string> /*msg*/,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
        {
            if (response)
            {
                auto server_response =
                    std::static_pointer_cast<corelink::client::request_response::server_cb_on_update_response>(
                        response);
                //updateMsg("Update received");
                std::cout << "Update received\n";
                print_delim;
                DBG("Update received");
                DBG("-------------------------------------------");
                return("Update received");
            }
        }
    );
    return("None");
}

void Client::add_on_subscribed_handler(
    ns_cl_core::network::channel_id_type control_channel_id,
    out<corelink::client::corelink_classic_client> client)
{
    client.request(
            control_channel_id,
            corelink::client::corelink_functions::server_callback_on_subscribed,
            nullptr,
            [&](corelink::core::network::channel_id_type /*host_id*/,
                in<std::string> /*msg*/,
                in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
            {
                if (response) {
                    auto server_response = std::static_pointer_cast<corelink::client::request_response::server_cb_on_subscribed_response> (response);
                    std::cout << "Subscribe received\n";

                    print_delim;
                    DBG("Subscribe received");
                    DBG("Stream ID == " << server_response->stream_id);
                    DBG("-------------------------------------------");
                }

            }
    );
}

void Client::breakSender() {
    break_sender = !break_sender;
}

bool Client::get_done()const {
    return done;
}

bool Client::update_done(bool update) {
    done = update;
    return done;
}


std::string Client::getMsg()const {
    return msg;
}


std::string Client::updateMsg(std::string update) {
    msg = update;
    return msg;
}

void Client::close(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client) {
    DBG("disconnect!!!");
    client.request(
        control_channel_id,
        corelink::client::corelink_functions::disconnect,
        std::make_shared<corelink::client::request_response::disconnect_streams_request>(),
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string>,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
        {
            cout << "disconnecting!" << endl;
        }
    );
    client.destroy();
}

Client::~Client() {
    /*client.request(
        control_channel_id,
        corelink::client::corelink_functions::disconnect,
        nullptr,
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string>,
            in<std::shared_ptr<corelink::client::request_response::corelink_server_response_base>> response)
        {
            DBG("disconnecting!");
        }
    );

    client.destroy();*/
}

corelink::core::network::channel_id_type Client::getHostId()const {
    return hostId;
}

bool Client::getLoading()const {
    return loading;
}