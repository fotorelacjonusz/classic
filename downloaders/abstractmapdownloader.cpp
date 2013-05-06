#include "abstractmapdownloader.h"
#include "gpsdata.h"
#include <QMetaType>

AbstractMapDownloader::AbstractMapDownloader()
{
	static bool registered = false;
	if (!registered)
	{
		qRegisterMetaType<Points>("Points");
		registered = true;
	}
}

