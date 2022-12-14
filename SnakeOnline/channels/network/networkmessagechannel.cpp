#include "networkmessagechannel.h"

namespace Network {

// ###############
// #   PRIVATE   #
// ###############

void NetworkMessageChannel::addTimer(const int &host_id) {
	// По идее такого быть не должно
	// Но на всякий случай лучше перепровериться, чтобы себе ничего не попортить
	if (host_id == id) {
		qWarning("Tried to set timer on your own id.");
		return;
	}

	auto &timer = *id_timers.insert(host_id, new QTimer(this));

	QObject::connect(
		timer, &QTimer::timeout,
		this, [&, timer]() -> void {

			// remove buffers
			send_buffers.remove(host_id);
			message_buffers.remove(host_id);

			// remove id <-> ip mapping
			auto &address_port = id2host[host_id];
			host2id.remove(address_port);
			id2host.remove(host_id);

			delete timer;
			id_timers.remove(host_id);
	});

	timer->start(delay);
}

void NetworkMessageChannel::processMulticastMessages() {
	while (multicast_socket->hasPendingDatagrams()) {
		QNetworkDatagram datagram = multicast_socket->receiveDatagram();

		QByteArray data = datagram.data();
		QHostAddress sender_address = datagram.senderAddress();
		quint16 sender_port = datagram.senderPort();

		if (host2id.contains({sender_address, sender_port})) {
			int &host_id = host2id[{sender_address, sender_port}];

			id_timers[host_id]->start(delay);
		}

		snakes::GameMessage msg = ProtoHelp::Message::deserialize(data);
		const snakes::GameMessage::TypeCase &msg_type = ProtoHelp::Message::getMsgType(msg);

		switch (msg_type) {
		case snakes::GameMessage::TypeCase::kDiscover: {
			// Ответ на дискавер будет (если вообще будет) от логики выше
			// Логика выше увидит дискавер и сделает сенд(анонс)
			pending_messages.enqueue(msg);
			emit readyReceive();
			break;
		}
		case snakes::GameMessage::TypeCase::kAnnouncement: {

			// Добавляем айпи мастера в сообщение и передаём его наверх
			// Сверху оно встанет в табличку с доступными играми. Если юзер выберет эту игру,
			//	то при отправке Join мы возьмём всех этих пользователей и их айпи и
			//	добавим в список известных хостов

			ProtoHelp::AnnouncementMsg::addMasterIp(msg, sender_address);
			ProtoHelp::AnnouncementMsg::addMasterPort(msg, sender_port);

			pending_messages.enqueue(msg);
			emit readyReceive();
			break;
		}
		default: { // other types of messages
			qWarning() << "Message with type '" + \
						  ProtoHelp::Message::typeToString(msg_type) + \
						  "' was sent via multicast.";
			break;
		}
		}
	}
}

void NetworkMessageChannel::processUnicastMessages() {
	while (unicast_socket->hasPendingDatagrams()) {
		QNetworkDatagram datagram = unicast_socket->receiveDatagram();

		QByteArray data = datagram.data();
		QHostAddress sender_address = datagram.senderAddress();
		quint16 sender_port = datagram.senderPort();

		if (host2id.contains({sender_address, sender_port})) {
			int &host_id = host2id[{sender_address, sender_port}];
			id_timers[host_id]->start(delay);
		}

		snakes::GameMessage msg = ProtoHelp::Message::deserialize(data);
		const snakes::GameMessage::TypeCase &msg_type = ProtoHelp::Message::getMsgType(msg);

		qInfo() << "SOCKET received msg with type " << ProtoHelp::Message::typeToString(msg_type);

		switch (msg_type) {
		case snakes::GameMessage::TypeCase::kPing:
		case snakes::GameMessage::TypeCase::kSteer:
		case snakes::GameMessage::TypeCase::kError:
		case snakes::GameMessage::TypeCase::kState:
		// Похоже, знание о топологии не нужно в сокете
		case snakes::GameMessage::TypeCase::kRoleChange: {

			// Если пришло какое-то из этих сообщений, то надо его просто акнуть
			// И если благодаря ему в буфере принятых сообщений появился непрерывный блок,
			//	надо достать этот блок и передать его наверх

			// Если мы ещё не выставили свой id, то скипаем сообщение
			//	так как не сможем на него ответить аком
			if (id == -1000) {
				break;
			}

			if (!host2id.contains({sender_address, sender_port})) {
				qInfo("Message from unknown host.");
				break;
			}

			int &sender_id = host2id[{sender_address, sender_port}];

			const long long &msg_seq = ProtoHelp::Message::getMsgSeq(msg);
			const long long &message_buffer_base = message_buffers[sender_id].getBase();

			if (msg_seq <= message_buffer_base) {
				snakes::GameMessage ack_msg = ProtoHelp::AckMsg::make(
							id,			// sender_id
							sender_id	// receiver_id
							);
				ProtoHelp::Message::setMsgSeq(ack_msg, message_buffer_base);
				if (!send(ack_msg, sender_id)) {
					qWarning("Can not answer on message");
				}
				break;
			}

			if (msg_seq > message_buffer_base + message_buffers[sender_id].getCapacity()) {
				qWarning("Got message with too big msg_seq. Ignoring...");
				break;
			}

			// buffer_base < msg_seq <= buffer_base + WINDOW_SIZE
			ProtoHelp::Message::setSenderId(msg, sender_id);
			message_buffers[sender_id].insertByPosition(msg, msg_seq);

			if (message_buffers[sender_id].hasContinuousBlock()) {
				auto [cont_block_start, cont_block_end] = message_buffers[sender_id].getContinuousBlock();

				int cur_msg_idx = cont_block_start;
				while (cur_msg_idx <= cont_block_end) {
					// Сначала была идея хранить отдельный буфер под состояния.
					// И передавать их в порядке прихода. То есть
					// 1 ---(state2)---> me : state_buffer[2] = state2
					// 2 ---(state3)---> me : state_buffer[3] = state3
					// 1 ---(state1)---> me : state_buffer[1] = state1
					// Типа появился непрерывный блок из состояний, поэтому
					//	только сейчас передаём его наверх.
					// Но от этого решения отказался, потому что этого не требуется в ТЗ
					//	и ещё потому что это породит некоторые проблемы - например,
					//	если мастер отвалится, успев сказать о смене состояния только
					//	своему депутю, то мы не сможем получить состояние, которое было
					//	отправлено депутю. Потому что депуть не знает, что оно нам нужно,
					//	поэтому он будет кидаться в нас следующим состоянием, которое,
					//	по-хорошему, надо бы обработать, а мы будем стоять на прошлом, зависнув.
					// Теперь будет так:
					// 1 ---(state1)---> me : обработано в логике выше
					// 2 ---(state3)---> me : обработано в логике выше
					// 1 ---(state2)---> me : логика выше скипнет это состояние
					// При такой реализации, смерть мастера и последующая потеря
					//	пакетов будет выглядеть как телепортация змейки
					snakes::GameMessage cur_msg = \
							message_buffers[sender_id].getByPosition(cur_msg_idx);
					pending_messages.enqueue(cur_msg);
					++cur_msg_idx;
				}

				// Отправить групповое подтверждение
				snakes::GameMessage ack_msg = ProtoHelp::AckMsg::make(
							id,			// sender_id
							sender_id	// receiver_id
							);
				ProtoHelp::Message::setMsgSeq(ack_msg, message_buffers[sender_id].getBase());
				if (send(ack_msg, sender_id) != 0) {
					qWarning("Can not answer on message");
				}

				emit readyReceive();
			}
			break;
		}
		// Если пришёл ак, то надо сделать групповое подтверждение
		//	датаграм из буфера отправки
		case snakes::GameMessage::TypeCase::kAck: {
			if (!host2id.contains({sender_address, sender_port})) {
				qInfo("Message from unknown host.");
				break;
			}

			int &sender_id = host2id[{sender_address, sender_port}];

			long long msg_seq = ProtoHelp::Message::getMsgSeq(msg);
			const long long &buffer_base = send_buffers[sender_id].getBase();

			if (msg_seq <= buffer_base) {
				break;
			}

			// msg_seq > buffer_base
			send_buffers[sender_id].setBase(msg_seq);

			// Отправим ак наверх. Это нужно для того, чтобы поменять свой id
			// Если бы меняли это тут, то тогда сокет знал бы о том, подключен
			//	он или нет. Но это нарушение заложенной в него абстракции
			message_buffers[sender_id].insertByPosition(msg, msg_seq);
			if (message_buffers[sender_id].hasContinuousBlock()) {
				auto [cont_block_start, cont_block_end] = message_buffers[sender_id].getContinuousBlock();

				int cur_msg_idx = cont_block_start;
				while (cur_msg_idx <= cont_block_end) {
					snakes::GameMessage cur_msg = \
							message_buffers[sender_id].getByPosition(cur_msg_idx);
					pending_messages.enqueue(cur_msg);
					++cur_msg_idx;
				}

				// Аки не подтверждаются, поэтому ничего не отправляем обратно
				emit readyReceive();
			}
			break;
		}
		// Если пришёл джоин, то надо добавить хост в список известных хостов
		// Далее по известному сценарию - групповое подтверждение + передача наверх
		case snakes::GameMessage::TypeCase::kJoin: {
			// Если мы ещё не выставили свой id, то скипаем сообщение
			//	так как не сможем на него ответить аком
			if (id == -1000) {
				break;
			}

			auto sender_id_iter = host2id.find({sender_address, sender_port});
			if (sender_id_iter == host2id.end()) {

				int max_id = -1;
				for (auto &host_id : id2host.keys()) {
					max_id = std::max(max_id, host_id);
				}

				sender_id_iter = host2id.insert({sender_address, sender_port}, max_id + 1);
				id2host.insert(max_id + 1, {sender_address, sender_port});

				addTimer(max_id + 1);
			}

			int &sender_id = *sender_id_iter;

			const long long &msg_seq = ProtoHelp::Message::getMsgSeq(msg);
			const long long &message_buffer_base = message_buffers[sender_id].getBase();

			if (msg_seq <= message_buffer_base) {
				snakes::GameMessage ack_msg = ProtoHelp::AckMsg::make(
							id,			// sender_id
							sender_id	// receiver_id
							);
				ProtoHelp::Message::setMsgSeq(ack_msg, message_buffer_base);
				if (send(ack_msg, sender_id) != 0) {
					qWarning("Can not answer on message");
				}
				break;
			}

			if (msg_seq > message_buffer_base + message_buffers[sender_id].getCapacity()) {
				qWarning("Got message with too big msg_seq. Ignoring...");
				break;
			}

			// buffer_base < msg_seq <= buffer_base + WINDOW_SIZE
			ProtoHelp::Message::setSenderId(msg, sender_id);
			message_buffers[sender_id].insertByPosition(msg, msg_seq);

			if (message_buffers[sender_id].hasContinuousBlock()) {
				auto [cont_block_start, cont_block_end] = message_buffers[sender_id].getContinuousBlock();

				int cur_msg_idx = cont_block_start;
				while (cur_msg_idx <= cont_block_end) {

					snakes::GameMessage cur_msg = \
							message_buffers[sender_id].getByPosition(cur_msg_idx);
					pending_messages.enqueue(cur_msg);
					++cur_msg_idx;
				}

				// Отправить групповое подтверждение
				snakes::GameMessage ack_msg = ProtoHelp::AckMsg::make(
							id,			// sender_id
							sender_id	// receiver_id
							);
				ProtoHelp::Message::setMsgSeq(ack_msg, message_buffers[sender_id].getBase());
				if (send(ack_msg, sender_id) != 0) {
					qWarning("Can not answer on message");
				}

				emit readyReceive();
			}
			break;
		}
		default:
			qWarning() << "Message with type '" + \
						  ProtoHelp::Message::typeToString(msg_type) + \
						  "' was sent via unicast.";
			break;
		}
	}
}

// ##############
// #   PUBLIC   #
// ##############

NetworkMessageChannel::NetworkMessageChannel(
			QHostAddress multicast_address,
			quint16 multicast_port,
			QObject *parent)
		: MessageChannel{parent},
		  multicast_address{multicast_address},
		  multicast_port{multicast_port}
		{

	// Create multicast socket
	multicast_socket = new QUdpSocket(this);
	if (!multicast_socket->bind(QHostAddress(QHostAddress::AnyIPv4), multicast_port + 1)) {
		qWarning("Can not bind multicast socket");
	}
	if (!multicast_socket->joinMulticastGroup(multicast_address)) {
		qWarning("Can not join to the multicast group");
	}
	// If multicast message arrives, it will be processed
	//	in the "processMulticastMessages" function
	QObject::connect(multicast_socket, &QUdpSocket::readyRead,
					 this, &NetworkMessageChannel::processMulticastMessages);

	// Create unicast socket
	unicast_socket = new QUdpSocket(this);
	// If unicast message arrives, it will be processed
	//	in the "processMessages" function
	QObject::connect(unicast_socket, &QUdpSocket::readyRead,
					 this, &NetworkMessageChannel::processUnicastMessages);
	delay = 1000;
	msg_seq = 0;
	id = -1000;
}

NetworkMessageChannel::~NetworkMessageChannel() {
	QObject::disconnect(multicast_socket, &QUdpSocket::readyRead,
						this, &NetworkMessageChannel::processMulticastMessages);
	QObject::disconnect(unicast_socket, &QUdpSocket::readyRead,
						this, &NetworkMessageChannel::processUnicastMessages);
	multicast_socket->leaveMulticastGroup(multicast_address);

	for (auto &timer : id_timers) {
		delete timer;
	}

	qInfo("Deleted ReliableMessageSocket");
}

void NetworkMessageChannel::setId(const int &_id) {
	id = _id;
	id2host[id] = {QHostAddress(), 0};
}

void NetworkMessageChannel::setDelay(const int &delay_ms) {
	delay = delay_ms;
}

void NetworkMessageChannel::reset() {
	host2id.clear();
	id2host.clear();

	msg_seq = 0;
	id = -1000;

	send_buffers.clear();
	message_buffers.clear();

	pending_messages.clear();

	for (auto &timer : id_timers) {
		delete timer;
	}

	id_timers.clear();
}

bool NetworkMessageChannel::joinGame(
		const snakes::GameAnnouncement &announcement,
		const snakes::PlayerType &_player_type,
		const QString &_player_name,
		const snakes::NodeRole &_role
		) {
	//	3) Вообще говоря, ситуация 1) возможна, но не сразу, а только
	//		через время, которое нужно каналу для того, чтобы понять, что игрок
	//		отвалился. Канал кладёт сообщение в буффер и переотправляет
	//		его снова, если не получит Ack за определённое время.
	//	   Во время очередной переотправки канал может попытаться
	//		переотправить сообщение удалённому по таймауту (долго не
	//		получал от него сообщений) мастеру. Тогда канал
	if (!ProtoHelp::GameAnnouncement::hasMasterAddress(announcement)) {
		qWarning("Game announcement does not contain address of the master!");
		return false;
	}

	if (!ProtoHelp::GameAnnouncement::hasMasterPort(announcement)) {
		qWarning("Game announcement does not contain port of the master!");
		return false;
	}

	delay = ProtoHelp::GameAnnouncement::getStateDelay(announcement) * 0.8;

	QVector<snakes::GamePlayer> announced_players = \
			ProtoHelp::GameAnnouncement::getPlayers(announcement);

	int master_id = -1;

	for (auto &player : announced_players) {
		int player_id = ProtoHelp::GamePlayer::getPlayerId(player);

		if (ProtoHelp::GamePlayer::getPlayerRole(player) == snakes::NodeRole::MASTER) {
			master_id = player_id;
		}

		QHostAddress player_address = ProtoHelp::GamePlayer::getPlayerAddress(player);
		quint16 player_port = ProtoHelp::GamePlayer::getPlayerPort(player);

		id2host.insert(player_id, {player_address, player_port});
		host2id.insert({player_address, player_port}, player_id);

		addTimer(player_id);
	}

	snakes::GameMessage join_msg = ProtoHelp::JoinMsg::make(
				_player_name,
				ProtoHelp::GameAnnouncement::getGameName(announcement),
				_role,
				_player_type
				);

	return send(join_msg, master_id);
}

int NetworkMessageChannel::send(
		snakes::GameMessage &msg,
		const int &receiver_id
		) {

	if (receiver_id == id) {
		qInfo() << "Send to myself";
		pending_messages.enqueue(msg);
		emit readyReceive();
		return 0;
	}

	const snakes::GameMessage::TypeCase &msg_type = ProtoHelp::Message::getMsgType(msg);

	ProtoHelp::Message::setMsgSeq(msg, msg_seq);
	++msg_seq;

	// kDiscover, kAnnouncement
	if (msg_type == snakes::GameMessage::kDiscover || msg_type == snakes::GameMessage::kAnnouncement) {

		QByteArray msg_bytes = ProtoHelp::Message::serialize(msg);
		QNetworkDatagram datagram(
					msg_bytes,
					multicast_address,
					multicast_port
					);

		multicast_socket->writeDatagram(datagram);
		return 0;
	}

	// if receiver_id is -1, send messages to all known hosts
	if (receiver_id == -1) {
		for (auto &certain_receiver_id : id2host.keys()) {
			int send_result = send(msg, certain_receiver_id);
			if (send_result != 0) {
				return send_result;
			}
		}
		return 0;
	}

	if (!id2host.contains(receiver_id)) {
		qWarning("Trying to send message to unknown host");
		return -1;
	}

	auto &receiver_address_port = id2host[receiver_id];

	// kAck
	if (msg_type == snakes::GameMessage::kAck) {

		if (!id2host.contains(receiver_id)) {
			qWarning("Trying to ack unknown host");
			return -1;
		}

		QByteArray msg_bytes = ProtoHelp::Message::serialize(msg);
		QNetworkDatagram datagram(
					msg_bytes,
					receiver_address_port.first, // address
					receiver_address_port.second // port
					);
		unicast_socket->writeDatagram(datagram);
		return 0;
	}

	// other types
	if (send_buffers[receiver_id].isFull()) {
		qWarning("Sending buffer is full.");
		return 1;
	}

	QByteArray msg_bytes = ProtoHelp::Message::serialize(msg);
	QNetworkDatagram datagram(
				msg_bytes,
				receiver_address_port.first, // address
				receiver_address_port.second // port
				);

	send_buffers[receiver_id].insertByPosition(datagram, msg_seq - 1);
	unicast_socket->writeDatagram(datagram);

	return 0;
}

bool NetworkMessageChannel::hasPendingMessages() {
	return pending_messages.size() != 0;
}

snakes::GameMessage NetworkMessageChannel::receive() {
	Q_ASSERT(pending_messages.size() > 0);

	return pending_messages.dequeue();
}

} // namespace Network
