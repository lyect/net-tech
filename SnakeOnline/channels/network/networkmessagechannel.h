#ifndef NETWORK_RELIABLEMESSAGESOCKET_H
#define NETWORK_RELIABLEMESSAGESOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QQueue>
#include <QDebug>
#include <QTimer>

#include "../../proto/protohelp.h"
#include "../messagechannel.h"
#include "./proto/snakes.pb.h"

#include "../messagebuffer.h"

namespace Network {

class NetworkMessageChannel : public MessageChannel {
	Q_OBJECT

private:
	QUdpSocket *multicast_socket;
	QUdpSocket *unicast_socket;

	const QHostAddress multicast_address;
	const quint16 multicast_port;

	QHash<QPair<QHostAddress, quint16>, int> host2id;
	QHash<int, QPair<QHostAddress, quint16>> id2host;

	long long msg_seq;
	int id;

	QHash<int, MessageBuffer<QNetworkDatagram>> send_buffers;
	QHash<int, MessageBuffer<snakes::GameMessage>> message_buffers;

	QQueue<snakes::GameMessage> pending_messages;

	int delay;
	QHash<int, QMetaObject::Connection> timer_connections;
	QHash<int, QTimer*> id_timers;

	void addTimer(const int &host_id);

private slots:

	void processMulticastMessages();
	void processUnicastMessages();

public:
	NetworkMessageChannel(
			QHostAddress multicast_address,
			quint16 multicast_port,
			QObject *parent = nullptr
			);

	~NetworkMessageChannel() override;

	void setId(const int &_id) override;
	void setDelay(const int &delay_ms) override;
	void reset() override;

	[[nodiscard]] bool joinGame(
			const snakes::GameAnnouncement &announcement,
			const snakes::PlayerType &_player_type,
			const QString &_player_name,
			const snakes::NodeRole &_role
			) override;
	[[nodiscard]] int send(
			snakes::GameMessage &msg,
			const int &receiver_id = -1
			) override;
	[[nodiscard]] bool hasPendingMessages() override;
	snakes::GameMessage receive() override;
};

} // namespace Network

#endif // NETWORK_RELIABLEMESSAGESOCKET_H
