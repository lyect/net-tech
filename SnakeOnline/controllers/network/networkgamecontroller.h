#ifndef NETWORK_NETWORKGAMECONTROLLER_H
#define NETWORK_NETWORKGAMECONTROLLER_H

#include <QObject>
#include <QHostAddress>

#include "../gamecontroller.h"
#include "../../channels/messagechannel.h"
#include "../../channels/network/networkmessagechannel.h"
#include "../../proto/protohelp.h"
#include "../../field/field.h"

namespace Network {

class NetworkGameController : public GameController {
	Q_OBJECT
private:

	enum JoinState {
		NOT_JOINED,
		SENT_JOIN,
		JOINED
	};

	// Your own stuff
	int CurrentGame_id;
	QString CurrentGame_name;
	JoinState CurrentGame_join_state;

	// Players stuff
	QSet<int> CurrentGame_players_ids;
	QHash<int, QString> CurrentGame_id2name;
	QHash<int, snakes::PlayerType> CurrentGame_id2type;
	QHash<int, snakes::NodeRole> CurrentGame_id2role;
	QHash<int, int> CurrentGame_id2score;
	QHash<int, snakes::Direction> CurrentGame_id2dir;
	int CurrentGame_master_id;
	int CurrentGame_deputy_id;

	// Field
	int CurrentGame_field_height;
	int CurrentGame_field_width;
	int CurrentGame_field_state_order;
	Field CurrentGame_field;
	QHash<int, snakes::GameState::Snake> CurrentGame_player_snakes;
	QSet<QPair<int , int>> CurrentGame_foods;

	// Game config
	int CurrentGame_static_foods;
	int CurrentGame_state_delay_ms;
	int CurrentGame_player_timeout_ms;

	// Game info
	QString CurrentGame_game_name;


	// Message sending delays
	const int MulticastMessages_DISCOVER_DELAY_MS = 1000;
	const int MulticastMessages_ANNOUNCEMENT_DELAY_MS = 1000;

	const int Constraints_MIN_FIELD_WIDTH = 10;
	const int Constraints_MAX_FIELD_WIDTH = 100;
	const int Constraints_MIN_FIELD_HEIGHT = 10;
	const int Constraints_MAX_FIELD_HEIGHT = 100;
	const int Constraints_MIN_STATIC_FOODS = 0;
	const int Constraints_MAX_STATIC_FOODS = 100;
	const int Constraints_MIN_STATE_DELAY_MS = 100;
	const int Constraints_MAX_STATE_DELAY_MS = 3000;
	// Для подключения игрока на поле должен быть
	//	блок таких размеров:
	const int Constraints_MIN_BLOCK_HEIGHT = 5;
	const int Constraints_MIN_BLOCK_WIDTH = 5;
	// Через такое время игра пропадает из списка доступных
	const int Constraints_GAME_EXPIRATION_TIME_MS = 60000;

	QHostAddress multicast_address;
	quint16 multicast_port;
	MessageChannel *channel;

	QTimer *discover_timer;
	QTimer *announcement_timer;

	QHash<QString, snakes::GameAnnouncement> known_games;
	QHash<QString, QTimer*> known_games_timers;

	void sendDiscoverMessage();
	void sendAnnouncementMessage();

	void setDiscoverTimer();
	void setAnnouncementTimer();

	void setAnnouncedGameTimer(const QString &game_name);

	void move(const snakes::Direction &direction);

	void removePlayer(int player_id);

	QTimer *process_field_timer;
	int process_field_order;
	void startFieldProcessingTimer();

	QHash<int, QTimer*> ping_timers;
	void setPingMessageTimer(int player_id);
	void restartPingMessageTimer(int player_id);

	void chooseDeputy();

	void resetCurrentGame();

private slots:

	void processMessages();

public:
	NetworkGameController(
			QHostAddress multicast_address,
			quint16 multicast_port,
			QObject *parent = nullptr
			);

	~NetworkGameController() override;


public slots:
	void hostGame(
			const int field_width,
			const int field_height,
			const int static_foods,
			const int state_delay_ms,
			const QString game_name,
			const QString player_name,
			const char type_ch
			) override;

	void joinGame(
			const QString game_name,
			const QString player_name,
			const char type_ch,
			const char role_ch
			) override;

	void disconnectGame() override;

	void uMove() override;
	void dMove() override;
	void lMove() override;
	void rMove() override;
};

} // namespace Network

#endif // NETWORK_NETWORKGAMECONTROLLER_H
