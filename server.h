#pragma once

#include "boost/asio/buffer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/address_v4.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/system/error_code.hpp"
#include "boost/asio.hpp"
#include "boost/bind/bind.hpp"
#include <iostream>
#include <memory>
#include <cinttypes>
#include "database.h"
#include "commandHelper.h"

namespace asio = boost::asio;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::io_context &context, asio::ip::tcp::socket socket, database& db);
    void run();

    void do_read();
    void do_write();
    asio::io_context & m_context;
    boost::asio::ip::tcp::socket m_socket;
    std::array<char, 1024> m_data;
    database& m_db;

    commandHelper m_helper;
    std::string m_serverAnswer;
};

class Server {
public:
    Server(asio::io_context &context, unsigned short port);
    void accept();
private:
    asio::io_context & m_context;
    unsigned short m_port_number = 9000;
    asio::ip::tcp::acceptor acceptor;
    database m_db;
};