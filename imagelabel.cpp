#include "imagelabel.h"
#include "arrowwidget.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QVector2D>
#include <QDebug>

ImageLabel::ImageLabel(QWidget *parent) :
	QLabel(parent),
	grabbedArrow(0)
{
}

QPixmap ImageLabel::mergedPixmap() const
{
	QPixmap background = pixmap()->copy();
	foreach (ArrowWidget *arrow, arrows)
	{
		arrow->clearFocus();
		arrow->unselected();
		arrow->render(&background, arrow->geometry().topLeft(), QRegion(), QWidget::DrawChildren);
	}
	return background;
}

void ImageLabel::mousePressEvent(QMouseEvent *event)
{
	start = event->pos();
	event->accept();

	return QLabel::mousePressEvent(event);
}

void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
	if (!start.isNull())
	{
		QPoint end = event->pos();
		qreal length = QVector2D(end - start).length();
		if (length > 20)
		{
			grabbedArrow = new ArrowWidget(start, end, this);
			grabbedArrow->show();
			start = QPoint();
		}
		return event->accept();
	}
	else if (grabbedArrow)
	{
		grabbedArrow->setEnd(event->pos());
		return event->accept();
	}

	return QLabel::mouseMoveEvent(event);
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
	if (grabbedArrow)
	{
		connect(grabbedArrow, SIGNAL(selected(ArrowWidget*)), this, SIGNAL(selected(ArrowWidget*)));
		connect(grabbedArrow, SIGNAL(destroyed(QObject*)), this, SLOT(remove(QObject*)));
		grabbedArrow->showEdit();
		arrows << grabbedArrow;
		grabbedArrow = 0;
		return event->accept();
	}
	start = QPoint();
	return QLabel::mouseReleaseEvent(event);
}

void ImageLabel::remove(QObject *arrow)
{
	arrows.removeAll(static_cast<ArrowWidget *>(arrow));
}

QDataStream &operator >> (QDataStream &stream, ImageLabel &imageLabel)
{
	int count;
	stream >> count;
	QPoint start, end;
	QString text;
	QColor color;
	for (int i = 0; i < count; ++i)
	{
		stream >> start >> end >> text >> color;
		ArrowWidget *arrow = new ArrowWidget(start, end, &imageLabel);
		QObject::connect(arrow, SIGNAL(selected(ArrowWidget*)), &imageLabel, SIGNAL(selected(ArrowWidget*)));
		QObject::connect(arrow, SIGNAL(destroyed(QObject*)), &imageLabel, SLOT(remove(QObject*)));
		imageLabel.arrows << arrow;
		arrow->color = color;
		arrow->showEdit(text);
//		if (inverted)
//			arrow->invert();
		arrow->show();
	}
	return stream;
}

QDataStream &operator << (QDataStream &stream, const ImageLabel &imageLabel)
{
	stream << imageLabel.arrows.count();
	foreach (ArrowWidget *arrow, imageLabel.arrows)
		stream << arrow->start << arrow->end << arrow->lineEdit->text() << arrow->color;
	return stream;
}
