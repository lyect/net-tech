#ifndef VIEW_GRAPHIC_GAMEINFO_H
#define VIEW_GRAPHIC_GAMEINFO_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QDebug>

#include <iostream>

namespace View {
namespace Graphic {

class GameInfoWidget : public QWidget {
	Q_OBJECT
private:
	// Layouts
	QVBoxLayout *main_layout;
	QHBoxLayout *header_layout;
	QHBoxLayout *info_layout;
	QVBoxLayout *config_layout;

	// Widgets
	QLabel *rating_header;
	QLabel *config_header;
	QListWidget *rating_list;
	QLabel *config_host_label;
	QLabel *config_host_info;
	QLabel *config_size_label;
	QLabel *config_size_info;
	QLabel *config_static_foods_label;
	QLabel *config_static_foods_info;

public:
	explicit GameInfoWidget(QWidget *parent = nullptr);
	~GameInfoWidget();

public slots:
	void showRating(QVector<QString> rating);
	void showConfig(QString game_name, QString field_size, QString static_foods);
signals:

};

} // namespace Graphic
} // namespace View

#endif // VIEW_GRAPHIC_GAMEINFO_H
