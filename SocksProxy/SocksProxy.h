#ifndef SOCKS_PROXY_SOCKSPROXY_H
#define SOCKS_PROXY_SOCKSPROXY_H

#include <boost/asio.hpp>
#include <memory>

#include "Logger.h"
#include "ClientSession.h"

class SocksProxy {
private:

	boost::asio::ip::tcp::acceptor *acceptor;
	int next_session_id;

	void do_accept();

public:
	SocksProxy(boost::asio::io_context &context, short port);
	~SocksProxy();
};


#endif //SOCKS_PROXY_SOCKSPROXY_H
