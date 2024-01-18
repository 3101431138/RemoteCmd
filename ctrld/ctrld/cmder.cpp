#include "cmder.h"

Cmder::Cmder(const std::string& serv_ip)
    : clnt_(serv_ip)
{

}

void Cmder::run()
{
    clnt_.set_connect_cb(std::bind(&Cmder::on_user_connect, this, std::placeholders::_1));
    clnt_.set_disconnect_cb(std::bind(&Cmder::on_user_disconnect, this, std::placeholders::_1));
    clnt_.set_read_packet_cb(std::bind(&Cmder::on_user_read_packet, this, std::placeholders::_1, std::placeholders::_2));
    clnt_.run();
}

void Cmder::on_user_connect(session_ptr sess)
{
    sess_ = sess;
    shell_.set_cmd_complete_cb(std::bind(&Cmder::on_cmd_complete, this, std::placeholders::_1, std::placeholders::_2));
    shell_.invoke();

    INFO("Connect Successfuly");
}

void Cmder::on_user_disconnect(session_ptr sess)
{
    INFO("DisConnect Successfuly");
}

void Cmder::on_user_read_packet(session_ptr sess, packet_ptr pkt)
{
    shell_.invoke_commmand(pkt->data);
}

void Cmder::on_cmd_complete(const char* cmd_result, uint32_t length)
{
    packet_ptr pkt(new Packet);
    pkt->data.assign(cmd_result, length);
    pkt->length = length;

    session_ptr sess = sess_.lock();
    if (sess)
    {
        if (!sess_.expired())
        {
            sess->send(pkt);
        }
    }
}
