#pragma once
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

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT 58888

class Packet
{
public:
    int         length;
    std::string data;
};

class Server;
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
    Session(Server* srv, SOCKET skt, SOCKADDR_IN addr);

    void set_connect_cb(connect_cb_t cb);

    void set_disconnect_cb(disconnect_cb_t cb);

    void set_read_packet_cb(read_packet_cb_t cb);

    void send(packet_ptr pkt);

    void establish();

    void close();

    SOCKET socket();

    const std::string& id();
public:
    void _send();
    void _recv();
private:
    void send_packet(packet_ptr pkt);
    void parse_packet();
private:
    SOCKET                 skt_;
    SOCKADDR_IN            addr_;
    Server*                srv_;
    std::queue<packet_ptr> send_pkt_queue_;
    std::string            send_pkt_buffer_;
    std::string            recv_pkt_buffer_;
    std::string            session_id_;

    connect_cb_t           connect_cb_;
    disconnect_cb_t        disconnect_cb_;
    read_packet_cb_t       read_complete_cb_;
};

class Server
{
public:
    Server();
   ~Server();
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
    SOCKET                          srv_skt_;
    std::thread                     srv_thread_;
    std::mutex                      srv_mutex_;
    std::atomic<bool>               srv_run_;
    std::queue<func_t>              func_queue_;
    std::map<SOCKET, session_ptr>   sess_map_;

    connect_cb_t                    connect_cb_;
    read_packet_cb_t                read_cb_;
    disconnect_cb_t                 disconnect_cb_;
};
