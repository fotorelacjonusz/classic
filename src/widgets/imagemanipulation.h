#pragma once

#include <QImage>
#include "math.h"

#define BRIGHTNESS_MIN -100
#define BRIGHTNESS_DEFAULT 0
#define BRIGHTNESS_MAX 100

#define CONTRAST_MIN 0
#define CONTRAST_DEFAULT 100
#define CONTRAST_MAX 200

#define GAMMA_MIN 0
#define GAMMA_DEFAULT 100
#define GAMMA_MAX 200

class ImageManipulation
{
public:
	static void changeBrightness(QImage &image, int brightness)
	{
		if (brightness == BRIGHTNESS_DEFAULT)
			return;
		changeImage<changeBrightness>(image, brightness);
	}

	static void changeContrast(QImage &image, int contrast)
	{
		if (contrast == CONTRAST_DEFAULT)
			return;
		changeImage<changeContrast>(image, contrast);
	}

	static void changeGamma(QImage &image, int gamma)
	{
		if (gamma == GAMMA_DEFAULT)
			return;
		changeImage<changeGamma>(image, gamma);
	}

private:
	static inline int changeBrightness(int value, int brightness)
	{
		return qBound(0, value + brightness * 255 / 100, 255);
	}

	static inline int changeContrast(int value, int contrast)
	{
		return qBound(0, ((value - 127) * contrast / 100) + 127, 255);
	}

	static inline int changeGamma(int value, int gamma)
	{
		return qBound(0, int(pow(value / 255.0, 100.0 / gamma) * 255), 255);
	}

	static inline int changeUsingTable(int value, const int table[])
	{
		return table[value];
	}

	template<int operation(int, int)>
	static void changeImage(QImage &im, int value)
	{
		if (im.colorCount() == 0)
		{
			if (im.format() != QImage::Format_RGB32)
				im = im.convertToFormat(QImage::Format_RGB32);
			int table[256];
			for (int i = 0; i < 256; ++i)
				table[i] = operation(i, value);
			if (im.hasAlphaChannel())
			{
				for (int y = 0; y < im.height(); ++y)
				{
					QRgb *line = reinterpret_cast<QRgb *>(im.scanLine(y));
					for (int x = 0; x < im.width(); ++x)
						line[x] = qRgba(changeUsingTable(qRed(line[x]), table),
										changeUsingTable(qGreen(line[x]), table),
										changeUsingTable(qBlue(line[x]), table),
										changeUsingTable(qAlpha(line[x]), table));
				}
			}
			else
			{
				for (int y = 0; y < im.height(); ++y)
				{
					QRgb *line = reinterpret_cast<QRgb *>(im.scanLine(y));
					for (int x = 0; x < im.width(); ++x)
						line[x] = qRgb(changeUsingTable(qRed(line[x]), table),
									   changeUsingTable(qGreen(line[x]), table),
									   changeUsingTable(qBlue(line[x]), table));
				}
			}
		}
		else
		{
			QVector<QRgb> colors = im.colorTable();
			for (int i = 0; i < im.colorCount(); ++i)
				colors[i] = qRgb(operation(qRed(colors[i]), value),
								 operation(qGreen(colors[i]), value),
								 operation(qBlue(colors[i]), value));
		}
	}
};
