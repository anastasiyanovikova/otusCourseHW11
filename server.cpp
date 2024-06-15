#include "server.h"

Session::Session(asio::io_context &context, asio::ip::tcp::socket socket, database& db):
m_context(context), m_socket(std::move(socket)), m_db(db)
{

}
void Session::run()
{
    do_read();
}

void Session::do_read()
{
    std::optional<std::string> cmd = m_helper.getCommand();
    if(cmd)
    {
        m_db.exec(*cmd, m_serverAnswer);
        do_write();
        return;
    }
    auto self(shared_from_this());
    m_socket.async_read_some(boost::asio::buffer(m_data, 1024),
    [this, self](boost::system::error_code er, std::size_t recv_n)
    {
        if(!er)
        {
            m_helper.addNewCommand(std::string_view(m_data.data(), recv_n));
            do_read();
        }
    });
}
void Session::do_write()
{
    auto self(shared_from_this());
    asio::async_write(m_socket, boost::asio::buffer(m_serverAnswer),
    [this, self](boost::system::error_code er, std::size_t)
    {
        if(er)
        {
            std::cerr << "error " << er.message() <<std::endl;
        }
        do_read();
    });
}

Server::Server(asio::io_context &context, unsigned short port):
m_context{context},
m_port_number(port),
acceptor{context, asio::ip::tcp::endpoint {asio::ip::make_address_v4("127.0.0.1"), port} }
{

}
void Server::accept()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_context);
    acceptor.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        assert(!ec);
        std::cout << "connected a client\n";
        auto session = std::make_shared<Session>(m_context, std::move(*socket), m_db);
        session->run();
        accept();
    });
}
