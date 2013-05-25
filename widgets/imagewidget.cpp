#include "imagewidget.h"
#include "settings/settingsdialog.h"
#include "uploaders/abstractuploader.h"

#include "arrowwidget.h"
#include "imagelabel.h"
#include "downloaders/gpsdata.h"
#include "imagemanipulation.h"

#define Q_TEST_QPIXMAPCACHE

#include <QPixmapCache>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QBuffer>
#include <QDir>
#include <QCryptographicHash>
#include <QCheckBox>
#include <QTime>

ImageWidget::ImageWidget(QWidget *parent, QString _filePath, QDataStream *stream) throw(Exception):
	SelectableWidget<ImageWidget>(parent),
	filePath(_filePath),
	m_brightness(BRIGHTNESS_DEFAULT), m_contrast(CONTRAST_DEFAULT), m_gamma(GAMMA_DEFAULT),
	gpsData(0)
{
	numberLabel = new QLabel(this);
	captionEdit = lineEdit;
	imageLabel = new ImageLabel(this, captionEdit);

	gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(1);
	gridLayout->setContentsMargins(3, 3, 3, 3);

	if (stream)
	{
		QString caption;
		*stream >> filePath >> num >> caption >> sourceFile >> *imageLabel;
		captionEdit->setText(caption);
	}
	else
	{
		num = parent->layout()->count();
		QFile file(filePath);
		file.open(QIODevice::ReadOnly);
		int ba = file.bytesAvailable();
		sourceFile = file.readAll();
		file.close();
		if (sourceFile.size() != ba)
			THROW(tr("Nie można załadować zdjęcia do pamięci. Pamięć wyczerpana?"));
	}
	
	QBuffer buff(&sourceFile);
	buff.open(QIODevice::ReadOnly);
	gpsData = new GpsData(&buff, &num);
	buff.close();

	connect(gpsData, SIGNAL(mapReady(QImage)), this, SLOT(mapDownloaded(QImage)));
	
	SETTINGS->connectMany(this, SLOT(imageSizeChanged()), &SETTINGS->setImageWidth, &SETTINGS->imageLength, &SETTINGS->dontScalePanoramas);
	SETTINGS->connectMany(this, SLOT(updateNumber()), &SETTINGS->numberImages, &SETTINGS->startingNumber);
	SETTINGS->connectMany(this, SLOT(updateLayout()), &SETTINGS->captionsUnder, &SETTINGS->extraSpace);
	SETTINGS->connectMany(this, SLOT(updatePixmap()), &SETTINGS->addImageBorder, &SETTINGS->addLogo, &SETTINGS->logoPixmap, 
						  &SETTINGS->logoCorner, &SETTINGS->logoMargin, &SETTINGS->logoInvert);

	setFocusPolicy(Qt::NoFocus);

	imageLabel->setToolTip(filePath);
	imageLabel->setStatusTip(imageLabel->toolTip());
	captionEdit->setToolTip(tr("Podpis zdjęcia"));
	captionEdit->setStatusTip(captionEdit->toolTip());
	captionEdit->setAcceptDrops(false);

	updatePixmap();
	updateNumber();
	updateLayout();
	
//	if (SETTINGS->addImageMap)
//		gpsData->downloadMap();
}

ImageWidget::~ImageWidget()
{
	delete gpsData;
}

QString ImageWidget::fileName() const
{
	return filePath; //.section(QDir::separator(), -1);
}

QString ImageWidget::caption() const
{
	QString caption = captionEdit->text();
	if (SETTINGS->numberImages)
		caption.prepend(numberLabel->text());
	return caption;
}

int ImageWidget::number() const
{
	return num;
}

void ImageWidget::serialize(QDataStream &stream) const
{
	QBuffer buffer; //(&sourceFile);
	buffer.buffer() = sourceFile;
	buffer.open(QIODevice::ReadWrite);
	gpsData->writeExif(&buffer);
	buffer.close();
	
	stream << filePath << num << captionEdit->text() << buffer.buffer() << *imageLabel;
}

void ImageWidget::write(QIODevice *device) const
{
//	captionEdit->setFocus();

	imageLabel->write(device);
	device->seek(0);
	gpsData->writeExif(device);
}

QWidget *ImageWidget::firstWidget() const
{
	return captionEdit;
}

QWidget *ImageWidget::lastWidget() const
{
	QWidget *widget = imageLabel->lastArrow();
	return widget ? widget : captionEdit;
}

void ImageWidget::rotate(bool left)
{
	QPixmap pixmap = sourcePixmap().transformed(QMatrix().rotate(left ? -90 : 90));
	QBuffer buffer(&sourceFile);
	buffer.open(QIODevice::WriteOnly);
	pixmap.save(&buffer, "JPG");
	buffer.close();
	QPixmapCache::remove(filePath);
	if (QFile::exists(filePath) && QMessageBox::question(this, tr("Obrót zdjęcia"), tr("Czy obrócić również oryginalne zdjęcie na dysku?"), 
														 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		pixmap.save(filePath, "JPG");
	updatePixmap();
}

QPixmap ImageWidget::sourcePixmap() const
{
	QPixmap pixmap;
	if (!pixmap.loadFromData(sourceFile))
		THROW(tr("Nie można załadować zdjęcia. Pamięć wyczerpana?"));
//	if (!pixmap.loadFromData(sourceFile))
//		return QPixmap();

	return pixmap;
}

QPixmap ImageWidget::scaledSourcePixmap() const
{
	QPixmap pixmap = sourcePixmap();
	if (SETTINGS->dontScalePanoramas && isPanoramic(pixmap.size()))
		return pixmap;
	if (SETTINGS->setImageWidth)
	{
		if (pixmap.width() <= SETTINGS->imageLength)
			return pixmap;
		return pixmap.scaledToWidth(SETTINGS->imageLength, Qt::SmoothTransformation);
	}
	else
	{
		if (pixmap.width() <= SETTINGS->imageLength && pixmap.height() <= SETTINGS->imageLength)
			return pixmap;
		return pixmap.scaled(SETTINGS->imageLength, SETTINGS->imageLength, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
}

QByteArray ImageWidget::scaledSourceFile() const
{
	QByteArray byteArray;
	QBuffer buffer(&byteArray);
	buffer.open(QIODevice::WriteOnly);
	if (!scaledSourcePixmap().save(&buffer, "JPG", SETTINGS->jpgQuality))
		return QByteArray();

	buffer.close();
	return byteArray;
}

void ImageWidget::imageSizeChanged()
{
	QPixmapCache::remove(filePath);
	updatePixmap();
}

void ImageWidget::updatePixmap()
{
//	QTime time;
//	time.start();
//	qDebug() << objectName() << "start";

	QPixmap photo;
	if (!QPixmapCache::find(filePath, &photo))
	{
		photo = scaledSourcePixmap();
		gpsData->setPhotoSize(photo.size());
		if (photo.isNull())
			THROW(tr("Nie można utworzyć pixmapy ze zdjęcia. Pamięć wyczerpana?"));

		QPixmapCache::insert(filePath, photo);
//		qDebug() << "after makecache" << time.elapsed();
	}
	
	if (m_brightness != BRIGHTNESS_DEFAULT || m_contrast != CONTRAST_DEFAULT || m_gamma != GAMMA_DEFAULT)
	{
		QImage img = photo.toImage();
		ImageManipulation::changeBrightness(img, m_brightness);
		ImageManipulation::changeContrast(img, m_contrast);
		ImageManipulation::changeGamma(img, m_gamma);
		photo = QPixmap::fromImage(img);
//		qDebug() << "after color reg" << time.elapsed();
	}

	if (SETTINGS->addLogo)
	{
		QImage logo = SettingsDialog::object()->logoPixmap.v().toImage();
		QRect logoRect = alignedRectangle(logo.size(), photo.rect(), SETTINGS->logoCorner, SETTINGS->logoMargin);

		if (SETTINGS->logoInvert)
		{
			QImage image = photo.toImage();
			qreal colorSum = 0;
			int pixelCount = 0;
			QRgb p;
			for (int x = logoRect.left(); x <= logoRect.right(); x += 4)
				for (int y = logoRect.top(); y <= logoRect.bottom(); y += 4, ++pixelCount, colorSum += qRed(p) + qGreen(p) + qBlue(p))
					p = image.pixel(x, y);

			qreal colorAvg = colorSum / pixelCount / 3;
			if (colorAvg < 128.0)
				logo.invertPixels();
		}
		QPainter painter(&photo);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawImage(logoRect, logo);
//		qDebug() << "after add logo" << time.elapsed();
	}

	if (SETTINGS->addImageMap && !gpsMap.isNull())
	{
		if (SETTINGS->imageMapCorner >= SettingsDialog::Expand)
		{
			QRect photoRect = photo.rect();
			QRect mapRect = gpsMap.rect();
			QRect expandedRect = expandedRectangle(photoRect, mapRect, SETTINGS->imageMapCorner, 0); // SETTINGS->imageMapMargin
			QPixmap expanded(expandedRect.size());
//			expanded.fill(Qt::white);
			
			QPainter painter(&expanded);
			painter.drawPixmap(photoRect, photo);
			painter.drawImage(mapRect, gpsMap);
			painter.end();
			photo = expanded;
		} 
		else
		{
			QRect mapRect = alignedRectangle(gpsMap.size(), photo.rect(), SETTINGS->imageMapCorner, SETTINGS->imageMapMargin);
			QPainter painter(&photo);
			painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
			painter.setOpacity(SETTINGS->imageMapOpacity);
			painter.drawImage(mapRect, gpsMap);
		}
//		qDebug() << "after add map" << time.elapsed();
	}

	if (SETTINGS->addImageBorder)
	{
		QPainter painter(&photo);
		painter.setPen(QPen(Qt::black, 3));
		painter.drawRect(photo.rect().adjusted(1, 1, -2, -2));
//		qDebug() << "after add border" << time.elapsed();
	}

//	qDebug() << "after conditions" << time.elapsed();
	imageLabel->setPixmap(photo);
	setMaximumWidth(photo.width() + 6);

	m_url = QString();
//	qDebug() << objectName() << time.elapsed() << "total" << QPixmapCache::totalUsed();
}

void ImageWidget::updateNumber(int now)
{
	if (now >= 0)
		num = now;
	numberLabel->setVisible(SETTINGS->numberImages);
	numberLabel->setText(QString("%1. ").arg(num + 1 + SETTINGS->startingNumber - 1));
}

void ImageWidget::updateLayout()
{
	gridLayout->removeWidget(numberLabel);
	gridLayout->removeWidget(captionEdit);
	gridLayout->removeWidget(imageLabel);

	gridLayout->addWidget(numberLabel, SETTINGS->captionsUnder, 0, 1, 1);
	gridLayout->addWidget(captionEdit, SETTINGS->captionsUnder, 1, 1, 1);
	gridLayout->addWidget(imageLabel, !SETTINGS->captionsUnder, 0, 1, 2);

	gridLayout->setVerticalSpacing(SETTINGS->extraSpace ? 20 : 1);
}

void ImageWidget::setBrightness(int value)
{
	m_brightness = value;
	updatePixmap();
}

void ImageWidget::setContrast(int value)
{
	m_contrast = value;
	updatePixmap();
}

void ImageWidget::setGamma(int value)
{
	m_gamma = value;
	updatePixmap();
}

int ImageWidget::brightness() const
{
	return m_brightness;
}

int ImageWidget::contrast() const
{
	return m_contrast;
}

int ImageWidget::gamma() const
{
	return m_gamma;
}

bool ImageWidget::isPanoramic(QSize size)
{
	static const qreal maxAspectRatio = 17.0 / 9.0;
	return size.width() / (qreal)size.height() > maxAspectRatio;
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ImageWidget::mapDownloaded(QImage map)
{
	gpsMap = map;
	updatePixmap();
}

QRect ImageWidget::alignedRectangle(QSize size, QRect outerRect, SettingsDialog::Corner corner, int margin)
{
	QRect rect(QPoint(0, 0), size);
	outerRect.adjust(margin, margin, -margin, -margin);
	switch (corner)
	{
		case SettingsDialog::BottomRight: rect.moveBottomRight(outerRect.bottomRight()); break;
		case SettingsDialog::BottomLeft:  rect.moveBottomLeft (outerRect.bottomLeft());  break;
		case SettingsDialog::TopRight:    rect.moveTopRight   (outerRect.topRight());    break;
		case SettingsDialog::TopLeft:     rect.moveTopLeft    (outerRect.topLeft());     break;
		default: Q_ASSERT(false);
	}
	return rect;
}

QRect ImageWidget::expandedRectangle(QRect &photo, QRect &map, SettingsDialog::Corner corner, int margin)
{
	switch (corner)
	{
		case SettingsDialog::ExpandLeft:   photo.moveLeft(map.width() + margin); break;
		case SettingsDialog::ExpandRight:  map.moveLeft(photo.width() + margin); break;
		case SettingsDialog::ExpandTop:    photo.moveTop(map.height() + margin); break;
		case SettingsDialog::ExpandBottom: map.moveTop(photo.height() + margin); break;
		default: Q_ASSERT(false);
	}
	return photo | map;
}
