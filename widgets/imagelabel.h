#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
class ArrowWidget;

class ImageLabel : public QLabel
{
	Q_OBJECT
public:
	explicit ImageLabel(QWidget *parent, QWidget *firstWidget);
	
	QPixmap mergedPixmap() const;
	QWidget *getLastArrow() const;
signals:
//	void selected(ArrowWidget *widget);
	
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

#endif // IMAGELABEL_H
