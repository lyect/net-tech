#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QRunnable>
#include <QSharedPointer>

#include "../field/field.h"
#include "./proto/snakes.pb.h"

class GameController : public QObject {
	Q_OBJECT
protected:
	GameController(QObject *parent = nullptr);

public:

	virtual ~GameController() = default;
public slots:
	virtual void hostGame(
			const int width,
			const int height,
			const int static_foods,
			const int state_delay_ms,
			const QString game_name,
			const QString name,
			const char type_ch
			) = 0;
	virtual void joinGame(
			const QString game_name,
			const QString player_name,
			const char type_ch,
			const char role_ch
			) = 0;
	virtual void disconnectGame() = 0;

	virtual void uMove() = 0;
	virtual void dMove() = 0;
	virtual void lMove() = 0;
	virtual void rMove() = 0;

signals:

	void connectedToGame();

	void error(QString error_msg);

	// Сигналы для вьюхи
	void ratingReady(QVector<QString> rating);
	void configReady(QString game_name, QString field_size, QString static_foods);
	void gotNewGameAnnouncement(
			const QString game_name,
			const QString master_address,
			const QString field_height,
			const QString field_width,
			const QString static_foods
			);
	void removedGameAnnouncement(QString game_name);

	// сигналы для контроллера поля
	// Нет сигнала для поворота игрока, т.к. от игрока может
	//	быть много поворотов. Повороты мы обработаем в контроллере
	//	а итоговый просигналим через drawSnake
	void fieldResized(int height, int width);
	void clearField();
	void drawBlock(int row, int col, QColor block_color);

};

#endif // GAMECONTROLLER_H
