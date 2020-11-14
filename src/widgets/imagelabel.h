#pragma once

#include "abstractimage.h"
#include <QLabel>
class ArrowWidget;

class ImageLabel : public QLabel, public AbstractImage
{
	Q_OBJECT
public:
	explicit ImageLabel(QWidget *parent, QWidget *firstWidget = nullptr);

	bool isNull() const;
	QPixmap mergedPixmap() const;
	QWidget *lastArrow() const;
	void setFirstWidget(QWidget *widget);

	virtual void write(QIODevice *device) const;

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private slots:
	void remove(QObject *arrow);

private:
	QPoint start;
	QList<ArrowWidget *> arrows;
	ArrowWidget *grabbedArrow;
	QWidget *firstWidget;

	friend QDataStream &operator >> (QDataStream &stream, ImageLabel &imageLabel);
	friend QDataStream &operator << (QDataStream &stream, const ImageLabel &imageLabel);
};

QDataStream &operator >> (QDataStream &stream, ImageLabel &imageLabel);
QDataStream &operator << (QDataStream &stream, const ImageLabel &imageLabel);
