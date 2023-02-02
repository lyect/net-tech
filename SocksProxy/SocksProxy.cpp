#include "SocksProxy.h"
#include "ClientSession.h"

// ###############
// #   PRIVATE   #
// ###############

void SocksProxy::do_accept() {
	acceptor->async_accept(
		[this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
			if (!ec) {
				std::make_shared<ClientSession>(std::move(socket), next_session_id)->start();
				next_session_id += 1;
				Logger::info("Accepted new connection");
			}
			do_accept();
		}
	);
}

// ##############
// #   PUBLIC   #
// ##############

SocksProxy::SocksProxy(boost::asio::io_context &context, short port) {
	acceptor = new boost::asio::ip::tcp::acceptor(
			context,
			boost::asio::ip::tcp::endpoint(
					boost::asio::ip::tcp::v4(),
					port
			)
	);

	next_session_id = 1;
	Logger::fine("Created SocksProxy!");

	do_accept();
}

SocksProxy::~SocksProxy() {
	Logger::info("Deleted boost::asio::io_context");
}