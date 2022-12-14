#include "networkgamecontroller.h"

namespace Network {

// ================================================================================================
// ######   PRIVATE METHODS   #####################################################################
// ================================================================================================


void NetworkGameController::sendDiscoverMessage() {
	auto msg = ProtoHelp::DiscoverMsg::make();
	if (channel->send(msg, -1) != 0) {
		qWarning("Can not send discover message");
	}
	discover_timer->start(MulticastMessages_ANNOUNCEMENT_DELAY_MS);
}

// ================================================================================================

void NetworkGameController::sendAnnouncementMessage() {

	QVector<snakes::GamePlayer> players_vector;
	for (auto &id : CurrentGame_players_ids) {
		players_vector.push_back(ProtoHelp::GamePlayer::make(
									 id,
									 CurrentGame_id2name[id],
									 CurrentGame_id2role[id],
									 CurrentGame_id2type[id],
									 CurrentGame_id2score[id]
									 )
								 );
	}

	snakes::GamePlayers players = ProtoHelp::GamePlayers::make(players_vector);

	snakes::GameConfig config = ProtoHelp::GameConfig::make(
				CurrentGame_field_width,
				CurrentGame_field_height,
				CurrentGame_static_foods,
				CurrentGame_state_delay_ms
				);

	snakes::GameAnnouncement announcement = ProtoHelp::GameAnnouncement::make(
				players,
				config,
				CurrentGame_field.hasEmptyBlock(
					Constraints_MIN_BLOCK_HEIGHT,
					Constraints_MIN_BLOCK_WIDTH
					),
				CurrentGame_game_name
				);

	// Один экземпляр приложения - одна игра, так что
	//	просто запихнём в массив анонсов один наш анонс
	auto msg = ProtoHelp::AnnouncementMsg::make(
				{announcement}
				);
	if (channel->send(msg, -1) != 0) {
		qWarning("Can not send announcement message");
	}

	//delete a;
	//delete b;
	announcement_timer->start(MulticastMessages_ANNOUNCEMENT_DELAY_MS);
}


// ================================================================================================

void NetworkGameController::setDiscoverTimer() {
	QObject::connect(discover_timer, &QTimer::timeout,
					 this, &NetworkGameController::sendDiscoverMessage);

	discover_timer->start(MulticastMessages_DISCOVER_DELAY_MS);
}

// ================================================================================================

void NetworkGameController::setAnnouncementTimer() {
	QObject::connect(announcement_timer, &QTimer::timeout,
					 this, &NetworkGameController::sendAnnouncementMessage);

	announcement_timer->start(MulticastMessages_ANNOUNCEMENT_DELAY_MS);
}

// ================================================================================================

void NetworkGameController::setAnnouncedGameTimer(const QString &game_name) {

	if (!known_games_timers.contains(game_name)) {
		known_games_timers[game_name] = new QTimer();
	}
	QObject::connect(known_games_timers[game_name], &QTimer::timeout,
					 [&]() -> void {
		delete known_games_timers[game_name];
		known_games.remove(game_name);
		known_games_timers.remove(game_name);
		emit removedGameAnnouncement(game_name);
	});
	known_games_timers[game_name]->start(Constraints_GAME_EXPIRATION_TIME_MS);
}

// ================================================================================================

void NetworkGameController::move(const snakes::Direction &direction) {
	if (CurrentGame_join_state == JoinState::JOINED &&
		CurrentGame_id2type[CurrentGame_id] != snakes::PlayerType::ROBOT &&
		CurrentGame_id2role[CurrentGame_id] != snakes::NodeRole::VIEWER
	) {
		snakes::GameMessage msg = ProtoHelp::SteerMsg::make(direction);
		if (channel->send(msg, CurrentGame_master_id) == -1) {
			removePlayer(CurrentGame_master_id);
		}
	}
}

// ================================================================================================

void NetworkGameController::removePlayer(int player_id) {

	CurrentGame_players_ids.remove(player_id);
	CurrentGame_id2name.remove(player_id);
	CurrentGame_id2type.remove(player_id);
	CurrentGame_id2role.remove(player_id);
	CurrentGame_id2score.remove(player_id);
	CurrentGame_id2dir.remove(player_id);

	if (ping_timers.contains(player_id)) {
		auto t = ping_timers[player_id];
		delete t;
		ping_timers.remove(player_id);
	}

	if (player_id == CurrentGame_master_id) {
		// Если мы и есть мастер, то значит, мы сами себе доставили сообщение
		// Поскольку канал реализован так, что если отправляем себе, то просто
		//	возвращаем сообщение, то мы себе НЕ МОГЛИ НЕ ДОСТАВИТЬ
		// А эта функция вызывается только если не смогли себе доставить.
		// Значит, не делаем проверку, что мы - мастер

		CurrentGame_master_id = CurrentGame_deputy_id;
		CurrentGame_deputy_id = -1;
		CurrentGame_id2role[CurrentGame_master_id] = snakes::NodeRole::MASTER;
		if (CurrentGame_id == CurrentGame_master_id) {
			chooseDeputy();
		}
	}
	else if (player_id == CurrentGame_deputy_id) {
		// Депуть не может прислать себе что-то
		// Поэтому никаких дополнительных проверок делать не нужно
		CurrentGame_deputy_id = -1;
		if (CurrentGame_id == CurrentGame_master_id) {
			chooseDeputy();
		}
	}
	else {
		// Удалили и всё
	}
}

// ================================================================================================

void NetworkGameController::startFieldProcessingTimer() {
	process_field_timer = new QTimer(this);
	QObject::connect(process_field_timer, &QTimer::timeout,
					 [&]() -> void {
		qInfo() << "field timer!";
		auto &data = CurrentGame_field.getData();
		int foods = CurrentGame_field.getFoodCount();

		// Проверить столкновения змеек
		//	если при попытке сдвинуть голову в направлении,
		//	встречен не 0 и не 1, то убиваем змейку
		//		проходимся по всем её клеткам и с шансом 50% ставим еду
		// Если столкновения нет
		//	Если встали на еду
		//		добавляем новую клетку головы
		//	Если не встали на еду
		//		добавляем клетку головы
		//		удвляем последнюю клетку

		for (auto &id : CurrentGame_player_snakes.keys()) {

			auto &snake = CurrentGame_player_snakes[id];

			auto dir = ProtoHelp::Snake::getDirection(snake);

			if (!CurrentGame_players_ids.contains(id)) {
				ProtoHelp::Snake::setState(snake, snakes::GameState::Snake::ZOMBIE);
			}
			else {
				dir = CurrentGame_id2dir[id];
			}

			ProtoHelp::Snake::setDirection(snake, dir);

			auto head = ProtoHelp::Snake::getHead(snake);

			snakes::GameState::Coord next_head;
			switch (dir) {
			case snakes::Direction::UP: {
				next_head = ProtoHelp::Coord::make(
							head.x(),
							(head.y() - 1 + CurrentGame_field_height) % CurrentGame_field_height);

				break;
			}
			case snakes::Direction::DOWN: {
				next_head = ProtoHelp::Coord::make(
							head.x(),
							(head.y() + 1) % CurrentGame_field_height);
				break;
			}
			case snakes::Direction::LEFT: {
				next_head = ProtoHelp::Coord::make(
							(head.x() - 1 + CurrentGame_field_width) % CurrentGame_field_width,
							head.y());
				break;
			}
			case snakes::Direction::RIGHT: {
				next_head = ProtoHelp::Coord::make(
							(head.x() + 1) % CurrentGame_field_width,
							head.y());
				break;
			}
			}

			// столкнулись
			if (data[next_head.y()][next_head.x()] >= 2) {
				for (auto &point : ProtoHelp::Snake::getPoints(snake)) {
					int p = std::rand() % 100;
					if (p < 50) {
						CurrentGame_foods.insert({point.x(), point.y()});
						foods += 1;
					}
				}
				CurrentGame_player_snakes.remove(id);
				auto msg = ProtoHelp::RoleChangeMsg::make(
							snakes::NodeRole::VIEWER,
							snakes::NodeRole::VIEWER,
							ProtoHelp::RoleChangeMsg::ADD_RECEIVER_ROLE,
							CurrentGame_id,
							id
							);
				if (channel->send(msg, id) == -1) {
					removePlayer(id);
				}
			}
			else {
				ProtoHelp::Snake::addHead(snake, next_head);
				// не наехали на еду
				if (data[next_head.y()][next_head.x()] == 0) {
					ProtoHelp::Snake::removeTail(snake);
				}
				// наехали на еду
				else {
					CurrentGame_foods.remove({next_head.x(), next_head.y()});
					foods -= 1;
				}
			}
		}

		// Добавляем еду, если надо и если возможно
		for (int i = foods; i <= CurrentGame_static_foods; ++i) {
			auto [y, x] = CurrentGame_field.getRandomEmptyCell();

			if (x == -1 && y == -1) {
				break;
			}

			CurrentGame_foods.insert({x, y});

		}

		// делаем сообщение

		process_field_order += 1;
		auto snakes = CurrentGame_player_snakes.values();

		QVector<snakes::GamePlayer> players_vector;
		for (auto &id : CurrentGame_players_ids) {
			if (CurrentGame_player_snakes.contains(id)) {
				CurrentGame_id2score[id] = ProtoHelp::Snake::getLength(CurrentGame_player_snakes[id]);
			}
			players_vector.push_back(ProtoHelp::GamePlayer::make(
										 id,
										 CurrentGame_id2name[id],
										 CurrentGame_id2role[id],
										 CurrentGame_id2type[id],
										 CurrentGame_id2score[id]
										 )
									 );
		}

		snakes::GamePlayers players = ProtoHelp::GamePlayers::make(players_vector);

		auto game_state = ProtoHelp::GameState::make(
					process_field_order,
					snakes,
					CurrentGame_foods,
					players
					);

		auto msg = ProtoHelp::StateMsg::make(game_state);

		if (channel->send(msg, -1)) {}
		process_field_timer->start(CurrentGame_state_delay_ms);
	});
	process_field_timer->start(CurrentGame_state_delay_ms);
}

// ================================================================================================

void NetworkGameController::setPingMessageTimer(int player_id) {
	if (player_id == -1) {
		for (auto &id : CurrentGame_players_ids) {
			if (ping_timers.contains(id)) {
				ping_timers[id]->start(CurrentGame_state_delay_ms * 0.8);
				continue;
			}
			ping_timers[id] = new QTimer();
			QObject::connect(ping_timers[id], &QTimer::timeout,
							 [&]() -> void {
				auto msg = ProtoHelp::PingMsg::make();
				if (channel->send(msg, id) == -1) {
					removePlayer(id);
				}
			});
		}
	}
	else {
		if (ping_timers.contains(player_id)) {
			ping_timers[player_id]->start(CurrentGame_state_delay_ms * 0.8);
			return;
		}
		ping_timers[player_id] = new QTimer();
		QObject::connect(ping_timers[player_id], &QTimer::timeout,
						 [&]() -> void {
			auto msg = ProtoHelp::PingMsg::make();
			if (channel->send(msg, player_id) == -1) {
				removePlayer(player_id);
			}
		});
	}
}

// ================================================================================================

void NetworkGameController::restartPingMessageTimer(int player_id) {
	if (ping_timers.contains(player_id)) {
		ping_timers[player_id]->start(CurrentGame_state_delay_ms * 0.8);
	}
}

// ================================================================================================

void NetworkGameController::chooseDeputy() {
	for (auto &id : CurrentGame_players_ids) {
		if (CurrentGame_id2role[id] == snakes::NodeRole::NORMAL) {
			CurrentGame_id2role[id] = snakes::NodeRole::DEPUTY;
		}
		CurrentGame_deputy_id = id;

		if (CurrentGame_id2role[CurrentGame_id] == snakes::NodeRole::MASTER) {
			auto msg = ProtoHelp::RoleChangeMsg::make(
						snakes::NodeRole::VIEWER,
						snakes::NodeRole::DEPUTY,
						ProtoHelp::RoleChangeMsg::ADD_RECEIVER_ROLE,
						CurrentGame_id,
						id);
			if (channel->send(msg, id) == -1) {
				removePlayer(id);
			}
		}
	}
}

//

void NetworkGameController::resetCurrentGame() {
	// Your own stuff
	CurrentGame_id = -1;
	CurrentGame_name = 0;
	CurrentGame_join_state = JoinState::NOT_JOINED;

	// Players stuff
	CurrentGame_players_ids = {};
	CurrentGame_id2name = {};
	CurrentGame_id2type = {};
	CurrentGame_id2role = {};
	CurrentGame_id2score = {};
	CurrentGame_id2dir = {};
	CurrentGame_master_id = -1;
	CurrentGame_deputy_id = -1;

	// Field
	CurrentGame_field_height = 0;
	CurrentGame_field_width = 0;
	CurrentGame_field_state_order = 0;
	//CurrentGame_field;
	CurrentGame_player_snakes = {};
	CurrentGame_foods = {};

	// Game config
	CurrentGame_static_foods = 0;
	CurrentGame_state_delay_ms = 0;
	CurrentGame_player_timeout_ms = 0;

	// Game info
	CurrentGame_game_name = 0;

	discover_timer->stop();
	announcement_timer->stop();

	channel->reset();

	known_games = {};
	known_games_timers = {};

	process_field_timer->stop();
	process_field_order = 0;

	ping_timers = {};
}

// ================================================================================================

void NetworkGameController::processMessages() {

	int msg_count = 0;
	while (channel->hasPendingMessages()) {
		msg_count++;
		if (msg_count == 10) {
			break;
		}
		snakes::GameMessage msg = channel->receive();

		const snakes::GameMessage::TypeCase msg_type = ProtoHelp::Message::getMsgType(msg);

		if (msg_type != snakes::GameMessage::TypeCase::kDiscover && msg_type != snakes::GameMessage::TypeCase::kAnnouncement) {
			qInfo() << "got" << ProtoHelp::Message::typeToString(msg_type);
		}

		switch (msg_type) {
		// Если пришёл дискавер, то ответим на него
		//	анонсом игры, но только если мы - мастер
		case snakes::GameMessage::TypeCase::kDiscover: {
			if (CurrentGame_join_state == JoinState::JOINED && CurrentGame_id2role[CurrentGame_id] == snakes::NodeRole::MASTER) {
				sendAnnouncementMessage();
			}
			break;
		}
		// Если пришёл анонс игры, то
		//		1) Сохраним анонс и заведём на него таймер на минуту
		//		   Если игра с таким же именем не придёт в течение
		//			этого времени, то считаем, что игра кончилась и удалим
		//			её из списка доступных игр.
		//		2) Преобразуем игру в формат для вьюхи и выпустим сигнал
		case snakes::GameMessage::TypeCase::kAnnouncement: {
			QVector<snakes::GameAnnouncement> announced_games = ProtoHelp::AnnouncementMsg::getAnnouncedGames(msg);
			for (auto &announced_game : announced_games) {

				QString game_name = ProtoHelp::GameAnnouncement::getGameName(announced_game);

				if (known_games.contains(game_name)) {
					continue;
				}

				/*
				if (game_name == CurrentGame_game_name) {
					continue;
				}
				*/

				//  Проверяем, что к игре можно подключиться
				if (!ProtoHelp::GameAnnouncement::getCanJoin(announced_game)) {
					continue;
				}

				QString master_address = ProtoHelp::GameAnnouncement::getMasterAddress(announced_game);
				QString field_height = QString::number(ProtoHelp::GameAnnouncement::getHeight(announced_game));
				QString field_width = QString::number(ProtoHelp::GameAnnouncement::getWidth(announced_game));
				QString static_foods = QString::number(ProtoHelp::GameAnnouncement::getStaticFoods(announced_game));

				// Cохраняем игру
				known_games[game_name] = announced_game;
				// Заводим таймер
				setAnnouncedGameTimer(game_name);

				// Сигналим вьюхе
				emit gotNewGameAnnouncement(
							game_name,
							master_address,
							field_height,
							field_width,
							static_foods
							);
			}
			break;
		}
		// Если пришёл пинг, то просто его пропускаем. Потому что канал сам
		//	обновил то, что игрок живой. Смерть игрока (если пинг дошёл до нас
		//	поздно) мы заметим лишь тогда, когда попытаемся отправить ему сообщение
		case snakes::GameMessage::TypeCase::kPing: {
			break;
		}
		// Если пришёл поворот, то его следует обрабатывать только в том случае,
		//	если мы - мастер.
		// Может быть такое, что мы депуть и ни с того ни с сего начнём принимать
		//	сообщения о повороте. Но тогда, по описанной логике,
		//	мы будем игнорировать их до тех пор, пока сами не поймём, что
		//	что мы - мастер. Конечно, если мы успеем сделать это до того, как
		//	другие игроки посчитают нас мёртвым.
		case snakes::GameMessage::TypeCase::kSteer: {
			if (CurrentGame_join_state == JoinState::JOINED && CurrentGame_id2role[CurrentGame_id] == snakes::NodeRole::MASTER) {
				int sender_id = ProtoHelp::Message::getSenderId(msg);
				if (CurrentGame_players_ids.contains(sender_id)) {
					snakes::Direction steer_dir = ProtoHelp::SteerMsg::getDirection(msg);
					// Сохраним поворот игрока, перезаписав старый, если такой вообще был.
					// Итоговый поворот обработается, когда мы (т.е. мастер) начнём
					//	пересчёт поля
					if (CurrentGame_player_snakes.contains(sender_id)) {
						auto dir = ProtoHelp::Snake::getDirection(CurrentGame_player_snakes[sender_id]);
						if ((dir == snakes::Direction::UP && steer_dir == snakes::Direction::DOWN) ||
							(dir == snakes::Direction::DOWN && steer_dir == snakes::Direction::UP) ||
							(dir == snakes::Direction::LEFT && steer_dir == snakes::Direction::RIGHT) ||
							(dir == snakes::Direction::RIGHT && steer_dir == snakes::Direction::LEFT)
						) {
							break;
						}
						else {
							CurrentGame_id2dir[sender_id] = steer_dir;
						}
					}

				}
			}
			break;
		}
		case snakes::GameMessage::TypeCase::kError: {
			// Т.к. сообщения об ошибке рассылает только мастер,
			//	то обрабатывать будем его только если мы не мастер
			//	и сообщение принято от мастера

			// Убрал
			/*if (CurrentGame_join_state == JoinState::SENT_JOIN ||
				(CurrentGame_join_state == JoinState::JOINED &&
				 CurrentGame_id2role[CurrentGame_id] != snakes::NodeRole::MASTER
				)
			) {*/
			// Т.к. мастер по факту разделён на две части: с одной
			//	стороны он обычный клиент, а с другой он пересчётчик поля.
			// Поэтому лучше принять сообщения об ошибке. В таком случае
			//	получится, что мы отправим сообщение самому себе. Грех будет
			//	не показать это
			int sender_id = ProtoHelp::Message::getSenderId(msg);
			if (sender_id == CurrentGame_master_id) {
				QString error_msg = ProtoHelp::ErrorMsg::getErrorMessage(msg);
				// Сигналим вьюхе об ошибке
				emit error(error_msg);
			}
			//}
			break;
		}
		// Если пришло сообщение об изменении поля, то надо его обработать
		//	только в том случае, если мы подключены к игре.
		// Сначала проверим, что пришло новое поле. После сотрём всё старое поле,
		//	почистим контейнеры с данными игроков и заполним их новыми значениями.
		// И в завершении всего просигналим вьюхе, чтобы та перерисовала поле
		case snakes::GameMessage::TypeCase::kState: {
			// Так как мастер сам себе режиссёр, то ему обрабатывать
			//	сообщение об изменении поля совершенно не нужно
			// А нам, обычным крестьянам, нужно его обработать, но только в
			//	том случае, если оно было принято от мастера
			// Убрал
			//	 && CurrentGame_id2role[CurrentGame_id] != snakes::NodeRole::MASTER
			// Потому что мастер принимает от себя сообщения
			if (CurrentGame_join_state == JoinState::JOINED) {
				int sender_id = ProtoHelp::Message::getSenderId(msg);
				// Если сообщение пришло не от мастера, то скипнем его
				if (sender_id != CurrentGame_master_id) {
					break;
				}

				int state_order = ProtoHelp::StateMsg::getStateOrder(msg);
				// Пришло старое поле
				if (state_order <= CurrentGame_field_state_order) {
					break;
				}

				CurrentGame_field_state_order = state_order;

				QVector<snakes::GameState::Snake> snakes = ProtoHelp::StateMsg::getSnakes(msg);
				QVector<snakes::GameState::Coord> foods = ProtoHelp::StateMsg::getFoods(msg);
				QVector<snakes::GamePlayer> players = ProtoHelp::StateMsg::getPlayers(msg);

				// Почистим контейнеры
				CurrentGame_players_ids.clear();
				CurrentGame_id2name.clear();
				CurrentGame_id2type.clear();
				CurrentGame_id2role.clear();
				CurrentGame_id2score.clear();
				CurrentGame_id2dir.clear();
				CurrentGame_player_snakes.clear();
				CurrentGame_foods.clear();

				QVector<QPair<int, QString>> rating_base;
				// Вставим в контейнеры новые значения
				for (auto &player : players) {
					int player_id = ProtoHelp::GamePlayer::getPlayerId(player);
					CurrentGame_players_ids.insert(player_id);
					CurrentGame_id2name.insert(player_id, ProtoHelp::GamePlayer::getPlayerName(player));
					CurrentGame_id2type.insert(player_id, ProtoHelp::GamePlayer::getPlayerType(player));
					CurrentGame_id2role.insert(player_id, ProtoHelp::GamePlayer::getPlayerRole(player));
					CurrentGame_id2score.insert(player_id, ProtoHelp::GamePlayer::getPlayerScore(player));


					rating_base.push_back({CurrentGame_id2score[player_id] - 2, CurrentGame_id2name[player_id]});

					// Вставка значения поворота происходит ниже, при отрисовке змеек

					if (CurrentGame_id2role[player_id] == snakes::NodeRole::MASTER) {
						CurrentGame_master_id = player_id;
					}
					if (CurrentGame_id2role[player_id] == snakes::NodeRole::DEPUTY) {
						CurrentGame_deputy_id = player_id;
					}
				}

				std::sort(rating_base.begin(), rating_base.end());

				QVector<QString> rating;

				for (auto &x : rating_base) {
					rating.push_back(x.second + ": " + QString::number(x.first));
				}

				emit ratingReady(rating);

				// Почистим поле, отрисуем змеек и еду
				CurrentGame_field.clear();
				for (auto &snake : snakes) {
					CurrentGame_player_snakes.insert(
								ProtoHelp::Snake::getPlayerId(snake),
								snake
								);
					CurrentGame_id2dir.insert(
								ProtoHelp::Snake::getPlayerId(snake),
								ProtoHelp::Snake::getDirection(snake)
								);
					CurrentGame_field.drawSnake(snake);
				}
				for (auto &food : foods) {
					CurrentGame_field.drawFood(food);
					CurrentGame_foods.insert({food.x(), food.y()});
				}

				// Просигналим вьюхе, что поле обновилось и надо его перерисовать
				auto data = CurrentGame_field.getData();
				for (int i = 0; i < CurrentGame_field_height; ++i) {
					for (int j = 0; j < CurrentGame_field_width; ++j) {

						if (data[i][j] == 0) {
							emit drawBlock(i, j, Qt::green);
						}
						else if (data[i][j] == 1) {
							emit drawBlock(i, j, Qt::red);
						}
						else if (data[i][j] % 2 == 1) {
							emit drawBlock(i, j, Qt::black);
						}
						else {
							emit drawBlock(i, j, Qt::blue);
						}
					}
				}

				emit ratingReady(rating);
			}
			break;
		}
		// ЕБАТЬ, СУКА, ЧТО ТАК СЛОЖНО-ТО
		case snakes::GameMessage::TypeCase::kRoleChange: {

			// Если мы не подключены, то зачем менять топологию?
			if (CurrentGame_join_state != JOINED) {
				break;
			}

			int receiver_id = ProtoHelp::Message::getReceiverId(msg);
			// Сообщение пришло не нам
			if (receiver_id != CurrentGame_id) {
				break;
			}

			int sender_id = ProtoHelp::Message::getSenderId(msg);

			if (ProtoHelp::RoleChangeMsg::hasSenderRole(msg)) {
				snakes::NodeRole sender_role = ProtoHelp::RoleChangeMsg::getSenderRole(msg);

				switch (sender_role) {
				case snakes::NodeRole::MASTER: {
					break;
				}
				case snakes::NodeRole::VIEWER: {
					break;
				}
				default:
					break;
				}
			}

			if (ProtoHelp::RoleChangeMsg::hasReceiverRole(msg)) {
				snakes::NodeRole receiver_role = ProtoHelp::RoleChangeMsg::getReceiverRole(msg);

				switch (receiver_role) {
				case snakes::NodeRole::MASTER: {
					break;
				}
				case snakes::NodeRole::DEPUTY: {
					break;
				}
				case snakes::NodeRole::VIEWER: {

				}
				default:
					break;
				}
			}
			break;
		}
		// Ак нужен лишь для того, чтобы выставить свой id в сокете
		// Такая ситуация может произойти, если мы только
		//	отправили join, но ещё не получили свой id.
		// После того, как мы поменяем своё состояние на "подключён",
		//	то сможем принимать состояния поля и оно начнёт отображаться во вьюхе
		case snakes::GameMessage::TypeCase::kAck: {

			if (CurrentGame_join_state == JoinState::SENT_JOIN) {
				CurrentGame_id = ProtoHelp::Message::getReceiverId(msg);
				CurrentGame_join_state = JoinState::JOINED;
				channel->setId(CurrentGame_id);

				emit connectedToGame();
			}
			break;
		}
		// Если пришёл join, то его обрабатывать нужно только в том случае, если
		//	мы - мастер. Иначе просто скипнем его. Логика работы (почему можно скипать),
		//	если мы - депуть, описана около случая со SteerMsg.
		case snakes::GameMessage::TypeCase::kJoin: {
			if (CurrentGame_join_state == JoinState::JOINED && CurrentGame_id2role[CurrentGame_id] == snakes::NodeRole::MASTER) {

				// У сообщения есть sender_id, так как его ему выдал сокет
				int sender_id = ProtoHelp::Message::getSenderId(msg);

				QString game_name = ProtoHelp::JoinMsg::getGameName(msg);
				// Если пытаются присоединится к не нашей игре
				if (CurrentGame_game_name != game_name) {
					snakes::GameMessage error_msg = ProtoHelp::ErrorMsg::make("Wrong GAME_NAME parameter");
					int status = 0;
					if ((status = channel->send(error_msg, sender_id)) != 0) {
						qWarning("Can not send error message");
						if (status == -1) {
							removePlayer(sender_id);
						}
					}
					restartPingMessageTimer(sender_id);
					break;
				}

				snakes::NodeRole req_role = ProtoHelp::JoinMsg::getRequestedRole(msg);
				// Если пытаются зайти как мастер или депуть
				if (req_role == snakes::NodeRole::MASTER || req_role == snakes::NodeRole::DEPUTY) {
					snakes::GameMessage error_msg = ProtoHelp::ErrorMsg::make("Wrong REQUESTED_ROLE parameter");
					int status = 0;
					if ((status = channel->send(error_msg, sender_id)) != 0) {
						qWarning("Can not send error message");
						if (status == -1) {
							removePlayer(sender_id);
						}
					}
					restartPingMessageTimer(sender_id);
					break;
				}
				auto [block_y, block_x] = CurrentGame_field.getEmptyBlock(Constraints_MIN_BLOCK_HEIGHT, Constraints_MIN_BLOCK_WIDTH);
				// Если нет свободного места, отправляем ошибку
				if (block_x == -1 && block_y == -1) {
					snakes::GameMessage error_msg = ProtoHelp::ErrorMsg::make("No space");
					int status = 0;
					if ((status = channel->send(error_msg, sender_id)) != 0) {
						qWarning("Can not send error message");
						if (status == -1) {
							removePlayer(sender_id);
						}
					}
					restartPingMessageTimer(sender_id);
					break;
				}


				CurrentGame_players_ids.insert(sender_id);
				CurrentGame_id2name.insert(sender_id, ProtoHelp::JoinMsg::getPlayerName(msg));
				CurrentGame_id2type.insert(sender_id, ProtoHelp::JoinMsg::getPlayerType(msg));
				CurrentGame_id2role.insert(sender_id, req_role);
				CurrentGame_id2score.insert(sender_id, 0);

				if (req_role == snakes::NodeRole::NORMAL) {
					auto [head, tail] = CurrentGame_field.spawnSnake(sender_id, block_x, block_y, Constraints_MIN_BLOCK_HEIGHT, Constraints_MIN_BLOCK_WIDTH);

					CurrentGame_player_snakes[sender_id] = ProtoHelp::Snake::make(
								sender_id,
								{head, tail},
								snakes::GameState::Snake::ALIVE,
								snakes::Direction::UP);

					// Если депуть не выбран, то выберем его
					if (CurrentGame_deputy_id == -1) {
						chooseDeputy();
					}
				}
			}
			break;
		}
		default: {
			qWarning("Received message has no type");
			break;
		}
		}
	}
}

// ================================================================================================
// ######  PUBLIC METHODS   #######################################################################
// ================================================================================================

NetworkGameController::NetworkGameController(
		QHostAddress multicast_address,
		quint16 multicast_port,
		QObject *parent)
	: GameController{parent},
	  multicast_address{multicast_address},
	  multicast_port{multicast_port}
	{

	// Your own stuff
	CurrentGame_id = -1;
	CurrentGame_name = 0;
	CurrentGame_join_state = JoinState::NOT_JOINED;

	// Players stuff
	CurrentGame_players_ids = {};
	CurrentGame_id2name = {};
	CurrentGame_id2type = {};
	CurrentGame_id2role = {};
	CurrentGame_id2score = {};
	CurrentGame_id2dir = {};
	CurrentGame_master_id = 0;
	CurrentGame_deputy_id = 0;

	// Field
	CurrentGame_field_height = 0;
	CurrentGame_field_width = 0;
	CurrentGame_field_state_order = 0;
	CurrentGame_field = 0;
	CurrentGame_player_snakes = {};

	// Game config
	CurrentGame_static_foods = 0;
	CurrentGame_state_delay_ms = 0;
	CurrentGame_player_timeout_ms = 0;

	// Game info
	CurrentGame_game_name = "";

	channel = new Network::NetworkMessageChannel(multicast_address, multicast_port, this);
	QObject::connect(channel, &NetworkMessageChannel::readyReceive,
					 this, &NetworkGameController::processMessages);

	discover_timer = new QTimer(this);
	announcement_timer = new QTimer(this);

	// Сделаем дискавер сообщение и заведём таймер на его отправку
	setDiscoverTimer();

	process_field_order = 0;
}

// ================================================================================================

NetworkGameController::~NetworkGameController() {
	delete channel;
	for (auto &timer : known_games_timers) {
		delete timer;
	}
}

// ================================================================================================

void NetworkGameController::hostGame(
		const int field_width,
		const int field_height,
		const int static_foods,
		const int state_delay_ms,
		const QString game_name,
		const QString name,
		const char type_ch
		) {

	// Для того, чтобы захостить игру, надо сделать:
	//	Проверить правильность введённых данных
	//	Заполнить настройки игры
	//	Заспавнить свою змейку
	//	Сделать announcement message
	//	Установить свой id в канале (он не умеет это сам делать)
	//	Просигналить вьюхе, чтобы та показала конфиг
	//  Просигналить вьюхе, чтобы та персчитала размеры поля
	//	Запустить таймер для просчёта игры
	//	Завести таймер для сообщений (отправлять PingMsg, если нужно)

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Проверки введённых данных @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	if (field_width < Constraints_MIN_FIELD_WIDTH || Constraints_MAX_FIELD_WIDTH < field_width) {
		QString error_msg;
		QTextStream stream(&error_msg);

		stream << "Field width must be from ";
		stream << Constraints_MIN_FIELD_WIDTH;
		stream << " to ";
		stream << Constraints_MAX_FIELD_WIDTH;

		emit error(error_msg);
		return;
	}

	if (field_height < Constraints_MIN_FIELD_HEIGHT || Constraints_MAX_FIELD_HEIGHT < field_height) {
		QString error_msg;
		QTextStream stream(&error_msg);

		stream << "Field height must be from ";
		stream << Constraints_MIN_FIELD_HEIGHT;
		stream << " to ";
		stream << Constraints_MAX_FIELD_HEIGHT;

		emit error(error_msg);
		return;
	}

	if (static_foods < Constraints_MIN_STATIC_FOODS || Constraints_MAX_STATIC_FOODS < static_foods) {
		QString error_msg;
		QTextStream stream(&error_msg);

		stream << "Static food count must be from ";
		stream << Constraints_MIN_STATIC_FOODS;
		stream << " to ";
		stream << Constraints_MAX_STATIC_FOODS;

		emit error(error_msg);
		return;
	}

	if (state_delay_ms < Constraints_MIN_STATE_DELAY_MS || Constraints_MAX_STATE_DELAY_MS < state_delay_ms) {
		QString error_msg;
		QTextStream stream(&error_msg);

		stream << "Delay between field updates must be from ";
		stream << Constraints_MIN_STATE_DELAY_MS;
		stream << " to ";
		stream << Constraints_MAX_STATE_DELAY_MS;

		emit error(error_msg);
		return;
	}

	if (type_ch != 0 && type_ch != 1) {
		emit error(QString("Unknown type"));
		return;
	}

	// @@@@@@@@@@@@@@@@@@@@@@@
	// @ Заполняем настройки @
	// @@@@@@@@@@@@@@@@@@@@@@@

	// Создадим контейнеры
	CurrentGame_players_ids = {};
	CurrentGame_id2name = {};
	CurrentGame_id2type = {};
	CurrentGame_id2role = {};
	CurrentGame_id2score = {};
	CurrentGame_id2dir = {};
	CurrentGame_player_snakes = {};
	CurrentGame_field = Field(field_height, field_width);

	// Выставим свои настройки
	CurrentGame_id = 0; // Начинаем нумерацию игроков с 0
	CurrentGame_name = name;
	CurrentGame_id2type[CurrentGame_id] = ((type_ch == 0) ? snakes::PlayerType::HUMAN : snakes::PlayerType::ROBOT);
	CurrentGame_id2role[CurrentGame_id] = snakes::NodeRole::MASTER; // Мы хостим => мы - мастер
	CurrentGame_join_state = JoinState::JOINED; // Странно было бы считать, что мы не подключены к своей же игре

	// Поместим себя в контейнеры
	CurrentGame_players_ids.insert(CurrentGame_id);
	CurrentGame_id2name.insert(CurrentGame_id, CurrentGame_name);
	CurrentGame_id2type.insert(CurrentGame_id, CurrentGame_id2type[CurrentGame_id]);
	CurrentGame_id2role.insert(CurrentGame_id, CurrentGame_id2role[CurrentGame_id]);
	CurrentGame_id2score.insert(CurrentGame_id, 0); // Наш счёт сейчас = 0

	// Заполним остальные настройки
	CurrentGame_master_id = CurrentGame_id; // Мы - мастер
	CurrentGame_deputy_id = -1; // Типа депутя нет

	CurrentGame_field_height = field_height;
	CurrentGame_field_width = field_width;
	CurrentGame_field_state_order = 0;

	CurrentGame_static_foods = static_foods;
	CurrentGame_state_delay_ms = state_delay_ms;
	CurrentGame_player_timeout_ms = CurrentGame_state_delay_ms * 0.8;

	CurrentGame_game_name = game_name;

	// @@@@@@@@@@@@@@@@@@
	// @ Спавним змейку @
	// @@@@@@@@@@@@@@@@@@

	// Еду не выставляем, т.к. потом по таймауту просчитается поле и произойдёт следующее:
	//		просчитаются столкновения (их не будет)
	//		змейки (она одна на поле) продвинутся
	//		поставится еда
	//		поле преобразуется в StateMsg и отправится всем по сокету
	//		сокет отправит и самому себе, поэтому мы примем сообщение и обновим у себя поле
	auto [head, tail] = CurrentGame_field.spawnSnake(CurrentGame_id, 0, 0, CurrentGame_field_height / 2, CurrentGame_field_width);
	CurrentGame_player_snakes.insert(CurrentGame_id, ProtoHelp::Snake::make(
				CurrentGame_id,
				{head, tail},
				snakes::GameState::Snake::ALIVE,
				snakes::Direction::UP));
	// Все змейки спавнятся по одному алгоритму - ставим голову посередине свободного блока,
	//	а второй кусочек тела ставим ниже головы.
	// Поэтому изначально змейка смотрит наверх
	CurrentGame_id2dir.insert(CurrentGame_id, snakes::Direction::UP);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Делаем сообщение-анонс @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@

	// Функция делает сбор всех настроек, нужных для анонса, из класса CurrentGame
	//	и отправляет это всё дело мультикастом. Кто примет, а кто нет - не наша забота
	//makeAnnouncementMessage();

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Ставим свой айди в канале @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Важно помнить, что
	//	1) Канал не знает о топологии игры - для него нет понятий "мастер" и "депуть"
	//	2) Канал отправляет сообщения только тем хостам, которые установили с ним соединение
	// Изначально у канала нет id (он равен -1000), следовательно он "не в сети" и не
	//	сможет никому отправлять сообщения. И принимать тоже не сможет, потому что, не зная
	//	своего id, мы не сможем выдать id новому игроку, если канал принадлежит мастеру, а
	//	так же не сможем подтверждать сообщения от других игроков, если канал принадлежит
	//	не мастеру. Опять же, из-за того, что у нас нет id.
	// Поэтому насильно выставим id каналу
	channel->setId(CurrentGame_id);
	channel->setDelay(CurrentGame_state_delay_ms);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Сигналим вьюхе о конфиге @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Последняя строка, нужная для отправки конфига во вьюху
	QString field_size = QString::number(CurrentGame_field_height);
	field_size += " x ";
	field_size += QString::number(CurrentGame_field_width);

	emit configReady(
				CurrentGame_game_name,
				field_size,
				QString::number(CurrentGame_static_foods)
				);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Сигналим вьюхе об изменении размеров поля @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Вьюха только посчитает какого размера должны быть клетки, но ничего не отрисует

	emit fieldResized(CurrentGame_field_height, CurrentGame_field_width);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Запускаем таймер для просчёта игры @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Мы будем считать поле каждые state_delay_ms, а дальше:
	//		Отправлять всем, если мы - мастер
	//		Ничего не делать, если мы - не мастер

	startFieldProcessingTimer();

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Пацаны... Пацаны... Вы меня слышите? Пацаны... @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Так как мы хостим игру, то есть мы - мастер, то нам важно знать
	//	кто из игроков живой, а кто нет. Поэтому заводим таймеры
	//	для PingMsg на всех игроков. Обновлять каждый таймер будем,
	//	соответственно, если отправили какому-то игроку сообщение.

	setPingMessageTimer(-1);

	emit connectedToGame();
}

// ================================================================================================

void NetworkGameController::joinGame(
		const QString game_name,
		const QString player_name,
		const char type_ch,
		const char role_ch
		) {

	// Для того, чтобы подключиться к игре, на до сделать:
	//	Проверить правильность введённых данных
	//	Отправить сообщение о подключении
	//	Выставить первоначальные настройки игры
	//	Завести таймер на сообщения (отправлять PingMsg, если надо)
	//	Просигналить вьюхе, чтобы та показала конфиг
	//		Мы сможем показать конфиг, потому что в анонсе игры,
	//		по которому мы подключаемся, сожержится
	//		вся необходимая информация

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Проверки введённых данных @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	if (!known_games.contains(game_name)) {
		emit error(QString("Can not join to the game, it went out of scope"));
		return;
	}
	const snakes::GameAnnouncement &game = known_games[game_name];

	if (type_ch != 0 && type_ch != 1) {
		emit error(QString("Unknown type"));
		return;
	}

	if (role_ch != 0 && role_ch != 1) {
		emit error("Unknown role");
		return;
	}

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Отправка сообщения о подключении @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Достанем из анонса игры некоторые данные, нужные для подключения.
	// type и role сохраняем в перменные, а не в map-ы,
	//	потому что для сохранения в map-у нужен наш id в игре,
	//	которого на данный момент у нас нет. Он появится, когда
	//	соединение с мастером будет установлено и он сообщит нам
	//	наш id
	snakes::PlayerType type = ((type_ch == 0) ? snakes::PlayerType::HUMAN : snakes::PlayerType::ROBOT);
	snakes::NodeRole role = ((role_ch == 0) ? snakes::NodeRole::VIEWER : snakes::NodeRole::NORMAL);
	CurrentGame_name = player_name;

	// Отправляем сообщение
	// Мы не сможем отправить сообщение только в том случае, если
	//	1) Пытаемся отправить неизвестному хосту.
	//	   Это невозможно, потому что в функции joinGame в канале
	//		происходит парсинг анонса игры и всех игроков оттуда канал
	//		добавляет себе как известных
	//	2) Буфферы отправки канала переполнены.
	//	   Это тоже невозможная ситуация, т.к. мы ещё
	//		не отправили ни одного сообщения
	if (channel->joinGame(
				game,
				type,
				CurrentGame_name,
				role
				) != 0) {

		emit error(QString("Can not join to the game"));
		return;
	}

	// @@@@@@@@@@@@@@@@@@@@@@@
	// @ Парсинг анонса игры @
	// @@@@@@@@@@@@@@@@@@@@@@@

	// Пока находимся в ожидании Ack, можно выставить настройки игры, к которой
	//	подключаемся. Если подключение не удастся, то настройки будут сброшены.

	// Наше состояние подключения
	CurrentGame_join_state = JoinState::SENT_JOIN;

	QVector<snakes::GamePlayer> players = ProtoHelp::GameAnnouncement::getPlayers(game);
	for (auto &player : players) {
		int player_id = ProtoHelp::GamePlayer::getPlayerId(player);
		CurrentGame_players_ids.insert(player_id);
		CurrentGame_id2name.insert(player_id, ProtoHelp::GamePlayer::getPlayerName(player));
		CurrentGame_id2type.insert(player_id, ProtoHelp::GamePlayer::getPlayerType(player));
		CurrentGame_id2role.insert(player_id, ProtoHelp::GamePlayer::getPlayerRole(player));
		CurrentGame_id2score.insert(player_id, ProtoHelp::GamePlayer::getPlayerScore(player));
	}

	CurrentGame_master_id = ProtoHelp::GameAnnouncement::getMasterId(game);

	if (ProtoHelp::GameAnnouncement::hasDeputy(game)) {
		CurrentGame_deputy_id = ProtoHelp::GameAnnouncement::getDeputyId(game);
	}

	CurrentGame_field_height = ProtoHelp::GameAnnouncement::getHeight(game);
	CurrentGame_field_width = ProtoHelp::GameAnnouncement::getWidth(game);
	CurrentGame_field_state_order = -1;

	CurrentGame_static_foods = ProtoHelp::GameAnnouncement::getStaticFoods(game);
	CurrentGame_state_delay_ms = ProtoHelp::GameAnnouncement::getStateDelay(game);
	CurrentGame_player_timeout_ms = CurrentGame_state_delay_ms * 0.8;

	CurrentGame_game_name = ProtoHelp::GameAnnouncement::getGameName(game);

	// @@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Мастер, ты там живой? @
	// @@@@@@@@@@@@@@@@@@@@@@@@@

	// Итак, мы отправили сообщение о подключении и находимся в состоянии
	//	ожидания Ack сообщения. Если ничего больше не предпринимать, то, может быть так,
	//	что канал удалит мастера, которому мы слали сообщение, по
	//	таймауту (от мастера долго ничего не приходило).
	// Но об этом мы не узнаем, мы просто продолжим ждать Ack.
	// Поэтому надо завести таймер на PingMsg. Мы будем отправлять его,
	//	если клиент будет сидеть сложа руки и не тыкать на кнопки поворота.
	//	Ну а если тыкать будет, то будет отправляться не PingMsg, а SteerMsg.
	// Так или иначе, мы будем пытаться отправить мастеру сообщение.
	// Благодаря этому, мы сможем узнать от канала (попытаемся отправить и он
	//	вернёт нам -1), что мастер отвалился. Тогда надо будет начинать слать
	//	сообщения депутю, если таковой есть.
	// Канал сохранит буфер отправки для мастера, поэтому мы сможем
	//	нормально переотправить JoinMsg, который лежал в мастеровом буфере.

	setPingMessageTimer(CurrentGame_master_id);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Сигналим вьюхе о конфиге @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Последняя строка, нужная для отправки конфига во вьюху
	QString field_size = QString::number(CurrentGame_field_height);
	field_size += " x ";
	field_size += QString::number(CurrentGame_field_width);

	emit configReady(
				CurrentGame_game_name,
				field_size,
				QString::number(CurrentGame_static_foods)
				);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @ Сигналим вьюхе об изменении размеров поля @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Вьюха только посчитает какого размера должны быть клетки, но ничего не отрисует

	emit fieldResized(CurrentGame_field_height, CurrentGame_field_width);
}

// ================================================================================================

void NetworkGameController::disconnectGame() {
	// Отключиться от игры можем только в том случае, если мы были подключены к ней
	if (CurrentGame_join_state == JoinState::JOINED) {
		// Формируем сообщение
		snakes::GameMessage disconnect_msg = ProtoHelp::RoleChangeMsg::make(
					snakes::NodeRole::VIEWER,
					snakes::NodeRole::VIEWER,
					ProtoHelp::RoleChangeMsg::AddMode::ADD_SENDER_ROLE,
					CurrentGame_id,
					CurrentGame_master_id
					);
		// Кидаем сообщение об отключении. Канал может не доставить сообщения по
		//	тем же двум причинам, что описаны в начале функции joinGame.
		// Вообще говоря, если сообщение не доставится, это не наша проблема.
		// Доставится - хорошо, нас сделают зомби
		// Не доставится - не очень хорошо, но тоже окей, потому что тогда
		//	канал получателя вырубит нас по таймауту, а потом поставит нашей змейке
		//	режим зомби. В двух случаях результат одинаковый.
		// И да, в обоих случаях, если мы были мастером, результат тоже будет
		//	одинаковый. Но к нему мы придём разными путями.
		// Если человеку доходит сообщение, что мастер выходит, то он назначает
		//	мастером депутя и начинает слать сообщения ему.
		// Если же сообщение не дойдёт, то канал человека выбросит мастера по таймауту.
		//	Но поскольку клиент постоянно шлёт сообщения (поворот или пинг), то
		//	в какой-то момент его контроллер получит -1 от канала при отправке
		//	сообщения своему мастеру. Тогда контроллер переключится на депутя и начнёт
		//	слать сообщения ему
		if (CurrentGame_id2role[CurrentGame_id] == snakes::NodeRole::MASTER) {
			// Если мастер, то рассылаем всем. Все переключатся на депутя.
			// Депуть же возьмёт на себя обязанности мастера, пометит змейку
			//	мастера как зомби, выберет нового депутя и продолжит игру.
			if (channel->send(disconnect_msg, -1) != 0) {
				qWarning("Disconnect message was not sent!");
			}
		}
		else {
			// Если не мастер, то шлём мастеру. Мастер у себя обработает наш выход,
			//	пометит нашу змейку как зомби, если надо, выберет депутя (если
			//	депутем были мы) и продолжит игру.
			if (channel->send(disconnect_msg, CurrentGame_master_id) != 0) {
				qWarning("Disconnect message was not sent!");
			}
		}
		// Клиентам не нужно знать, что кто-то из игроков вышел. Для них
		//	змейки вышешдших будут просто двигаться прямо (т.к. так
		//	двигаются змейки зомби)

	}
	// Сбрасываем канал
	channel->reset();
	// Сбрасываем настройки игры
	resetCurrentGame();
	// Помечаем, что мы сейчас не подключены
	CurrentGame_join_state = JoinState::NOT_JOINED;
	// Сигналим вьюхе, что надо почистить поле
	emit clearField();
}

// ================================================================================================

void NetworkGameController::uMove() {
	move(snakes::Direction::UP);
}

// ================================================================================================

void NetworkGameController::dMove() {
	move(snakes::Direction::DOWN);
}

// ================================================================================================

void NetworkGameController::lMove() {
	move(snakes::Direction::LEFT);
}

// ================================================================================================

void NetworkGameController::rMove() {
	move(snakes::Direction::RIGHT);
}


} // namespace Network
