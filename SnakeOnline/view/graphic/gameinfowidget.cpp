#include "gameinfowidget.h"

namespace View {
namespace Graphic {

GameInfoWidget::GameInfoWidget(QWidget *parent) : QWidget(parent) {
	main_layout = new QVBoxLayout();

	// Create rating and config headers
	header_layout = new QHBoxLayout();
	rating_header = new QLabel("Rating");
	rating_header->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	rating_header->setStyleSheet("font-weight: bold");
	config_header = new QLabel("Game config");
	config_header->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	config_header->setStyleSheet("font-weight: bold");

	// Group rating and config headers into layout
	header_layout->addWidget(rating_header, 50);
	header_layout->addWidget(config_header, 50);

	// Add layout with headers to the main layout
	main_layout->addLayout(header_layout);

	// Create rating list
	rating_list = new QListWidget();

	// Create labels in which game information will be shown
	config_layout = new QVBoxLayout();
	config_host_label = new QLabel("Game:");
	config_host_info = new QLabel();
	config_size_label = new QLabel("Field size:");
	config_size_info = new QLabel();
	config_static_foods_label = new QLabel("Static foods:");
	config_static_foods_info = new QLabel();
	config_layout->addWidget(config_host_label);
	config_layout->addWidget(config_host_info);
	config_layout->addWidget(config_size_label);
	config_layout->addWidget(config_size_info);
	config_layout->addWidget(config_static_foods_label);
	config_layout->addWidget(config_static_foods_info);

	// Group game information into layout
	info_layout = new QHBoxLayout();
	info_layout->addWidget(rating_list, 50);
	info_layout->addLayout(config_layout, 50);

	// Add info layout to the main layout
	main_layout->addLayout(info_layout);

	// Set main layout to this widget
	this->setLayout(main_layout);
}

GameInfoWidget::~GameInfoWidget() {
	// Layout deletes all of its children
	delete main_layout;

	qInfo("Deleted GameInfoWidget");
}

void GameInfoWidget::showRating(QVector<QString> rating) {
	rating_list->clear();
	for (auto &rating_entry : rating) {
		rating_list->addItem(rating_entry);
	}
}
void GameInfoWidget::showConfig(QString game_name, QString field_size, QString static_foods) {
	config_host_info->setText(game_name);
	config_size_info->setText(field_size);
	config_static_foods_info->setText(static_foods);
}

} // namespace Graphic
} // namespace View
