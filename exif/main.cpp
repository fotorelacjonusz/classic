#include <exiv2/exiv2.hpp>
#include <cstdio>
#include "license.h"

using namespace Exiv2;

double dmsToDouble(const Exifdatum &dms, const Exifdatum &ref)
{
	char c = ref.toString(0)[0];
	double d = dms.toRational(0).first / (double)dms.toRational(0).second;
	double m = dms.toRational(1).first / (double)dms.toRational(1).second;
	double s = dms.toRational(2).first / (double)dms.toRational(2).second;
	return ((m * 60 + s) / 3600 + d) * ((c == 'S' || c == 'W') ? -1 : 1);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("%s", LICENSE);
		return 1;
	}

	FILE *f = fopen(argv[1], "r");
	if (!f)
	{
		printf("No such file: %s\n", argv[1]);
		return 1;
	}
	fclose(f);

#ifdef Q_WS_WIN
	std::wstring file(argv[1]);
#else
	std::string file(argv[1]);
#endif

	try
	{
		Image::AutoPtr image = ImageFactory::open(file);
		if (!image.get())
		{
			printf("exiv2 library is unable to read the image.\n");
			return 1;
		}
		image->readMetadata();

		ExifData &exifData = image->exifData();
		if (exifData.empty())
			return 1;

		ExifData::iterator gpsLat = exifData.findKey(ExifKey("Exif.GPSInfo.GPSLatitude"));
		ExifData::iterator gpsLatRef = exifData.findKey(ExifKey("Exif.GPSInfo.GPSLatitudeRef"));
		ExifData::iterator gpsLon = exifData.findKey(ExifKey("Exif.GPSInfo.GPSLongitude"));
		ExifData::iterator gpsLonRef = exifData.findKey(ExifKey("Exif.GPSInfo.GPSLongitudeRef"));
		if (gpsLat != exifData.end() && gpsLatRef != exifData.end() &&
			gpsLon != exifData.end() && gpsLonRef != exifData.end())
		{
			double latitude = dmsToDouble(*gpsLat, *gpsLatRef);
			double longitude = dmsToDouble(*gpsLon, *gpsLonRef);
			printf("%.8f,%.8f", latitude, longitude);
		}

		ExifData::iterator gpsDirection = exifData.findKey(ExifKey("Exif.GPSInfo.GPSImgDirection"));
		if (gpsDirection != exifData.end())
		{
			double direction = (*gpsDirection).toRational(0).first / (double)(*gpsDirection).toRational(0).second;
			printf(",%f", direction);
		}
		printf("\n");
	}
	catch (Error &e)
	{
		printf("Exception caught from exiv2 library: %s\n", e.what());
		return 1;
	}
	catch (...)
	{
		printf("Unknown exception caught from exiv2 library.\n");
		return 1;
	}

	return 0;
}
