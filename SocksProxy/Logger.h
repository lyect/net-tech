#ifndef SOCKS_PROXY_LOGGER_H
#define SOCKS_PROXY_LOGGER_H

#include <string>

class Logger {

private:

	static constexpr auto &INFO_WRAPPER = "[INFO] %s\n";
	static constexpr auto &WARNING_WRAPPER = "[WARNING] %s\n";
	static constexpr auto &ERROR_WRAPPER = "[ERROR] %s\n";
	static constexpr auto &FINE_WRAPPER = "[FINE] %s\n";

	static constexpr auto &SESSION_INFO_WRAPPER = "[INFO] Session #%d: %s\n";
	static constexpr auto &SESSION_WARNING_WRAPPER = "[WARNING] Session #%d: %s\n";
	static constexpr auto &SESSION_ERROR_WRAPPER = "[ERROR] Session #%d: %s\n";
	static constexpr auto &SESSION_FINE_WRAPPER = "[FINE] Session #%d: %s\n";

protected:

	Logger() = default;

public:

	static void info(const char *message, int session_id = -1);
	static void warning(const char *message, int session_id = -1);
	static void error(const char *message, int session_id = -1);
	static void fine(const char *message, int session_id = -1);
};


#endif //SOCKS_PROXY_LOGGER_H
