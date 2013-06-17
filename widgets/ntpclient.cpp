#include "ntpclient.h"
#include "ntppacket.h"
#include <QDebug>
#include <QLocale>
#include <QStringList>
#include <limits>

NtpClient::NtpClient(QObject *parent) :
	QObject(parent),
	msCurrentToNtp(std::numeric_limits<qint64>::max())
{
	connect(&socket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	connect(&retryTimer, SIGNAL(timeout()), this, SLOT(updateTime()));
	retryTimer.setSingleShot(false);

	pool = QString("0.%1.pool.ntp.org").arg(QLocale::system().name().split('_').last().toLower());
}

QDateTime NtpClient::utcTime() const
{
	return msCurrentToNtp == std::numeric_limits<qint64>::max() ? QDateTime() : QDateTime::currentDateTimeUtc().toUTC().addMSecs(msCurrentToNtp);
}

void NtpClient::updateTime()
{
	socket.disconnectFromHost();
	retryTimer.start(200);
	qDebug() << QString("Resolving pool %1").arg(pool);
	QHostInfo::lookupHost(pool, this, SLOT(sendRequest(QHostInfo)));
}

void NtpClient::sendRequest(QHostInfo host)
{
	if (host.addresses().isEmpty())
	{
		qDebug() << QString("NTP pool %1 does not resolve.").arg(pool);
		return;
	}
	qDebug() << QString("Quering NTP server %1...").arg(host.addresses().first().toString());
	
	socket.bind(123);
	socket.connectToHost(host.addresses().first(), 123);
	socket.waitForConnected(1000);
	
	NtpHeader ntpHeader;
	ntpHeader.mode = 3; // Client
	ntpHeader.versionNumber = 4; // Ntp v4
	if (socket.write(reinterpret_cast<char *>(&ntpHeader), sizeof(ntpHeader)) < 0)
		qDebug() << "Error sending NTP packet";
	requestReplyTime.start();
}

void NtpClient::readPendingDatagrams()
{
	while (socket.hasPendingDatagrams())
	{
		NtpHeader ntpHeader;
		if (socket.readDatagram(reinterpret_cast<char *>(&ntpHeader), sizeof(ntpHeader)) < sizeof(ntpHeader))
			continue;		
		msCurrentToNtp = QDateTime::currentDateTimeUtc().msecsTo(ntpHeader.transmit.dateTime()) + requestReplyTime.elapsed() / 2;
		retryTimer.stop();
		qDebug() << QString("Current UTC time is %1, off by %2 ms from current system time").arg(utcTime().toString("hh:mm:ss.zzz")).arg(msCurrentToNtp);
		emit utcTimeFound(utcTime());
	}
}
