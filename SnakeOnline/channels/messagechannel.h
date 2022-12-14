#ifndef NETWORK_RELIABLEMESSAGECHANNEL_H
#define NETWORK_RELIABLEMESSAGECHANNEL_H

#include <QObject>
#include <QHostAddress>
#include <QtPlugin>

#include "./proto/snakes.pb.h"

namespace Network {

class MessageChannel : public QObject {
	Q_OBJECT
protected:
	MessageChannel(QObject *parent = nullptr);
public:

	virtual ~MessageChannel() = default;

	virtual void setId(const int &_id) = 0;
	virtual void setDelay(const int &delay_ms) = 0;
	virtual void reset() = 0;
	[[nodiscard]] virtual bool joinGame(
			const snakes::GameAnnouncement &announcement,
			const snakes::PlayerType &_player_type,
			const QString &_player_name,
			const snakes::NodeRole &_role
			) = 0;
	[[nodiscard]] virtual int send(
			snakes::GameMessage &msg,
			const int &receiver_id = -1
			) = 0;
	[[nodiscard]] virtual bool hasPendingMessages() = 0;
	virtual snakes::GameMessage receive() = 0;

signals:

	void readyReceive();
};

} // namespace Network

#endif // NETWORK_RELIABLEMESSAGECHANNEL_H
