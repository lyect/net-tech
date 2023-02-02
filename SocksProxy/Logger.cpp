#include "Logger.h"

void Logger::info(const char *message, int session_id) {
	if (session_id == -1) {
		printf(INFO_WRAPPER, message);
	}
	else {
		printf(SESSION_INFO_WRAPPER, session_id, message);
	}
}
void Logger::warning(const char *message, int session_id) {
	if (session_id == -1) {
		printf(WARNING_WRAPPER, message);
	}
	else {
		printf(SESSION_WARNING_WRAPPER, session_id, message);
	}
}
void Logger::error(const char *message, int session_id) {
	if (session_id == -1) {
		printf(ERROR_WRAPPER, message);
	}
	else {
		printf(SESSION_ERROR_WRAPPER, session_id, message);
	}
}
void Logger::fine(const char *message, int session_id) {
	if (session_id == -1) {
		printf(FINE_WRAPPER, message);
	}
	else {
		printf(SESSION_FINE_WRAPPER, session_id, message);
	}
}