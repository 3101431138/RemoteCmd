#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdint.h>
#include <assert.h>
#include <memory>
#include <map>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <sstream>
#include <functional>
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include "console_color_log.h"

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT 58888

class Packet
{
public:
    int length;
    std::string data;
};

class Client;
class Session;

typedef std::shared_ptr<Packet>                      packet_ptr;
typedef std::shared_ptr<Session>                     session_ptr;
typedef std::function<void()>                        func_t;
typedef std::function<void(session_ptr)>             connect_cb_t;
typedef std::function<void(session_ptr, packet_ptr)> read_packet_cb_t;
typedef std::function<void(session_ptr)>             disconnect_cb_t;

class Session : public std::enable_shared_from_this<Session>
{
public:
    friend class Client;

    Session(Client* srv, SOCKET skt);

   ~Session();

    void set_connect_cb(connect_cb_t cb);

    void set_disconnect_cb(disconnect_cb_t cb);

    void set_read_packet_cb(read_packet_cb_t cb);

    void send(packet_ptr pkt);

    void establish();

    void close();

    SOCKET socket();
private:
    void _send();
    void _recv();
    void send_packet(packet_ptr pkt);
    void parse_packet();
private:
    SOCKET                 skt_;
    Client*                srv_;
    std::queue<packet_ptr> send_pkt_queue_;
    std::string            send_pkt_buffer_;
    std::string            recv_pkt_buffer_;

    connect_cb_t           connect_cb_;
    disconnect_cb_t        disconnect_cb_;
    read_packet_cb_t       read_complete_cb_;
};

class Client
{
public:
    Client(const std::string& serv_ip);
   ~Client();
    void run();
    void runInLoop(const func_t& func);
    void set_connect_cb(connect_cb_t cb);
    void set_disconnect_cb(disconnect_cb_t cb);
    void set_read_packet_cb(read_packet_cb_t cb);
private:
    void internal_do_func();
    void internal_init();
    void internal_loop();

    void handle_connect(session_ptr sess);
    void handle_read_packet(session_ptr sess, packet_ptr pkt);
    void handle_disconnect(session_ptr sess);
private:
    SOCKET                          clt_skt_;
    std::thread                     srv_thread_;
    std::mutex                      srv_mutex_;
    std::atomic<bool>               srv_run_;
    std::queue<func_t>              func_queue_;
    std::map<SOCKET, session_ptr>   sess_map_;

    connect_cb_t                    connect_cb_;
    read_packet_cb_t                read_cb_;
    disconnect_cb_t                 disconnect_cb_;

    std::string                     serv_ip_;
};
