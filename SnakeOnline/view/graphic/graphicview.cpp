#include "graphicview.h"

namespace View {
namespace Graphic {

void GraphicView::keyPressEvent(QKeyEvent *event) {
	auto k = event->key();

	switch (k) {
	case Qt::Key_W:
		emit uMove();
		break;
	case Qt::Key_A:
		emit lMove();
		break;
	case Qt::Key_S:
		emit dMove();
		break;
	case Qt::Key_D:
		emit rMove();
		break;
	}
}

GraphicView::GraphicView(QWidget *parent) : QWidget(parent) {
	main_layout = new QHBoxLayout();
	controls_layout = new QVBoxLayout();

	field_widget = nullptr;
	announced_games_widget = nullptr;
	game_hosting_widget = nullptr;
	game_info_widget = nullptr;

	this->setWindowIcon(QIcon(":icon/icon.ico"));
	this->setWindowTitle("Online Snake");
}

GraphicView::~GraphicView() {
	// Layout deletes all of its children
	delete main_layout;

	qInfo("Deleted GraphicView");
}

void GraphicView::addFieldWidget(FieldWidget *fw) {
	main_layout->addWidget(fw, 60);
	main_layout->addLayout(controls_layout, 40);
	this->setLayout(main_layout);
}
void GraphicView::addAnnouncedGamesWidget(AnnouncedGamesWidget *agw) {
	controls_layout->addWidget(agw, 50);
}
void GraphicView::addGameHostingWidget(GameHostingWidget *ghw) {
	controls_layout->addWidget(ghw, 20);
}
void GraphicView::addGameInfoWidget(GameInfoWidget *giw) {
	controls_layout->addWidget(giw, 30);
}

} // namespace Graphic
} // namespace View
