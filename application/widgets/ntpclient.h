#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTime>
#include <QTimer>
#include <QHostInfo>

class NtpClient : public QObject
{
	Q_OBJECT
public:
	explicit NtpClient(QObject *parent = nullptr);
	QDateTime utcTime() const;

signals:
	void utcTimeFound(QDateTime);

public slots:
	void updateTime();

private slots:
	void sendRequest(QHostInfo host);
	void readPendingDatagrams();

private:
	QString pool;
	QUdpSocket socket;
	qint64 msCurrentToNtp;
	QTime requestReplyTime;
	QTimer retryTimer;
};

#endif // NTPCLIENT_H
