#pragma once

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

	explicit PostWidget(QToolBox *parent = nullptr);
	~PostWidget();

	void appendImage(AbstractImage *image);
	int lastImageNumber() const;
	QString text() const;
	void setLast(bool last = true);
	bool isFull() const;
	void setHeader(QString text);
	void setFooter(QString text);

private:
	Ui::PostWidget *ui;
	QList<AbstractImage *> images;

	QString openingTags;
	QString closingTags;
	bool isLast;
	QString header;
	QString footer;
};
