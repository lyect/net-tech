#include "gamehostingwidget.h"

namespace View {
namespace Graphic {

void GameHostingWidget::hostButtonAction() {
	QString width_str = width_text->text();
	QString height_str = height_text->text();
	QString static_foods_str = static_foods_text->text();
	QString state_delay_ms_str = delay_text->text();
	QString game_name = game_name_text->text();
	QString name = player_name_text->text();

	bool ok;
	int width = width_str.toInt(&ok);
	if (!ok) width = 40;
	int height = height_str.toInt(&ok);
	if (!ok) height = 30;
	int static_foods = static_foods_str.toInt(&ok);
	if (!ok) static_foods = 1;
	int state_delay_ms = state_delay_ms_str.toInt(&ok);
	if (!ok) state_delay_ms = 1000;

	if (name.size() == 0) {
		name = "VASYA";
	}

	if (game_name.size() == 0) {
		game_name = "VASYAGAME";
	}

	emit hostGame(
			width,
			height,
			static_foods,
			state_delay_ms,
			game_name,
			name,
			0
			);
}

GameHostingWidget::GameHostingWidget(QWidget *parent) : QWidget(parent) {
	main_layout = new QVBoxLayout();

	// Create header and add it to the main layout
	header = new QLabel("Host new game");
	header->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	header->setStyleSheet("font-weight: bold");
	main_layout->addWidget(header);

	// Create input widgets and their associated labels
	player_name_label = new QLabel("Player Name:");
	player_name_text = new QLineEdit();
	game_name_label = new QLabel("Game Name:");
	game_name_text = new QLineEdit();
	delay_label = new QLabel("Delay:");
	delay_text = new QLineEdit();
	delay_text->setPlaceholderText("1000");
	static_foods_label = new QLabel("Static food:");
	static_foods_text = new QLineEdit();
	static_foods_text->setPlaceholderText("1");
	height_label = new QLabel("Height:");
	height_text = new QLineEdit();
	height_text->setPlaceholderText("30");
	width_label = new QLabel("Width:");
	width_text = new QLineEdit();
	width_text->setPlaceholderText("40");
	port_label = new QLabel("Port:");
	port_text = new QLineEdit();
	port_text->setPlaceholderText("25565");

	// Group input widgets and labels into layout
	input_data_layout = new QFormLayout();
	input_data_layout->addRow(player_name_label, player_name_text);
	input_data_layout->addRow(game_name_label, game_name_text);
	input_data_layout->addRow(delay_label, delay_text);
	input_data_layout->addRow(static_foods_label, static_foods_text);
	input_data_layout->addRow(height_label, height_text);
	input_data_layout->addRow(width_label, width_text);
	input_data_layout->addRow(port_label, port_text);

	// Add layout with input widgets and labels to main layout
	main_layout->addLayout(input_data_layout);

	// Add button to main layout
	host_button = new QPushButton("Host game");
	QObject::connect(host_button, &QPushButton::clicked,
					 this, &GameHostingWidget::hostButtonAction);
	main_layout->addWidget(host_button);

	// Set main layout to this widget
	this->setLayout(main_layout);
}

GameHostingWidget::~GameHostingWidget() {
	// Layout deletes all of its children
	delete main_layout;

	qInfo("Deleted GameHostingWidget");
}

void GameHostingWidget::blockHostButton() {
	host_button->setEnabled(false);
}

void GameHostingWidget::unblockHostButton() {
	host_button->setEnabled(true);
}

} // namespace Graphic
} // namespace View
