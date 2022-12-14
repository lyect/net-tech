#ifndef PROTOHELP_H
#define PROTOHELP_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QHostAddress>

#include <string>

#include "./proto/snakes.pb.h"

namespace ProtoHelp {

	// #######################
	// #   COORD FUNCTIONS   #
	// #######################

	namespace Coord {
	snakes::GameState::Coord make(int x, int y);
	}

	// #######################
	// #   SNAKE FUNCTIONS   #
	// #######################

	namespace Snake {
	snakes::GameState::Snake make(
			const int &id,
			const QVector<snakes::GameState::Coord> &points,
			const snakes::GameState::Snake::SnakeState &state,
			const snakes::Direction &dir);
	void setState(
			snakes::GameState::Snake &snake,
			const snakes::GameState::Snake::SnakeState &state);
	snakes::GameState::Coord getHead(const snakes::GameState::Snake &snake);
	void addHead(snakes::GameState::Snake &snake, snakes::GameState::Coord h);
	void removeTail(snakes::GameState::Snake &snake);
	QVector<snakes::GameState::Coord> getPoints(const snakes::GameState::Snake &snake);
	int getPlayerId(const snakes::GameState::Snake &snake);
	snakes::Direction getDirection(const snakes::GameState::Snake &snake);
	void setDirection(snakes::GameState::Snake &snake, snakes::Direction dir);
	int getLength(const snakes::GameState::Snake &snake);
	}

	// ############################
	// #   GAME STATE FUNCTIONS   #
	// ############################

	namespace GameState {
	snakes::GameState make(
			const int &state_order,
			const QVector<snakes::GameState::Snake> &snakes,
			const QSet<QPair<int, int>> &foods,
			const snakes::GamePlayers &players
			);
	}

	// ###################################
	// #   GAME ANNOUNCEMENT FUNCTIONS   #
	// ###################################

	namespace GameAnnouncement {
	snakes::GameAnnouncement make(
			const snakes::GamePlayers &players,
			const snakes::GameConfig &config,
			const bool &can_join,
			const QString &game_name
			);
	bool hasMasterAddress(const snakes::GameAnnouncement &announcement);
	bool hasMasterPort(const snakes::GameAnnouncement &announcement);
	QVector<snakes::GamePlayer> getPlayers(const snakes::GameAnnouncement &announcement);
	QString getGameName(const snakes::GameAnnouncement &announcement);
	int getStateDelay(const snakes::GameAnnouncement &announcement);
	QString getMasterAddress(const snakes::GameAnnouncement &announcement);
	bool getCanJoin(const snakes::GameAnnouncement &announcement);
	int getHeight(const snakes::GameAnnouncement &announcement);
	int getWidth(const snakes::GameAnnouncement &announcement);
	int getStaticFoods(const snakes::GameAnnouncement &announcement);
	int getMasterId(const snakes::GameAnnouncement &announcement);
	int getDeputyId(const snakes::GameAnnouncement &announcement);
	bool hasDeputy(const snakes::GameAnnouncement &announcement);
	}

	// #############################
	// #   GAME PLAYER FUNCTIONS   #
	// #############################

	namespace GamePlayer {
	snakes::GamePlayer make(
			const int &id,
			const QString &name,
			const snakes::NodeRole &role,
			const snakes::PlayerType &type,
			const int &score
			);
	int getPlayerId(const snakes::GamePlayer &player);
	snakes::NodeRole getPlayerRole(const snakes::GamePlayer &player);
	QHostAddress getPlayerAddress(const snakes::GamePlayer &player);
	quint16 getPlayerPort(const snakes::GamePlayer &player);
	QString getPlayerName(const snakes::GamePlayer &player);
	snakes::PlayerType getPlayerType(const snakes::GamePlayer &player);
	int getPlayerScore(const snakes::GamePlayer &player);
	}

	// ##############################
	// #   GAME PLAYERS FUNCTIONS   #
	// ##############################

	namespace GamePlayers {
	snakes::GamePlayers make(
			const QVector<snakes::GamePlayer> &players
			);
	}

	// #############################
	// #   GAME CONFIG FUNCTIONS   #
	// #############################

	namespace GameConfig {
	snakes::GameConfig make(
			const int &width,
			const int &height,
			const int &static_foods,
			const int &state_delay
			);
	}

	// ################################
	// #   COMMON MESSAGE FUNCTIONS   #
	// ################################

	namespace Message {

	QByteArray serialize(const snakes::GameMessage &msg);
	snakes::GameMessage deserialize(const QByteArray &msg_bytes);
	snakes::GameMessage::TypeCase getMsgType(const snakes::GameMessage &msg);
	long long getMsgSeq(const snakes::GameMessage &msg);
	void setMsgSeq(snakes::GameMessage &msg, long long msg_seq);
	void setSenderId(snakes::GameMessage &msg, const int &sender_id);
	void setReceiverId(snakes::GameMessage &msg, const int &receiver_id);
	QString typeToString(const snakes::GameMessage::TypeCase &msg_type);
	int getSenderId(const snakes::GameMessage &msg);
	int getReceiverId(const snakes::GameMessage &msg);
	}

	// ##############################
	// #   PING MESSAGE FUNCTIONS   #
	// ##############################

	namespace PingMsg {
	snakes::GameMessage make();
	}

	// ###############################
	// #   STEER MESSAGE FUNCTIONS   #
	// ###############################

	namespace SteerMsg {
	snakes::GameMessage make(
			const snakes::Direction &direction
		);
	snakes::Direction getDirection(const snakes::GameMessage &msg);
	}

	// #############################
	// #   ACK MESSAGE FUNCTIONS   #
	// #############################

	namespace AckMsg {
	snakes::GameMessage make(
			const int &sender_id,
			const int &receiver_id
		);
	}

	// ###############################
	// #   STATE MESSAGE FUNCTIONS   #
	// ###############################

	namespace StateMsg {
	snakes::GameMessage make(
			const snakes::GameState &state
		);
	int getStateOrder(const snakes::GameMessage &msg);
	QVector<snakes::GameState::Snake> getSnakes(const snakes::GameMessage &msg);
	QVector<snakes::GameState::Coord> getFoods(const snakes::GameMessage &msg);
	QVector<snakes::GamePlayer> getPlayers(const snakes::GameMessage &msg);
	}
	// ######################################
	// #   ANNOUNCEMENT MESSAGE FUNCTIONS   #
	// ######################################

	namespace AnnouncementMsg {
	snakes::GameMessage make(
			const QVector<snakes::GameAnnouncement> &games
		);
	void addMasterIp(snakes::GameMessage &msg, const QHostAddress &address);
	void addMasterPort(snakes::GameMessage &msg, const quint16 &port);
	QVector<snakes::GameAnnouncement> getAnnouncedGames(const snakes::GameMessage &msg);
	}

	// ##############################
	// #   JOIN MESSAGE FUNCTIONS   #
	// ##############################

	namespace JoinMsg {
	snakes::GameMessage make(
			const QString &player_name,
			const QString &game_name,
			const snakes::NodeRole &node_role,
			const snakes::PlayerType &type
		);
	QString getGameName(const snakes::GameMessage &msg);
	snakes::NodeRole getRequestedRole(const snakes::GameMessage &msg);
	QString getPlayerName(const snakes::GameMessage &msg);
	snakes::PlayerType getPlayerType(const snakes::GameMessage &msg);
	}

	// ###############################
	// #   ERROR MESSAGE FUNCTIONS   #
	// ###############################

	namespace ErrorMsg {
	snakes::GameMessage make(
			const QString &error_message
		);
	QString getErrorMessage(const snakes::GameMessage &msg);
	}

	// #####################################
	// #   ROLE CHANGE MESSAGE FUNCTIONS   #
	// #####################################

	namespace RoleChangeMsg {

	enum AddMode {
		ADD_NO_ROLES,
		ADD_SENDER_ROLE,
		ADD_RECEIVER_ROLE,
		ADD_ALL_ROLES
	};

	// add_mode == 0 ---> no roles will be added
	// add_mode == 1 ---> sender_role will be added
	// add_mode == 2 ---> receiver_role will be added
	// add_mode == 3 ---> all roles will be added
	snakes::GameMessage make(
			const snakes::NodeRole &sender_role,
			const snakes::NodeRole &receiver_role,
			const AddMode &add_mode,
			const int &sender_id,
			const int &receiver_id
		);
	bool hasSenderRole(const snakes::GameMessage &msg);
	bool hasReceiverRole(const snakes::GameMessage &msg);
	snakes::NodeRole getSenderRole(const snakes::GameMessage &msg);
	snakes::NodeRole getReceiverRole(const snakes::GameMessage &msg);

	}

	// ##################################
	// #   DISCOVER MESSAGE FUNCTIONS   #
	// ##################################

	namespace DiscoverMsg {

	snakes::GameMessage make();
	}



}

#endif // PROTOHELP_H
