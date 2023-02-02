#include <iostream>

#include <boost/lexical_cast.hpp>

#include "SocksProxy.h"
#include "Logger.h"

void print_usage(const char *executable_name) {
	printf(
			"Usage:\n"
			"\t%s [listen port]\n",
			executable_name
	);
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		Logger::error("Not enough arguments!");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (argc > 2) {
		Logger::error("Too many arguments!");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	unsigned short port;
	try {
		port = boost::lexical_cast<unsigned short>(argv[1]);
	}
	catch (boost::bad_lexical_cast &e) {
		Logger::error("Invalid port!");
		exit(EXIT_FAILURE);
	}

	boost::asio::io_context context;
	SocksProxy socks_proxy(context, port);
	context.run();

    return 0;
}
