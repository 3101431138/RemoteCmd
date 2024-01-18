#include "cmder.h"

#pragma warning(disable: 6031)

Session::Session(Client * srv, SOCKET skt)
    : srv_(srv)
    , skt_(skt)
{
}

Session::~Session()
{
    
}

void Session::set_connect_cb(connect_cb_t cb)
{
    connect_cb_ = cb;
}

void Session::set_disconnect_cb(disconnect_cb_t cb)
{
    disconnect_cb_ = cb;
}

void Session::set_read_packet_cb(read_packet_cb_t cb)
{
    read_complete_cb_ = cb;
}

void Session::send(packet_ptr pkt)
{
    srv_->runInLoop(std::bind(&Session::send_packet, this, pkt));
}

void Session::establish()
{
    if (connect_cb_) connect_cb_(shared_from_this());
}

void Session::close()
{
    srv_->runInLoop([this]()
    {
        closesocket(skt_);
        if (disconnect_cb_) disconnect_cb_(shared_from_this());
    });
}

SOCKET Session::socket()
{
    return skt_;
}

void Session::_send()
{
    if (send_pkt_buffer_.size() == 0)
    {
        if (send_pkt_queue_.size() > 0)
        {
            packet_ptr pkt = std::move(send_pkt_queue_.front());
            send_pkt_queue_.pop();
            std::stringstream ss;
            ss.write((char*)&pkt->length, sizeof(pkt->length));
            send_pkt_buffer_.append(ss.str());
            send_pkt_buffer_.append(pkt->data);
        }
    }
    if (send_pkt_buffer_.size() > 0)
    {
        int ret = ::send(skt_, send_pkt_buffer_.c_str(), send_pkt_buffer_.size(), 0);
        if (ret <= 0)
        {
            closesocket(skt_);
            if (disconnect_cb_) disconnect_cb_(shared_from_this());
        }
        send_pkt_buffer_.erase(0, ret);
    }
}

void Session::_recv()
{
    char buf[1024 * 10];
    int ret = ::recv(skt_, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        closesocket(skt_);
        if (disconnect_cb_) disconnect_cb_(shared_from_this());
        return;
    }
    std::stringstream ss;
    ss.write(buf, ret);
    recv_pkt_buffer_.append(ss.str());
    parse_packet();
}

void Session::send_packet(packet_ptr pkt)
{
    send_pkt_queue_.push(pkt);
}

void Session::parse_packet()
{
    if (recv_pkt_buffer_.size() > sizeof(Packet::length))
    {
        int length = *(int*)recv_pkt_buffer_.c_str();
        int total_length = (sizeof(Packet::length) + length);
        if (recv_pkt_buffer_.size() >= (size_t)total_length)
        {
            packet_ptr pkt(new Packet);
            pkt->length = length;
            pkt->data.assign(recv_pkt_buffer_.c_str() + sizeof(Packet::length), length);
            if (read_complete_cb_) read_complete_cb_(shared_from_this(), pkt);
            recv_pkt_buffer_.erase(0, total_length);
        }
    }
}

Client::Client(const std::string& serv_ip)
    : clt_skt_(INVALID_SOCKET)
    , serv_ip_(serv_ip)
{
    WSADATA wsa_data = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

Client::~Client()
{
    srv_run_.store(false);
    srv_thread_.join();
    WSACleanup();
}

void Client::handle_connect(session_ptr sess)
{
    if (connect_cb_)
        connect_cb_(sess);
}

void Client::handle_read_packet(session_ptr sess, packet_ptr pkt)
{
    if (read_cb_)
        read_cb_(sess, pkt);
}

void Client::handle_disconnect(session_ptr sess)
{
    if (disconnect_cb_)
        disconnect_cb_(sess);
    sess_map_.erase(sess->socket());
}

void Client::run()
{
    srv_run_.store(true);
    srv_thread_ = std::thread(&Client::internal_loop, this);
}

void Client::runInLoop(const func_t & func)
{
    std::unique_lock<std::mutex> lock(srv_mutex_);
    func_queue_.push(func);
}

void Client::set_connect_cb(connect_cb_t cb)
{
    connect_cb_ = cb;
}

void Client::set_read_packet_cb(read_packet_cb_t cb)
{
    read_cb_ = cb;
}

void Client::set_disconnect_cb(disconnect_cb_t cb)
{
    disconnect_cb_ = cb;
}

void Client::internal_do_func()
{
    std::unique_lock<std::mutex> lock(srv_mutex_);
    if (func_queue_.size() > 0)
    {
        func_t func = std::move(func_queue_.front());
        func_queue_.pop();
        func();
    }
}

void Client::internal_init()
{
    clt_skt_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(clt_skt_ == INVALID_SOCKET)
    {
        ERRO("create socket failed! (errcode: %d)", GetLastError());
        system("pause"); exit(0);
    }

    SOCKADDR_IN srv_addr          = { 0 };
    srv_addr.sin_family           = AF_INET;
    srv_addr.sin_port             = htons(SERVER_PORT);
    srv_addr.sin_addr.S_un.S_addr = inet_addr(serv_ip_.c_str());

    if (connect(clt_skt_, (SOCKADDR*)&srv_addr, sizeof(srv_addr)) == SOCKET_ERROR)
    {
        ERRO("connect server failed! (errcode: %d)", GetLastError());
        system("pause"); exit(0);
    }
}

void Client::internal_loop()
{
    fd_set temp_fds;
    fd_set read_fds;
    fd_set writ_fds;
    timeval tm = { 0, 1000 * 10 };

    internal_init();

    sess_map_[clt_skt_] = session_ptr(new Session(this, clt_skt_));
    sess_map_[clt_skt_]->set_connect_cb(std::bind(&Client::handle_connect, this, std::placeholders::_1));
    sess_map_[clt_skt_]->set_disconnect_cb(std::bind(&Client::handle_disconnect, this, std::placeholders::_1));
    sess_map_[clt_skt_]->set_read_packet_cb(std::bind(&Client::handle_read_packet, this, std::placeholders::_1, std::placeholders::_2));
    sess_map_[clt_skt_]->establish();

    while (srv_run_.load())
    {
        internal_do_func();

        FD_ZERO(&temp_fds);
        for (std::map<SOCKET, session_ptr>::iterator it = sess_map_.begin(); it != sess_map_.end(); it++)
        {
            FD_SET(it->second->socket(), &temp_fds);
        }

        if(sess_map_.empty())
        {
            system("pause"); exit(0);
        }

        read_fds = writ_fds = temp_fds;

        int ret = select(64, &read_fds, &writ_fds, NULL, &tm);

        if(ret == 0)
        {
            continue;
        }

        if(ret == SOCKET_ERROR)
        {
            ERRO("select failed! (errcode: %d)", GetLastError());
            system("pause"); exit(0);
        }

        for (int i = 0; i < (int)temp_fds.fd_count; i++)
        {
            //Read
            if (FD_ISSET(temp_fds.fd_array[i], &read_fds))
            {
                sess_map_[temp_fds.fd_array[i]]->_recv();
            }
            //Write
            else if (FD_ISSET(temp_fds.fd_array[i], &writ_fds))
            {
                sess_map_[temp_fds.fd_array[i]]->_send();
            }
        }
    }
}
