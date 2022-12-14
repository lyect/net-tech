#include "protohelp.h"

namespace ProtoHelp {
	// ###################################
	// #   GAME ANNOUNCEMENT FUNCTIONS   #
	// ###################################

	namespace GameAnnouncement {
	snakes::GameAnnouncement make(
			const snakes::GamePlayers &players,
			const snakes::GameConfig &config,
			const bool &can_join,
			const QString &game_name
			) {
		snakes::GameAnnouncement game_announcement;
		game_announcement.set_allocated_players(new snakes::GamePlayers(players));
		game_announcement.set_allocated_config(new snakes::GameConfig(config));
		game_announcement.set_can_join(can_join);
		game_announcement.set_allocated_game_name(new std::string(game_name.toStdString()));
		return game_announcement;
	}
	bool hasMasterAddress(const snakes::GameAnnouncement &announcement) {
		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			if (player.role() == snakes::NodeRole::MASTER) {
				return player.has_ip_address();
			}
		}
		return false;
	}
	bool hasMasterPort(const snakes::GameAnnouncement &announcement) {
		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			if (player.role() == snakes::NodeRole::MASTER) {
				return player.has_port();
			}
		}
		return false;
	}
	QVector<snakes::GamePlayer> getPlayers(const snakes::GameAnnouncement &announcement) {
		QVector<snakes::GamePlayer> result;

		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			result.push_back(player);
		}

		return result;
	}

	QString getGameName(const snakes::GameAnnouncement &announcement) {
		return QString::fromStdString(announcement.game_name());
	}

	int getStateDelay(const snakes::GameAnnouncement &announcement) {
		snakes::GameConfig config = announcement.config();
		return config.state_delay_ms();
	}
	QString getMasterAddress(const snakes::GameAnnouncement &announcement) {
		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			if (player.role() == snakes::NodeRole::MASTER) {
				return QString::fromStdString(player.ip_address());
			}
		}
		return "";
	}
	bool getCanJoin(const snakes::GameAnnouncement &announcement) {
		return announcement.can_join();
	}
	int getHeight(const snakes::GameAnnouncement &announcement) {
		return announcement.config().height();
	}
	int getWidth(const snakes::GameAnnouncement &announcement) {
		return announcement.config().width();
	}
	int getStaticFoods(const snakes::GameAnnouncement &announcement) {
		return announcement.config().food_static();
	}
	int getMasterId(const snakes::GameAnnouncement &announcement) {
		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			if (player.role() == snakes::NodeRole::MASTER) {
				return player.id();
			}
		}
		return 0;
	}
	int getDeputyId(const snakes::GameAnnouncement &announcement) {
		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			if (player.role() == snakes::NodeRole::DEPUTY) {
				return player.id();
			}
		}
		return 0;
	}
	bool hasDeputy(const snakes::GameAnnouncement &announcement) {
		snakes::GamePlayers game_players = announcement.players();
		auto players = game_players.players();
		for (auto &player: players) {
			if (player.role() == snakes::NodeRole::DEPUTY) {
				return true;
			}
		}
		return false;
	}
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
			) {
		snakes::GamePlayer game_player;
		game_player.set_id(id);
		game_player.set_allocated_name(new std::string(name.toStdString()));
		game_player.set_role(role);
		game_player.set_type(type);
		game_player.set_score(score);
		return game_player;
	}
	int getPlayerId(const snakes::GamePlayer &player) {
		return player.id();
	}
	snakes::NodeRole getPlayerRole(const snakes::GamePlayer &player) {
		return player.role();
	}

	QHostAddress getPlayerAddress(const snakes::GamePlayer &player) {
		return QHostAddress(QString::fromStdString(player.ip_address()));
	}

	quint16 getPlayerPort(const snakes::GamePlayer &player) {
		return static_cast<quint16>(player.port());
	}
	QString getPlayerName(const snakes::GamePlayer &player) {
		return QString::fromStdString(player.name());
	}
	snakes::PlayerType getPlayerType(const snakes::GamePlayer &player) {
		return player.type();
	}
	int getPlayerScore(const snakes::GamePlayer &player) {
		return player.score();
	}

	}

	// ##############################
	// #   GAME PLAYERS FUNCTIONS   #
	// ##############################

	namespace GamePlayers {
	snakes::GamePlayers make(
			const QVector<snakes::GamePlayer> &players
			) {
		snakes::GamePlayers game_players;
		auto mut_players = game_players.mutable_players();

		for (auto &player : players) {
			mut_players->AddAllocated(new snakes::GamePlayer(player));
		}

		return game_players;
	}
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
			) {
		snakes::GameConfig config;
		config.set_width(width);
		config.set_height(height);
		config.set_food_static(static_foods);
		config.set_state_delay_ms(state_delay);

		return config;
	}
	}

	// ################################
	// #   COMMON MESSAGE FUNCTIONS   #
	// ################################

	namespace Message {

	QByteArray serialize(const snakes::GameMessage &msg) {
		std::string serialized_msg;
		if (!msg.SerializeToString(&serialized_msg)) {
			qWarning("Can not serialize message");
			return QByteArray();
		}

		return QByteArray::fromStdString(serialized_msg);
	}

	snakes::GameMessage deserialize(const QByteArray &msg_bytes) {

		std::string serialized_msg = msg_bytes.toStdString();
		snakes::GameMessage msg;

		if (!msg.ParseFromString(serialized_msg)) {
			qWarning("Can not deserialize message");
			return snakes::GameMessage();
		}

		return msg;
	}

	snakes::GameMessage::TypeCase getMsgType(const snakes::GameMessage &msg) {
		return msg.Type_case();
	}

	long long getMsgSeq(const snakes::GameMessage &msg) {
		return msg.msg_seq();
	}

	void setMsgSeq(snakes::GameMessage &msg, long long msg_seq) {
		msg.set_msg_seq(msg_seq);
	}

	void setSenderId(snakes::GameMessage &msg, const int &sender_id) {
		msg.set_sender_id(sender_id);
	}

	void setReceiverId(snakes::GameMessage &msg, const int &receiver_id) {
		msg.set_receiver_id(receiver_id);
	}

	QString typeToString(const snakes::GameMessage::TypeCase &msg_type) {
		switch (msg_type) {
		case snakes::GameMessage::TypeCase::kAck:
			return QString("AckMessage");
		case snakes::GameMessage::TypeCase::kAnnouncement:
			return QString("AnnouncementMessage");
		case snakes::GameMessage::TypeCase::kDiscover:
			return QString("DiscoverMessage");
		case snakes::GameMessage::TypeCase::kError:
			return QString("ErrorMessage");
		case snakes::GameMessage::TypeCase::kJoin:
			return QString("JoinMessage");
		case snakes::GameMessage::TypeCase::kPing:
			return QString("PingMessage");
		case snakes::GameMessage::TypeCase::kRoleChange:
			return QString("RoleChangeMessage");
		case snakes::GameMessage::TypeCase::kState:
			return QString("StateMessage");
		case snakes::GameMessage::TypeCase::kSteer:
			return QString("SteerMessage");
		case snakes::GameMessage::TypeCase::TYPE_NOT_SET:
			return QString("TYPE_NOT_SET");
		default: {
			return QString("UNKNOWN");
		}
		}
	}
	int getSenderId(const snakes::GameMessage &msg) {
		return msg.sender_id();
	}
	int getReceiverId(const snakes::GameMessage &msg) {
		return msg.receiver_id();
	}
	}

	// ##############################
	// #   PING MESSAGE FUNCTIONS   #
	// ##############################

	namespace PingMsg {
	snakes::GameMessage make() {
		snakes::GameMessage msg;

		snakes::GameMessage::PingMsg *ping_msg = new snakes::GameMessage::PingMsg();

		msg.set_allocated_ping(ping_msg);

		return msg;
	}
	}

	// ###############################
	// #   STEER MESSAGE FUNCTIONS   #
	// ###############################

	namespace SteerMsg {
	snakes::GameMessage make(
			const snakes::Direction &direction
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::SteerMsg *steer_msg = new snakes::GameMessage::SteerMsg();
		steer_msg->set_direction(direction);

		msg.set_allocated_steer(steer_msg);

		return msg;
	}
	snakes::Direction getDirection(const snakes::GameMessage &msg) {
		return msg.steer().direction();
	}
	}

	// #############################
	// #   ACK MESSAGE FUNCTIONS   #
	// #############################

	namespace AckMsg {
	snakes::GameMessage make(
			const int &sender_id,
			const int &receiver_id
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::AckMsg *ack_msg = new snakes::GameMessage::AckMsg();

		msg.set_allocated_ack(ack_msg);

		msg.set_sender_id(sender_id);
		msg.set_receiver_id(receiver_id);

		return msg;
	}
	}

	// ###############################
	// #   STATE MESSAGE FUNCTIONS   #
	// ###############################

	namespace StateMsg {
	snakes::GameMessage make(
			const snakes::GameState &state
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::StateMsg *state_msg = new snakes::GameMessage::StateMsg();
		state_msg->set_allocated_state(new snakes::GameState(state));

		msg.set_allocated_state(state_msg);

		return msg;
	}
	int getStateOrder(const snakes::GameMessage &msg) {
		return msg.state().state().state_order();
	}
	QVector<snakes::GameState::Snake> getSnakes(const snakes::GameMessage &msg) {
		QVector<snakes::GameState::Snake> res;
		auto ap = msg.state().state();
		for (auto &snake : ap.snakes()) {
			res.push_back(snake);
		}
		return res;
	}
	QVector<snakes::GameState::Coord> getFoods(const snakes::GameMessage &msg) {
		QVector<snakes::GameState::Coord> res;
		auto ap = msg.state().state();
		for (auto &food : ap.foods()) {
			res.push_back(food);
		}
		return res;
	}
	QVector<snakes::GamePlayer> getPlayers(const snakes::GameMessage &msg) {
		QVector<snakes::GamePlayer> res;
		auto ap = msg.state().state().players();
		for (auto &player : ap.players()) {
			res.push_back(player);
		}
		return res;
	}
	}

	// ######################################
	// #   ANNOUNCEMENT MESSAGE FUNCTIONS   #
	// ######################################

	namespace AnnouncementMsg {
	snakes::GameMessage make(
			const QVector<snakes::GameAnnouncement> &games
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::AnnouncementMsg *announcement_msg = new snakes::GameMessage::AnnouncementMsg();
		auto msg_games = announcement_msg->mutable_games();
		for (int i = 0; i < games.size(); ++i) {
			msg_games->AddAllocated(new snakes::GameAnnouncement(games[i]));
		}

		msg.set_allocated_announcement(announcement_msg);

		return msg;
	}
	void addMasterIp(snakes::GameMessage &msg, const QHostAddress &address) {
		snakes::GameMessage::AnnouncementMsg *announcement_msg = msg.mutable_announcement();

		auto games = *announcement_msg->mutable_games();

		for (auto &game : games) {
			snakes::GamePlayers *game_players = game.mutable_players();
			auto players = *game_players->mutable_players();
			for (auto &player: players) {
				if (player.role() == snakes::NodeRole::MASTER) {
					player.set_allocated_ip_address(new std::string(address.toString().toStdString()));
				}
			}
		}
	}
	void addMasterPort(snakes::GameMessage &msg, const quint16 &port) {
		snakes::GameMessage::AnnouncementMsg *announcement_msg = msg.mutable_announcement();

		auto games = *announcement_msg->mutable_games();

		for (auto &game : games) {
			snakes::GamePlayers *game_players = game.mutable_players();
			auto players = *game_players->mutable_players();
			for (auto &player: players) {
				if (player.role() == snakes::NodeRole::MASTER) {
					player.set_port(static_cast<int>(port));
				}
			}
		}
	}
	QVector<snakes::GameAnnouncement> getAnnouncedGames(const snakes::GameMessage &msg) {
		QVector<snakes::GameAnnouncement> res;
		auto games = msg.announcement().games();
		for (auto &game : games) {
			res.push_back(game);
		}
		return res;
	}
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
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::JoinMsg *join_msg = new snakes::GameMessage::JoinMsg();
		join_msg->set_allocated_player_name(new std::string(player_name.toStdString()));
		join_msg->set_allocated_game_name(new std::string(game_name.toStdString()));
		join_msg->set_requested_role(node_role);
		join_msg->set_player_type(type);

		msg.set_allocated_join(join_msg);

		return msg;
	}
	QString getGameName(const snakes::GameMessage &msg) {
		return QString::fromStdString(msg.join().game_name());
	}
	snakes::NodeRole getRequestedRole(const snakes::GameMessage &msg) {
		return msg.join().requested_role();
	}
	QString getPlayerName(const snakes::GameMessage &msg) {
		return QString::fromStdString(msg.join().player_name());
	}
	snakes::PlayerType getPlayerType(const snakes::GameMessage &msg) {
		return msg.join().player_type();
	}

	}

	// ###############################
	// #   ERROR MESSAGE FUNCTIONS   #
	// ###############################

	namespace ErrorMsg {
	snakes::GameMessage make(
			const QString &error_message
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::ErrorMsg *error_msg = new snakes::GameMessage::ErrorMsg();
		error_msg->set_allocated_error_message(new std::string(error_message.toStdString()));

		msg.set_allocated_error(error_msg);

		return msg;
	}
	QString getErrorMessage(const snakes::GameMessage &msg) {
		return QString::fromStdString(msg.error().error_message());
	}
	}

	// #####################################
	// #   ROLE CHANGE MESSAGE FUNCTIONS   #
	// #####################################

	namespace RoleChangeMsg {

	snakes::GameMessage make(
			const snakes::NodeRole &sender_role,
			const snakes::NodeRole &receiver_role,
			const AddMode &add_mode,
			const int &sender_id,
			const int &receiver_id
		) {
		snakes::GameMessage msg;

		snakes::GameMessage::RoleChangeMsg *role_change_msg = new snakes::GameMessage::RoleChangeMsg();
		if (add_mode == AddMode::ADD_SENDER_ROLE) {
			role_change_msg->set_sender_role(sender_role);
		}
		if (add_mode == AddMode::ADD_RECEIVER_ROLE) {
			role_change_msg->set_receiver_role(receiver_role);
		}
		if (add_mode == AddMode::ADD_ALL_ROLES) {
			role_change_msg->set_sender_role(sender_role);
			role_change_msg->set_receiver_role(receiver_role);
		}

		msg.set_allocated_role_change(role_change_msg);

		msg.set_sender_id(sender_id);
		msg.set_receiver_id(receiver_id);

		return msg;
	}
	bool hasSenderRole(const snakes::GameMessage &msg) {
		return msg.role_change().has_sender_role();
	}
	bool hasReceiverRole(const snakes::GameMessage &msg) {
		return msg.role_change().has_receiver_role();
	}
	snakes::NodeRole getSenderRole(const snakes::GameMessage &msg) {
		return msg.role_change().sender_role();
	}
	snakes::NodeRole getReceiverRole(const snakes::GameMessage &msg) {
		return msg.role_change().receiver_role();
	}
	}

	// ##################################
	// #   DISCOVER MESSAGE FUNCTIONS   #
	// ##################################

	namespace DiscoverMsg {

	snakes::GameMessage make() {
		snakes::GameMessage msg;

		snakes::GameMessage::DiscoverMsg *discover_msg = new snakes::GameMessage::DiscoverMsg();

		msg.set_allocated_discover(discover_msg);

		return msg;
	}
	}

	// #######################
	// #   SNAKE FUNCTIONS   #
	// #######################

	namespace Snake {
	snakes::GameState::Snake make(
			const int &id,
			const QVector<snakes::GameState::Coord> &points,
			const snakes::GameState::Snake::SnakeState &state,
			const snakes::Direction &dir) {

		snakes::GameState::Snake snake;

		snake.set_player_id(id);
		snake.set_state(state);
		snake.set_head_direction(dir);

		auto ps = snake.mutable_points();
		for (int i = 0; i < points.size(); ++i) {
			ps->AddAllocated(new snakes::GameState::Coord(points[i]));
		}

		return snake;
	}
	void setState(
			snakes::GameState::Snake &snake,
			const snakes::GameState::Snake::SnakeState &state) {
		snake.set_state(state);
	}
	snakes::GameState::Coord getHead(const snakes::GameState::Snake &snake) {
		auto points = ProtoHelp::Snake::getPoints(snake);
		return points[0];
	}
	void addHead(snakes::GameState::Snake &snake, snakes::GameState::Coord h){
		auto points = snake.mutable_points();
		points->AddAllocated(new snakes::GameState::Coord(h));

		for (int i = points->size() - 1; i > 0; --i) {
		  points->SwapElements(i, i - 1);
		}
	}
	void removeTail(snakes::GameState::Snake &snake) {
		auto points = snake.mutable_points();
		points->RemoveLast();
	}
	QVector<snakes::GameState::Coord> getPoints(const snakes::GameState::Snake &snake) {
		auto points = snake.points();
		QVector<snakes::GameState::Coord> res;
		for (auto &point : points) {
			res.push_back(point);
		}
		return res;
	}

	int getPlayerId(const snakes::GameState::Snake &snake) {
		return snake.player_id();
	}
	snakes::Direction getDirection(const snakes::GameState::Snake &snake) {
		return snake.head_direction();
	}
	void setDirection(snakes::GameState::Snake &snake, snakes::Direction dir) {
		snake.set_head_direction(dir);
	}
	int getLength(const snakes::GameState::Snake &snake) {
		return snake.points_size();
	}
	}

	// #######################
	// #   COORD FUNCTIONS   #
	// #######################

	namespace Coord {
	snakes::GameState::Coord make(int x, int y) {
		snakes::GameState::Coord coord;
		coord.set_x(x);
		coord.set_y(y);
		return coord;
	}
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
			) {
		snakes::GameState gs;
		gs.set_state_order(state_order);

		auto sn = gs.mutable_snakes();
		for (auto &snake : snakes) {
			sn->AddAllocated(new snakes::GameState::Snake(snake));
		}

		auto fs = gs.mutable_foods();
		for (auto &food : foods) {
			auto coord = new snakes::GameState::Coord;
			coord->set_x(food.first);
			coord->set_y(food.second);
			fs->AddAllocated(coord);
		}

		gs.set_allocated_players(new snakes::GamePlayers(players));

		return gs;
	}
	}

}

