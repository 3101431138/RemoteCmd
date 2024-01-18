#pragma once
#include "select_client.h"
#include "shell.h"

class Cmder
{
public:
    Cmder(const std::string& serv_ip = "127.0.0.1");
    void run();
private:
    void on_user_connect(session_ptr sess);
    void on_user_disconnect(session_ptr sess);
    void on_user_read_packet(session_ptr sess, packet_ptr pkt);
    void on_cmd_complete(const char* cmd_result, uint32_t length);
private:
    Client                 clnt_;
    Shell                  shell_;
    std::weak_ptr<Session> sess_;
};

