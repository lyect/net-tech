#ifndef VIEW_GRAPHIC_AVAILABLEGAMESWIDGET_H
#define VIEW_GRAPHIC_AVAILABLEGAMESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QDebug>
#include <QResizeEvent>
#include <QHeaderView>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>

namespace View {
namespace Graphic {

class AnnouncedGamesWidget : public QWidget {
	Q_OBJECT
private:

	enum TableColumn {
		GAME_INFO_COLUMN = 0,
		FIELD_SIZE_COLUMN = 1,
		FOOD_RULE_COLUMN = 2
	};

	// Layouts
	QVBoxLayout *main_layout;
	QFormLayout *name_form_layout;
	QHBoxLayout *choose_role_layout;

	// Widgets
	QLabel *header;
	QTableWidget *announced_games_table;
	QLabel *name_form_label;
	QLineEdit *name_form_text;
	QCheckBox *normal_role_cb;
	QCheckBox *viewer_role_cb;
	QPushButton *connect_button;

	// Other stuff
	QHash<QString, int> shown_games;
	bool is_connected;

	// Methods
	void addGameInfo(
			const int &row,
			const int &column,
			const QString &game_name,
			const QString &game_address
			);
	void addFieldSize(
			const int &row,
			const int &column,
			const QString &field_height,
			const QString &field_width
			);
	void addFoodRule(
			const int &row,
			const int &column,
			const QString &food_rule
			);

	void blockConnectButton();
	void unblockConnectButton();
	void connectButtonAction();

public:

	explicit AnnouncedGamesWidget(QWidget *parent = nullptr);
	~AnnouncedGamesWidget();


public slots:
	void showGameAnnouncement(
			const QString game_name,
			const QString game_address,
			const QString field_height,
			const QString field_width,
			const QString food_static
			);
	void removeGameAnnouncement(
			QString game_name
			);
	void showConnectedState();

signals:

	void joinGame(
			const QString game_name,
			const QString player_name,
			const char type_ch,
			const char role_ch);
	void disconnectGame();

};

} // namespace Graphic
} // namespace View

#endif // VIEW_GRAPHIC_AVAILABLEGAMESWIDGET_H
