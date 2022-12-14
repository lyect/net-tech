#ifndef VIEW_GRAPHIC_GAMEHOSTINGWIDGET_H
#define VIEW_GRAPHIC_GAMEHOSTINGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QPushButton>

namespace View {
namespace Graphic {

class GameHostingWidget : public QWidget {
	Q_OBJECT
private:
	// Layouts
	QVBoxLayout *main_layout;
	QFormLayout *input_data_layout;

	// Widgets
	QLabel *header;
	QLabel *player_name_label;
	QLineEdit *player_name_text;
	QLabel *game_name_label;
	QLineEdit *game_name_text;
	QLabel *delay_label;
	QLineEdit *delay_text;
	QLabel *static_foods_label;
	QLineEdit *static_foods_text;
	QLabel *height_label;
	QLineEdit *height_text;
	QLabel *width_label;
	QLineEdit *width_text;
	QLabel *port_label;
	QLineEdit *port_text;
	QPushButton *host_button;

	void hostButtonAction();

public:
	explicit GameHostingWidget(QWidget *parent = nullptr);
	~GameHostingWidget();

public slots:
	void blockHostButton();
	void unblockHostButton();
signals:

	void hostGame(
			const int width,
			const int height,
			const int food_static,
			const int state_delay_ms,
			const QString game_name,
			const QString name,
			const char type_ch
			);

};

} // namespace Graphic
} // namespace View

#endif // VIEW_GRAPHIC_GAMEHOSTINGWIDGET_H
