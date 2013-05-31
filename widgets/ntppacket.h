#ifndef NTPPACKET_H
#define NTPPACKET_H

#include <QtGlobal>
#include <QDateTime>

#pragma pack(push, 1)

struct NtpShort
{
	quint16 seconds;
	quint16 fraction;
};

struct NtpTimestamp 
{
	quint32 seconds;
	quint32 fraction;
	
	QDateTime dateTime() const;
	void setDateTime(QDateTime dateTime);
};

struct NtpHeader
{
	NtpHeader();
	
	quint8 mode : 3;
	quint8 versionNumber : 3;
	quint8 leapIndicator : 2;
	
	quint8 stratum;
	qint8 pool;
	qint8 precision;
	
	NtpShort rootDelay;
	NtpShort rootDispersion;
	char referenceId[4];
	
	NtpTimestamp reference;
	NtpTimestamp origin;
	NtpTimestamp receive;
	NtpTimestamp transmit;
};

#pragma pack(pop)

#endif // NTPPACKET_H
