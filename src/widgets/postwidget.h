#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <QWidget>

class QToolBox;
class AbstractImage;

namespace Ui {
class PostWidget;
}

class PostWidget : public QWidget
{
	Q_OBJECT
	
public:
	enum State { Incomplete, Full, Sent, Posted };
	
	explicit PostWidget(QToolBox *parent = 0);
	~PostWidget();

	void appendImage(AbstractImage *image);
	int lastImageNumber() const;
	QString text() const;
	void setLast(bool last = true);
	bool isFull() const;
	
private:
	Ui::PostWidget *ui;
	QList<AbstractImage *> images;
	
	QString openingTags;
	QString closingTags;
	bool isLast;
};

#endif // POSTWIDGET_H
