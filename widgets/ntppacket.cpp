#include "ntppacket.h"
#include <QtEndian>

// ntpEpoch = 1 Jan 1900
// qtEpoch = 1 Jan 1970

const qint64 msFromNtpToQt = 2208988800000ll;

QDateTime NtpTimestamp::dateTime() const
{
	quint32 secondsLE = qFromBigEndian(seconds);
	quint32 fractionLE = qFromBigEndian(fraction);
	qint64 msNtp = secondsLE * 1000ll + fractionLE * 1000ll / 0x100000000ll;
	return QDateTime::fromMSecsSinceEpoch(msNtp - msFromNtpToQt);
}

void NtpTimestamp::setDateTime(QDateTime dateTime)
{
	qint64 msNtp = dateTime.toMSecsSinceEpoch() + msFromNtpToQt;
	seconds = qFromLittleEndian(msNtp / 1000);
	fraction = qFromLittleEndian((msNtp % 1000) * 0x100000000ll / 1000);
}

NtpHeader::NtpHeader()
{
	qMemSet(reinterpret_cast<void *>(this), 0, sizeof(NtpHeader));
}
