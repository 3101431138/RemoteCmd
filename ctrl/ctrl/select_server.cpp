#include "select_server.h"

Session::Session(Server * srv, SOCKET skt, SOCKADDR_IN addr)
    : srv_(srv)
    , skt_(skt)
    , addr_(addr)
{
    char port[8] = { 0 };
    sprintf(port, "%d", htons(addr.sin_port));
    session_id_.append(inet_ntoa(addr.sin_addr));
    session_id_.append(":");
    session_id_.append(port);
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
    session_ptr s = shared_from_this();
    if(connect_cb_) connect_cb_(s);
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

const std::string & Session::id()
{
    return session_id_;
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
    if(send_pkt_buffer_.size() > 0)
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
        if (recv_pkt_buffer_.size() >= total_length)
        {
            packet_ptr pkt(new Packet);
            pkt->data.assign(recv_pkt_buffer_.c_str() + sizeof(Packet::length), length);
            pkt->length = length;
            if (read_complete_cb_) read_complete_cb_(shared_from_this(), pkt);
            recv_pkt_buffer_.erase(0, total_length);
        }
    }
}

Server::Server()
    : srv_skt_(INVALID_SOCKET)
{
    WSADATA wsa_data = { 0 };
    if (!WSAStartup(MAKEWORD(2, 2), &wsa_data))
    {
        printf("%d", GetLastError());
    }
}

Server::~Server()
{
    srv_run_.store(false);
    srv_thread_.join();
    WSACleanup();
}

void Server::handle_connect(session_ptr sess)
{
    if (connect_cb_)
        connect_cb_(sess);
}

void Server::handle_read_packet(session_ptr sess, packet_ptr pkt)
{
    if (read_cb_)
        read_cb_(sess, pkt);
}

void Server::handle_disconnect(session_ptr sess)
{
    if (disconnect_cb_)
        disconnect_cb_(sess);
    sess_map_.erase(sess->socket());
}

void Server::run()
{
    srv_run_.store(true);
    srv_thread_ = std::thread(&Server::internal_loop, this);
}

void Server::runInLoop(const func_t & func)
{
    std::unique_lock<std::mutex> lock(srv_mutex_);
    func_queue_.push(func);
}

void Server::set_connect_cb(connect_cb_t cb)
{
    connect_cb_ = cb;
}

void Server::set_read_packet_cb(read_packet_cb_t cb)
{
    read_cb_ = cb;
}

void Server::set_disconnect_cb(disconnect_cb_t cb)
{
    disconnect_cb_ = cb;
}

void Server::internal_do_func()
{
    std::unique_lock<std::mutex> lock(srv_mutex_);
    if(func_queue_.size() > 0)
    {
        func_t func = std::move(func_queue_.front());
        func_queue_.pop();
        func();
    }
}

void Server::internal_init()
{
    srv_skt_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    assert(srv_skt_ != INVALID_SOCKET);

    SOCKADDR_IN srv_addr = { 0 };
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SERVER_PORT);
    srv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    assert(bind(srv_skt_, (SOCKADDR*)&srv_addr, sizeof(srv_addr)) != SOCKET_ERROR);

    assert(listen(srv_skt_, 5) != SOCKET_ERROR);
}

void Server::internal_loop()
{
    fd_set temp_fds;
    fd_set read_fds;
    fd_set writ_fds;
    timeval tm = { 0, 1000 * 10 };

    internal_init();

    FD_ZERO(&temp_fds);
    FD_SET(srv_skt_, &temp_fds);

    while (srv_run_.load())
    {
        internal_do_func();

        FD_ZERO(&temp_fds);
        FD_SET(srv_skt_, &temp_fds);
        for(std::map<SOCKET, session_ptr>::iterator it = sess_map_.begin(); it != sess_map_.end(); it++)
        {
            FD_SET(it->second->socket(), &temp_fds);
        }

        read_fds = writ_fds = temp_fds;

        int ret = select(64, &read_fds, &writ_fds, NULL, &tm);

        if (ret == 0)
        {
            continue;
        }

        if (ret == SOCKET_ERROR)
        {
            //ERRO("select failed! (errcode: %d)", GetLastError());
            system("pause"); exit(0);
        }

        for (int i = 0; i < temp_fds.fd_count; i++)
        {
            if (FD_ISSET(temp_fds.fd_array[i], &read_fds))
            {
                //Accept
                if (temp_fds.fd_array[i] == srv_skt_ && temp_fds.fd_count < FD_SETSIZE)
                {
                    SOCKADDR_IN clt_addr = { 0 };
                    int clt_len = sizeof(clt_addr);
                    SOCKET clt_skt = accept(srv_skt_, (SOCKADDR*)&clt_addr, &clt_len);
                    assert(clt_skt != INVALID_SOCKET);

                    FD_SET(clt_skt, &temp_fds);

                    sess_map_[clt_skt] = std::make_shared<Session>(this, clt_skt, clt_addr);
                    sess_map_[clt_skt]->set_connect_cb(std::bind(&Server::handle_connect, this, std::placeholders::_1));
                    sess_map_[clt_skt]->set_disconnect_cb(std::bind(&Server::handle_disconnect, this, std::placeholders::_1));
                    sess_map_[clt_skt]->set_read_packet_cb(std::bind(&Server::handle_read_packet, this, std::placeholders::_1, std::placeholders::_2));
                    sess_map_[clt_skt]->establish();
                }
                //Read
                else 
                {
                    sess_map_[temp_fds.fd_array[i]]->_recv();
                }
            }
            //Write
            else if (FD_ISSET(temp_fds.fd_array[i], &writ_fds))
            {
                sess_map_[temp_fds.fd_array[i]]->_send();
            }
        }
    }
}
