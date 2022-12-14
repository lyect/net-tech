#include "announcedgameswidget.h"

namespace View {
namespace Graphic {

void AnnouncedGamesWidget::blockConnectButton() {
	connect_button->setEnabled(false);
}

void AnnouncedGamesWidget::unblockConnectButton() {
	connect_button->setEnabled(true);
}

void AnnouncedGamesWidget::connectButtonAction() {

	if (is_connected == true) {
		is_connected = false;
		connect_button->setText("Connect");
		blockConnectButton();
		emit disconnectGame();
		return;
	}

	blockConnectButton();

	auto select = announced_games_table->selectionModel();

	if (!select->hasSelection()) {
		auto selected_rows = select->selectedRows();

		// If selection contains more than one row, only first will be processed
		// Actually, it is impossible to choose more than one row because of table selection model
		int row = selected_rows.at(0).row();

		auto game_info_item = announced_games_table->item(row, TableColumn::GAME_INFO_COLUMN);
		QString game_info = game_info_item->text();
		int delim_idx = game_info.indexOf('[');
		QString game_name = game_info.left(delim_idx);

		QString player_name = name_form_text->text();

		char type_ch = 0;

		char role_ch = 0;
		if (normal_role_cb->checkState() == Qt::Checked) {
			role_ch = 1;
		}

		emit joinGame(
					game_name,
					player_name,
					type_ch,
					role_ch
					);
	}
}

AnnouncedGamesWidget::AnnouncedGamesWidget(QWidget *parent) : QWidget(parent) {
	main_layout = new QVBoxLayout();

	// Create widgets
	header = new QLabel("Announced games");
	header->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	header->setStyleSheet("font-weight: bold");
	announced_games_table = new QTableWidget(0, 3);
	announced_games_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	announced_games_table->setSelectionMode(QAbstractItemView::SingleSelection);
	announced_games_table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	announced_games_table->setHorizontalHeaderLabels({
				"Host info",
				"Field size",
				"Static foods"
	});
	announced_games_table->horizontalHeader()->setSectionResizeMode(
				TableColumn::GAME_INFO_COLUMN,
				QHeaderView::ResizeToContents
	);
	announced_games_table->horizontalHeader()->setSectionResizeMode(
				TableColumn::FIELD_SIZE_COLUMN,
				QHeaderView::ResizeToContents
	);
	announced_games_table->horizontalHeader()->setSectionResizeMode(
				TableColumn::FOOD_RULE_COLUMN,
				QHeaderView::ResizeToContents
	);
	announced_games_table->horizontalHeader()->setStretchLastSection(true);
	name_form_layout = new QFormLayout();
	name_form_label = new QLabel("Name:");
	name_form_text = new QLineEdit();
	name_form_layout->addRow(name_form_label, name_form_text);
	choose_role_layout = new QHBoxLayout();
	normal_role_cb = new QCheckBox("NORMAL");
	viewer_role_cb = new QCheckBox("VIEWER");
	choose_role_layout->addWidget(normal_role_cb);
	choose_role_layout->addWidget(viewer_role_cb);
	connect_button = new QPushButton("Connect");
	connect_button->setEnabled(false);
	is_connected = false;

	// Connect game selection to button unblocking
	QObject::connect(announced_games_table, &QTableWidget::cellClicked,
					 this, &AnnouncedGamesWidget::unblockConnectButton);

	// Connect button to signal
	QObject::connect(connect_button, &QPushButton::clicked,
					 this, &AnnouncedGamesWidget::connectButtonAction);

	// Make check boxes mutually exclusive
	viewer_role_cb->setCheckState(Qt::Checked);
	QObject::connect(normal_role_cb, &QCheckBox::stateChanged,
					 viewer_role_cb, [&]() -> void {
		if (normal_role_cb->checkState() == Qt::Checked) {
			viewer_role_cb->setCheckState(Qt::Unchecked);
		}
	});
	QObject::connect(viewer_role_cb, &QCheckBox::stateChanged,
					 normal_role_cb, [&]() -> void {
		if (viewer_role_cb->checkState() == Qt::Checked) {
			normal_role_cb->setCheckState(Qt::Unchecked);
		}
	});

	// Add widgets to the main layout
	main_layout->addWidget(header);
	main_layout->addWidget(announced_games_table);
	main_layout->addLayout(name_form_layout);
	main_layout->addLayout(choose_role_layout);
	main_layout->addWidget(connect_button);

	// Set layout to this widget
	this->setLayout(main_layout);
}

AnnouncedGamesWidget::~AnnouncedGamesWidget() {
	// Layout deletes all of its children
	delete main_layout;

	qInfo("Deleted AnnouncedGamesWidget");
}

void AnnouncedGamesWidget::addGameInfo(
		const int &row,
		const int &column,
		const QString &host_name,
		const QString &host_address
) {
	QString host_info = host_name + "[" + host_address + "]";

	QTableWidgetItem *new_host_info_item = new QTableWidgetItem(host_info);
	new_host_info_item->setFlags(new_host_info_item->flags() & ~Qt::ItemIsEditable);

	announced_games_table->setItem(row, column, new_host_info_item);
}

void AnnouncedGamesWidget::addFieldSize(
		const int &row,
		const int &column,
		const QString &field_height,
		const QString &field_width
) {
	QString field_size = field_height + "x" + field_width;

	QTableWidgetItem *new_field_size_item = new QTableWidgetItem(field_size);
	new_field_size_item->setFlags(new_field_size_item->flags() & ~Qt::ItemIsEditable);

	announced_games_table->setItem(row, column, new_field_size_item);
}

void AnnouncedGamesWidget::addFoodRule(
		const int &row,
		const int &column,
		const QString &food_rule
) {
	QTableWidgetItem *new_food_rule_item = new QTableWidgetItem(food_rule);
	new_food_rule_item->setFlags(new_food_rule_item->flags() & ~Qt::ItemIsEditable);

	announced_games_table->setItem(row, column, new_food_rule_item);
}

void AnnouncedGamesWidget::showGameAnnouncement(
		QString game_name,
		QString game_address,
		QString field_height,
		QString field_width,
		QString food_static
) {
	int last_row_index = announced_games_table->rowCount();
	announced_games_table->insertRow(last_row_index);

	// Set host info
	addGameInfo(last_row_index, TableColumn::GAME_INFO_COLUMN, game_name, game_address);
	// Set field size
	addFieldSize(last_row_index, TableColumn::FIELD_SIZE_COLUMN, field_height, field_width);
	// Set food rule
	addFoodRule(last_row_index, TableColumn::FOOD_RULE_COLUMN, food_static);

	shown_games.insert(game_name, last_row_index);
}

void AnnouncedGamesWidget::removeGameAnnouncement(QString game_name) {
	announced_games_table->removeRow(shown_games[game_name]);
	shown_games.remove(game_name);
}

void AnnouncedGamesWidget::showConnectedState() {
	is_connected = true;
	connect_button->setText("Disconnect");
	unblockConnectButton();
}

} // namespace Graphic
} // namespace View
