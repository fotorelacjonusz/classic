#ifndef GPXDIALOG_H
#define GPXDIALOG_H

#include "ntpclient.h"

#include <QDialog>
#include <QTimer>
#include <QDateTime>

namespace Ui {
class GpxDialog;
}

class QAbstractMessageHandler;

class GpxDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit GpxDialog(QWidget *parent = 0);
	~GpxDialog();
	
	virtual void setVisible(bool visible);
	virtual void accept();
	virtual void reject();
	
	QPointF position(QDateTime dateTime) const;
	bool updateFiles() const;
	
private slots:
	void updateTime();
	
	void on_loadButton_clicked();
	
private:
	Ui::GpxDialog *ui;
	QTimer timer;
	QAbstractMessageHandler * const handler;
	
	typedef QMap<QDateTime, QPointF> Segment;
	QList<Segment> track;

	NtpClient ntpClient;
};

#endif // GPXDIALOG_H
