#ifndef VIEW_GRAPHIC_FIELDWIDGET_H
#define VIEW_GRAPHIC_FIELDWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QGraphicsRectItem>

namespace View {
namespace Graphic {

class FieldWidget : public QWidget {
	Q_OBJECT
private:
	// Layouts
	QVBoxLayout *main_layout;

	// Widgets
	QGraphicsScene *field_scene = nullptr;
	QGraphicsView *field_view = nullptr;

	// Field
	int width;
	int height;
	const QColor empty_block_color = Qt::green;
	const QColor border_color = Qt::black;

	// Blocks size
	double block_width;
	double block_height;

	QVector<QVector<QGraphicsRectItem*>> field_blocks;

protected:
	void resizeEvent(QResizeEvent *event);

public:
	explicit FieldWidget(QWidget *parent = nullptr);
	~FieldWidget();

public slots:
	void resizeField(int _width, int _height);
	void clearField();
	void drawBlock(int row, int col, QColor block_color);
signals:

};

} // namespace Graphic
} // namespace View

#endif // VIEW_GRAPHIC_FIELDWIDGET_H
