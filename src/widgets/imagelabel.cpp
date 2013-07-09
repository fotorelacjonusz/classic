#include "imagelabel.h"
#include "arrowwidget.h"
#include "lineedit.h"
#include "settings/settingsdialog.h"
#include <QMouseEvent>
#include <QVector2D>
#include <QDebug>

ImageLabel::ImageLabel(QWidget *parent, QWidget *firstWidget) :
	QLabel(parent),
	grabbedArrow(0),
	firstWidget(firstWidget)
{
}

bool ImageLabel::isNull() const
{
	return !pixmap() || pixmap()->isNull();
}

QPixmap ImageLabel::mergedPixmap() const
{
	if (isNull())
		return QPixmap();
	QPixmap background = pixmap()->copy();
	foreach (ArrowWidget *arrow, arrows)
	{
		arrow->clearFocus();
		arrow->unselect();
		arrow->render(&background, arrow->geometry().topLeft(), QRegion(), QWidget::DrawChildren);
	}
	return background;
}

QWidget *ImageLabel::lastArrow() const
{
	return arrows.isEmpty() ? 0 : arrows.last();
}

void ImageLabel::setFirstWidget(QWidget *widget)
{
	firstWidget = widget;
}

void ImageLabel::write(QIODevice *device) const
{
	mergedPixmap().save(device, "JPG", SETTINGS->jpgQuality);
}

void ImageLabel::mousePressEvent(QMouseEvent *event)
{
	if (isNull())
		return QLabel::mousePressEvent(event);
	
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
			foreach (ArrowWidget *arrow, arrows)
				if (QVector2D(arrow->start - start).length() < 10.0)
				{
					grabbedArrow = new ArrowWidget(arrow->start, end, this, "");
					grabbedArrow->show();
					setTabOrder(grabbedArrow->lineEdit, arrow->lineEdit);
					start = QPoint();
					return event->accept();
					break;
				}

			grabbedArrow = new ArrowWidget(start, end, this, "...");
			grabbedArrow->show();
			setTabOrder(arrows.isEmpty() ? firstWidget : arrows.last(), grabbedArrow);
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
//	qDebug() << "mouserelease" << grabbedArrow;
	if (grabbedArrow)
	{
//		connect(grabbedArrow, SIGNAL(selected(ArrowWidget*)), this, SIGNAL(selected(ArrowWidget*)));
		connect(grabbedArrow, SIGNAL(destroyed(QObject*)), this, SLOT(remove(QObject*)));
		arrows << grabbedArrow;
		grabbedArrow->showEdit();
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
		ArrowWidget *arrow = new ArrowWidget(start, end, &imageLabel, "");
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
