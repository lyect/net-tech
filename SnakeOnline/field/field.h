#ifndef FIELD_FIELD_H
#define FIELD_FIELD_H

#include <QVector>
#include <QColor>
#include <QMutex>

#include "../proto/snakes.pb.h"
#include "../proto/protohelp.h"

class Field {
private:
	int height;
	int width;
	int food_count;
	QVector<QVector<int>> data;

public:
	Field(int _height = 0, int _width = 0);
	QSharedPointer<Field> getPointer();
	QVector<QVector<int>> &getData();
	int getFoodCount();
	void resize(int _height, int _width);
	void clear();
	void drawSnake(const snakes::GameState::Snake &snake);
	void drawFood(const snakes::GameState::Coord &coord);
	bool hasEmptyBlock(int h, int w);
	QPair<int, int> getEmptyBlock(int h, int w);
	QPair<int, int> getRandomEmptyCell();
	QPair<snakes::GameState::Coord, snakes::GameState::Coord> spawnSnake(int player_id, int x, int y, int h, int w);
};

#endif // FIELD_FIELD_H
