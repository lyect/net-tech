#include "fieldwidget.h"

namespace View {
namespace Graphic {

void FieldWidget::resizeEvent(QResizeEvent *event) {
	if (field_view != nullptr && field_scene != nullptr) {
		field_view->fitInView(field_scene->sceneRect(), Qt::KeepAspectRatio);
	}
	QWidget::resizeEvent(event);
}

FieldWidget::FieldWidget(QWidget *parent) : QWidget(parent) {
	main_layout = new QVBoxLayout();

	// Create view and add it to the layout
	field_view = new QGraphicsView();
	main_layout->addWidget(field_view);

	// Set layout to the widget
	this->setLayout(main_layout);

	// Set initial field size
	width = 0;
	height = 0;

	block_width = 0;
	block_height = 0;
}

FieldWidget::~FieldWidget() {
	// Layout deletes all of its children
	delete main_layout;

	qInfo("Deleted FieldWidget");
}

void FieldWidget::resizeField(int _height, int _width) {
	width = _width;
	height = _height;

	block_width = field_view->geometry().width() / static_cast<double>(width);
	block_height = field_view->geometry().height() / static_cast<double>(height);

	field_blocks.clear();
	field_blocks.squeeze();
	field_blocks.resize(height);

	delete field_scene;

	qInfo() << QString::number(field_view->geometry().width());
	qInfo() << QString::number(field_view->geometry().height());

	field_scene = new QGraphicsScene(0, 0, field_view->geometry().width(), field_view->geometry().height(), this);
	field_view->setScene(field_scene);

	for (int row = 0; row < height; ++row) {
		field_blocks[row].resize(width);
		for (int col = 0; col < width; ++col) {
			field_blocks[row][col] = field_scene->addRect(
				col * block_width,
				row * block_height,
				block_width,
				block_height,
				QPen(border_color),
				QBrush(empty_block_color)
			);
		}
	}

	field_view->fitInView(field_scene->sceneRect(), Qt::KeepAspectRatio);
}

void FieldWidget::clearField() {
	for (int row = 0; row < height; ++row) {
		field_blocks[row].resize(width);
		for (int col = 0; col < width; ++col) {
			field_blocks[row][col]->setBrush(empty_block_color);
		}
	}
}

void FieldWidget::drawBlock(int row, int col, const QColor block_color) {
	Q_ASSERT(0 <= row && row < height && 0 <= col && col < width);
	field_blocks[row][col]->setBrush(block_color);
	field_blocks[row][col]->update(
				col * block_width,
				row * block_height,
				block_width,
				block_height);
}

} // namespace Graphic
} // namespace View
