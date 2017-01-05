#include "ed2k.h"

using boost::asio::ip::tcp;
using boost::asio::ip::address;

class session : public boost::enable_shared_from_this<session> {
   public:
    session(boost::asio::io_service &io_service) : socket_(io_service) {}

    void start() {
        static tcp::no_delay option(true);
        socket_.set_option(option);

        boost::asio::async_read_until(socket_, sbuf_, "\n", boost::bind(&session::handle_read, shared_from_this(),
                                                                        boost::asio::placeholders::error,
                                                                        boost::asio::placeholders::bytes_transferred));
    }

    tcp::socket &socket() { return socket_; }

   private:
    void handle_write(const boost::system::error_code &error, size_t bytes_transferred) {
        boost::asio::async_read_until(socket_, sbuf_, "\n", boost::bind(&session::handle_read, shared_from_this(),
                                                                        boost::asio::placeholders::error,
                                                                        boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code &error, size_t bytes_transferred) {
        boost::asio::async_write(
            socket_, sbuf_, boost::bind(&session::handle_write, shared_from_this(), boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
    }

   private:
    tcp::socket socket_;
    boost::asio::streambuf sbuf_;
};

typedef boost::shared_ptr<session> session_ptr;

class server {
   public:
    server(boost::asio::io_service &io_service, tcp::endpoint &endpoint)
        : io_service_(io_service), acceptor_(io_service, endpoint) {
        session_ptr new_session(new session(io_service_));
        acceptor_.async_accept(new_session->socket(), boost::bind(&server::handle_accept, this, new_session,
                                                                  boost::asio::placeholders::error));
    }

    void handle_accept(session_ptr new_session, const boost::system::error_code &error) {
        if (error) {
            return;
        }

        new_session->start();
        new_session.reset(new session(io_service_));
        acceptor_.async_accept(new_session->socket(), boost::bind(&server::handle_accept, this, new_session,
                                                                  boost::asio::placeholders::error));
    }

    void run() { io_service_.run(); }

   private:
    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
};

int dosrv(int argc, char *argv[]) {
    printf("do server->%s\n", argv[1]);
    boost::asio::io_service io_service;
    tcp::endpoint endpoint(tcp::v4(), 10000);

    server s(io_service, endpoint);
    s.run();
    return 0;
}

typedef boost::shared_ptr<client> client_ptr;

int doc(int argc, char *argv[]) {
    printf("do client->%s\n", argv[1]);
    boost::asio::io_service io_service;
    tcp::endpoint endpoint(address::from_string("127.0.0.1"), 10000);

    client_ptr new_session(new client(io_service, endpoint));
    new_session->start();
    io_service.run();
    std::string ss;
    std::cin >> ss;
    return 0;
}

int main(int argc, char *argv[]) {
    printf("%s\n", argv[1]);
    if (strcmp(argv[1], "srv") == 0) {
        return dosrv(argc, argv);
    } else if (strcmp(argv[1], "c") == 0) {
        return doc(argc, argv);
    }
    return 0;
}
