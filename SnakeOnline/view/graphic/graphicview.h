#ifndef VIEW_GRAPHIC_GRAPHICVIEW_H
#define VIEW_GRAPHIC_GRAPHICVIEW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "announcedgameswidget.h"
#include "fieldwidget.h"
#include "gamehostingwidget.h"
#include "gameinfowidget.h"

namespace View {
namespace Graphic {

class GraphicView : public QWidget {
	Q_OBJECT
private:
	// Layouts
	QHBoxLayout *main_layout;
	QVBoxLayout *controls_layout;

	// Widget
	FieldWidget *field_widget;
	AnnouncedGamesWidget *announced_games_widget;
	GameHostingWidget *game_hosting_widget;
	GameInfoWidget *game_info_widget;

protected:
	void keyPressEvent(QKeyEvent *event) override;

public:
	explicit GraphicView(QWidget *parent = nullptr);
	~GraphicView();

	void addFieldWidget(FieldWidget *fw);
	void addAnnouncedGamesWidget(AnnouncedGamesWidget *agw);
	void addGameHostingWidget(GameHostingWidget *ghw);
	void addGameInfoWidget(GameInfoWidget *giw);

signals:
	void uMove();
	void dMove();
	void lMove();
	void rMove();
};

} // namespace Graphic
} // namespace View

#endif // VIEW_GRAPHIC_GRAPHICVIEW_H
