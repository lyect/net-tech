#include "ClientSession.h"

// ###############
// #   PRIVATE   #
// ###############

std::string ClientSession::process_hostname(int hostname_length) {
	return std::string(&client_buffer[5], hostname_length);
}

std::string ClientSession::process_ipv4_address() {
	auto *address_start = &client_buffer[4];
	auto *address_pointer = (uint32_t *)address_start;
	auto address = boost::asio::detail::socket_ops::network_to_host_long(*address_pointer);
	auto boost_address = boost::asio::ip::address_v4(address);
	return boost_address.to_string();
}

std::string ClientSession::process_port(int port_start_idx) {
	auto *port_start = &client_buffer[port_start_idx];
	auto *port_pointer = (uint16_t *)port_start;
	auto port = boost::asio::detail::socket_ops::network_to_host_short(*port_pointer);
	return std::to_string(port);
}

void ClientSession::do_read_socks5_handshake_request() {
	auto self(shared_from_this());

	client_socket->async_receive(
			boost::asio::buffer(client_buffer),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (ec) {
					Logger::error(("handshake async_receive error: " + ec.message()).c_str(), session_id);
					return;
				}

				if (length < 3 || client_buffer[0] != static_cast<char>(0x05)) {
					Logger::error("Invalid handshake request. Closing session", session_id);
					return;
				}

				bool method_found = false;
				char methods_count = client_buffer[1];

				// Only 0x00 - "NO AUTHENTICATION REQUIRED"
				for (char method_num = 0; method_num < methods_count; ++method_num) {
					if (client_buffer[2 + method_num] == static_cast<char>(0x00)) {
						client_buffer[1] = static_cast<char>(0x00);
						method_found = true;
						break;
					}
				}

				if (method_found) {
					do_write_socks5_handshake_response();
				}
				else {
					Logger::warning("Method not found. Closing session", session_id);
				}
			}
	);
}

void ClientSession::do_write_socks5_handshake_response() {
	auto self(shared_from_this());

	client_socket->async_send(
			boost::asio::buffer(client_buffer, 2),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (ec) {
					Logger::error(("handshake async_send error: " + ec.message()).c_str(), session_id);
					return;
				}

				do_read_socks5_request();
			}
	);
}

void ClientSession::do_read_socks5_request() {
	auto self(shared_from_this());

	client_socket->async_receive(
			boost::asio::buffer(client_buffer),
			[this, self](boost::system::error_code ec, std::size_t length) {

				if (ec) {
					Logger::error(("read_request async_receive error: " + ec.message()).c_str(), session_id);
					return;
				}

				// Too short || not SOCKS5 || not connection request
				if (length < 5 || client_buffer[0] != 0x05 || client_buffer[1] != 0x01) {
					Logger::error("Invalid request. Closing session", session_id);
					return;
				}

				const char addr_type = client_buffer[3];

				switch (addr_type) {
				case 0x01: { // IPv4 address
					if (length != 10) {
						Logger::error("Invalid request length. Closing session", session_id);
						return;
					}

					this->server_address = process_ipv4_address();
					this->server_port = process_port(8);

					break;
				}
				case 0x03: { // Domain name
					char hostname_length = client_buffer[4];

					if (length != static_cast<size_t>(5 + hostname_length + 2)) {
						Logger::error("Invalid length of the request. Closing session", session_id);
						return;
					}

					this->server_address = process_hostname(hostname_length);
					this->server_port = process_port(5 + hostname_length);

					break;
				}
				default: {
					Logger::error("Unsupported address type. Closing session", session_id);
					return;
				}
				}

				do_resolve();
			}
	);
}

void ClientSession::do_resolve() {
	auto self(shared_from_this());

	resolver->async_resolve(
			boost::asio::ip::tcp::resolver::query({this->server_address, this->server_port}),
			[this, self](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator it) {
				if (ec) {
					Logger::error(
							("Failed to resolve " + this->server_address + ":" + this->server_port).c_str(),
							session_id
					);
					return;
				}

				do_connect(it);
			}
   );
}

void ClientSession::do_connect(boost::asio::ip::tcp::resolver::iterator &it) {
	auto self(shared_from_this());

	server_socket->async_connect(
			*it,
			[this, self](boost::system::error_code ec) {
				if (ec) {
					Logger::warning(
							("Failed to connect to " + this->server_address + ":" + this->server_port).c_str(),
							session_id
					);
					return;
				}

				Logger::fine(
						("Connected to " + this->server_address + ":" + this->server_port).c_str(),
						session_id
				);
				do_write_socks5_response();
			}
	);
}

void ClientSession::do_write_socks5_response() {
	auto self(shared_from_this());

	client_buffer[0] = 0x05; // PROTOCOL VERSION
	client_buffer[1] = 0x00; // CONNECTION SUCCEEDED
	client_buffer[2] = 0x00; // RESERVED
	client_buffer[3] = 0x01; // IPv4

	uint32_t real_server_address = server_socket->remote_endpoint().address().to_v4().to_ulong();
	uint16_t real_server_port = boost::asio::detail::socket_ops::network_to_host_short(
			server_socket->remote_endpoint().port()
	);

	std::memcpy(&client_buffer[4], &real_server_address, 4);
	std::memcpy(&client_buffer[8], &real_server_port, 2);

	client_socket->async_send(
			boost::asio::buffer(client_buffer, 10),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (ec) {
					Logger::error(("write_response async_send error: " + ec.message()).c_str(), session_id);
					return;
				}

				do_read_client();
				do_read_server();
			}
	);
}

void ClientSession::do_read_client() {
	auto self(shared_from_this());

	client_socket->async_receive(
			boost::asio::buffer(client_buffer),
			[this, self](boost::system::error_code ec, std::size_t length) {

				if (ec) {
					Logger::error(("read_client async_receive error: " + ec.message()).c_str(), session_id);
					client_socket->close();
					server_socket->close();
					return;
				}

				Logger::info(("Read from client " + std::to_string(length) + " bytes").c_str(), session_id);

				do_write_client(length);
			}
	);
}

void ClientSession::do_read_server() {
	auto self(shared_from_this());

	server_socket->async_receive(
			boost::asio::buffer(server_buffer),
			[this, self](boost::system::error_code ec, std::size_t length) {

				if (ec) {
					Logger::error(("read_server async_receive error: " + ec.message()).c_str(), session_id);
					client_socket->close();
					server_socket->close();
					return;
				}

				Logger::info(("Read from server " + std::to_string(length) + " bytes").c_str(), session_id);

				do_write_server(length);
			}
	);
}

void ClientSession::do_write_client(std::size_t length) {
	auto self(shared_from_this());

	server_socket->async_send(
			boost::asio::buffer(client_buffer, length),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (ec) {
					Logger::error(("write_client async_send error: " + ec.message()).c_str(), session_id);
					client_socket->close();
					server_socket->close();
					return;
				}

				do_read_client();
			}
	);
}

void ClientSession::do_write_server(std::size_t length) {
	auto self(shared_from_this());

	client_socket->async_send(
			boost::asio::buffer(server_buffer, length),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (ec) {
					Logger::error(("write_server async_send error: " + ec.message()).c_str(), session_id);
					client_socket->close();
					server_socket->close();
					return;
				}

				do_read_server();
			}
	);
}


// ##############
// #   PUBLIC   #
// ##############

ClientSession::ClientSession(boost::asio::ip::tcp::socket socket, int session_id) {
	server_socket = new boost::asio::ip::tcp::socket(socket.get_executor());
	resolver = new boost::asio::ip::tcp::resolver(socket.get_executor());

	client_socket = new boost::asio::ip::tcp::socket(std::move(socket));

	server_buffer.resize(MAX_BUFFER_SIZE);
	client_buffer.resize(MAX_BUFFER_SIZE);

	this->session_id = session_id;
}

void ClientSession::start() {
	auto self(shared_from_this());
	do_read_socks5_handshake_request();
}
