#ifndef SOCKS_PROXY_CLIENTSESSION_H
#define SOCKS_PROXY_CLIENTSESSION_H

#include <memory>
#include <vector>
#include <boost/asio.hpp>

#include "Logger.h"

class ClientSession : public std::enable_shared_from_this<ClientSession> {

private:

	static constexpr int MAX_BUFFER_SIZE = 4096;

	int session_id;

	boost::asio::ip::tcp::socket *client_socket;
	boost::asio::ip::tcp::socket *server_socket;
	boost::asio::ip::tcp::resolver *resolver;

	std::vector<char> client_buffer;
	std::vector<char> server_buffer;

	std::string server_address;
	std::string server_port;

	std::string process_hostname(int hostname_length);
	std::string process_ipv4_address();
	std::string process_port(int port_start_idx);

	void do_read_socks5_handshake_request();
	void do_write_socks5_handshake_response();
	void do_read_socks5_request();
	void do_resolve();
	void do_connect(boost::asio::ip::tcp::resolver::iterator &it);
	void do_write_socks5_response();
	void do_read_client();
	void do_read_server();
	void do_write_client(std::size_t length);
	void do_write_server(std::size_t length);
public:
	ClientSession(boost::asio::ip::tcp::socket socket, int session_id);

	void start();
};


#endif //SOCKS_PROXY_CLIENTSESSION_H
