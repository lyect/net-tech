#include <QApplication>
#include <QScopedPointer>
#include <QThread>
#include <QMetaObject>

#include "./view/graphic/fieldwidget.h"
#include "./view/graphic/gameinfowidget.h"
#include "./view/graphic/announcedgameswidget.h"
#include "./view/graphic/gamehostingwidget.h"
#include "./view/graphic/graphicview.h"

#include "./controllers/gamecontroller.h"
#include "./controllers/network/networkgamecontroller.h"

int main(int argc, char *argv[]) {

	QApplication app(argc, argv);

	// @@@@@@@@@@@@
	// @   VIEW   @
	// @@@@@@@@@@@@

	// View widgets
	QScopedPointer<View::Graphic::FieldWidget> field_widget(new View::Graphic::FieldWidget());
	QScopedPointer<View::Graphic::GameInfoWidget> game_info_widget(new View::Graphic::GameInfoWidget());
	QScopedPointer<View::Graphic::AnnouncedGamesWidget> available_games_widget(new View::Graphic::AnnouncedGamesWidget());
	QScopedPointer<View::Graphic::GameHostingWidget> game_hosting_widget(new View::Graphic::GameHostingWidget());
	// The Main Class for view controlling
	QScopedPointer<View::Graphic::GraphicView> view(new View::Graphic::GraphicView());

	// @@@@@@@@@@@@@@@@@@@@@@
	// @   GAME CONTROLLER  @
	// @@@@@@@@@@@@@@@@@@@@@@

	QSharedPointer<GameController> game_controller(
				new Network::NetworkGameController(
							QHostAddress("239.192.0.4"),
							static_cast<quint16>(9192)
				)
	);

	QThread *cont_thread = new QThread();

	game_controller.data()->moveToThread(cont_thread);

	cont_thread->start();

	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @   CONNECT VIEW TO CONTROLLERS   @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// Game Controller ---> View
	QObject::connect(game_controller.data(), &GameController::ratingReady,
					 game_info_widget.data(), &View::Graphic::GameInfoWidget::showRating);

	QObject::connect(game_controller.data(), &GameController::configReady,
					 game_info_widget.data(), &View::Graphic::GameInfoWidget::showConfig);

	QObject::connect(game_controller.data(), &GameController::gotNewGameAnnouncement,
					 available_games_widget.data(), &View::Graphic::AnnouncedGamesWidget::showGameAnnouncement);

	QObject::connect(game_controller.data(), &GameController::removedGameAnnouncement,
					 available_games_widget.data(), &View::Graphic::AnnouncedGamesWidget::removeGameAnnouncement);

	QObject::connect(game_controller.data(), &GameController::fieldResized,
					 field_widget.data(), &View::Graphic::FieldWidget::resizeField);
	QObject::connect(game_controller.data(), &GameController::clearField,
					 field_widget.data(), &View::Graphic::FieldWidget::clearField);
	QObject::connect(game_controller.data(), &GameController::drawBlock,
					 field_widget.data(), &View::Graphic::FieldWidget::drawBlock);

	QObject::connect(game_controller.data(), &GameController::connectedToGame,
					 game_hosting_widget.data(), &View::Graphic::GameHostingWidget::blockHostButton);

	QObject::connect(game_controller.data(), &GameController::connectedToGame,
					 available_games_widget.data(), &View::Graphic::AnnouncedGamesWidget::showConnectedState);

	// View ---> Game Controller
	QObject::connect(view.data(), &View::Graphic::GraphicView::uMove,
					 game_controller.data(), &GameController::uMove);

	QObject::connect(view.data(), &View::Graphic::GraphicView::dMove,
					 game_controller.data(), &GameController::dMove);

	QObject::connect(view.data(), &View::Graphic::GraphicView::lMove,
					 game_controller.data(), &GameController::lMove);

	QObject::connect(view.data(), &View::Graphic::GraphicView::rMove,
					 game_controller.data(), &GameController::rMove);

	QObject::connect(game_hosting_widget.data(), &View::Graphic::GameHostingWidget::hostGame,
					 game_controller.data(), &GameController::hostGame);

	QObject::connect(available_games_widget.data(), &View::Graphic::AnnouncedGamesWidget::joinGame,
					 game_controller.data(), &GameController::joinGame);

	QObject::connect(available_games_widget.data(), &View::Graphic::AnnouncedGamesWidget::disconnectGame,
					 game_controller.data(), &GameController::disconnectGame);

	// @@@@@@@@@@@@@@@@@@@@@@@@@@
	// @   CONNECT VIEW PARTS   @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@



	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// @   ADD PARTS OF THE VIEW TO VIEW   @
	// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	view->addGameInfoWidget(game_info_widget.data());
	view->addAnnouncedGamesWidget(available_games_widget.data());
	view->addGameHostingWidget(game_hosting_widget.data());
	view->addFieldWidget(field_widget.data());

	// @@@@@@@@@@@@@@@@@
	// @   SHOW VIEW   @
	// @@@@@@@@@@@@@@@@@

	view->show();

	return app.exec();
}
