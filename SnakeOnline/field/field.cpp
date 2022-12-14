#include "field.h"

Field::Field(int _height, int _width) {

	height = _height;
	width = _width;
	food_count = 0;
	data.resize(height);
	for (int i = 0; i < height; ++i) {
		data[i].resize(width);
	}
}

QSharedPointer<Field> Field::getPointer() {
	return QSharedPointer<Field>(this);
}

QVector<QVector<int>> &Field::getData() {
	return data;
}

int Field::getFoodCount() {
	return food_count;
}

void Field::resize(int _height, int _width) {
	clear();
	Field(_height, _width);
}

void Field::clear() {
	food_count = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (data[i][j] > 1) {
				data[i][j] = 0;
			}
		}
	}
}

void Field::drawSnake(const snakes::GameState::Snake &snake) {
	auto points = ProtoHelp::Snake::getPoints(snake);
	int player_id2 = ProtoHelp::Snake::getPlayerId(snake) * 2 + 2;

	int prev_x = points[0].x();
	int prev_y = points[0].y();
	data[prev_y][prev_x] = player_id2 + 1;

	for (int k = 1; k < points.size(); ++k) {
		int x = points[k].x();
		int y = points[k].y();
		data[y][x] = player_id2;
	}
}

void Field::drawFood(const snakes::GameState::Coord &coord) {
	food_count += 1;
	data[coord.y()][coord.x()] = 1;
}

bool Field::hasEmptyBlock(int h, int w) {
	for (int i = 0; i < height - h; ++i) {
		for (int j = 0; j < width - w; ++j) {
			bool flag = true;
			for (int a = 0; a < h; ++a) {
				for (int b = 0; b < w; ++b) {
					if (data[a][b] != 0) {
						flag = false;
						break;
					}
				}
				if (!flag) {
					break;
				}
			}
			if (flag) {
				return flag;
			}
		}
	}
	return false;
}

QPair<int, int> Field::getEmptyBlock(int h, int w) {
	for (int i = 0; i < height - h; ++i) {
		for (int j = 0; j < width - w; ++j) {
			bool flag = true;
			for (int a = 0; a < h; ++a) {
				for (int b = 0; b < w; ++b) {
					if (data[a][b] != 0) {
						flag = false;
						break;
					}
				}
				if (!flag) {
					break;
				}
			}
			if (flag) {
				return {i, j};
			}
		}
	}
	return {-1, -1};
}

QPair<int, int> Field::getRandomEmptyCell() {
	QVector<QPair<int, int>> empty_cells;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (data[i][j] == 0) {
				empty_cells.push_back({i, j});
			}
		}
	}

	if (empty_cells.size() == 0) {
		return {-1, -1};
	}

	return empty_cells[rand() % empty_cells.size()];
}

QPair<snakes::GameState::Coord, snakes::GameState::Coord> Field::spawnSnake(int player_id, int x, int y, int h, int w) {
	int spawn_x = (x + w - 1) / 2;
	int spawn_y = (y + h - 1) / 2;
	// head
	data[spawn_y][spawn_x] =  player_id * 2 + 2;
	// body
	data[spawn_y + 1][spawn_x] = player_id * 2 + 3;

	return {
		ProtoHelp::Coord::make(spawn_x, spawn_y),
		ProtoHelp::Coord::make(spawn_x, spawn_y + 1)
	};
}


