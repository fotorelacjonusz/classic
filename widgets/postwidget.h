#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <QWidget>

class QToolBox;

namespace Ui {
class PostWidget;
}

class PostWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit PostWidget(QToolBox *parent = 0);
	~PostWidget();

	void append(QString text, bool ready = false);
	int imageNumber() const;
	void setImageNumber(int number);
	QString text() const;
	bool isReady() const;

signals:
	void appended(int progress);
	
private:
	Ui::PostWidget *ui;
	bool m_ready;
	int m_imageNumber;
};

#endif // POSTWIDGET_H
