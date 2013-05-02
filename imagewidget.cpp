#include "imagewidget.h"
#include "settingsdialog.h"
#include "abstractuploader.h"

#include "arrowwidget.h"
#include "imagelabel.h"
#include "gpsdata.h"
#include "imagemanipulation.h"

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

const qreal ImageWidget::maxAspectRatio = 17.0 / 9.0;
int ImageWidget::currentId = 0;

ImageWidget::ImageWidget(QWidget *parent, QString _filePath, QDataStream *stream) throw(Exception):
	QWidget(parent),
	id(++currentId),
	filePath(_filePath),
	brightness(BRIGHTNESS_DEFAULT), contrast(CONTRAST_DEFAULT), gamma(GAMMA_DEFAULT),
	gpsData(0)
{
	numberLabel = new QLabel(this);
	captionEdit = new QLineEdit(this);
	imageLabel = new ImageLabel(this);

	gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(1);
	gridLayout->setContentsMargins(3, 3, 3, 3);

	if (stream)
	{
		QString caption;
		*stream >> filePath >> num >> caption >> sourceFile >> *imageLabel;
		captionEdit->setText(caption);

		gpsData = new GpsData(&num, *stream);
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

		gpsData = new GpsData(&num, filePath);
	}

	connect(gpsData, SIGNAL(mapReady(QPixmap)), this, SLOT(mapDownloaded(QPixmap)));
	connect(SETTINGS, SIGNAL(imageMapOptionsChanged()), gpsData, SLOT(downloadMap()));
	connect(SETTINGS, SIGNAL(imageMapOptionsChanged()), this, SLOT(updatePixmap()));
	gpsData->downloadMap();

	setObjectName(QString("photo%1").arg(id));

	connect(SETTINGS, SIGNAL(pixmapOptionsChanged()), this, SLOT(updatePixmap()));
	connect(SETTINGS, SIGNAL(numberOptionsChanged()), this, SLOT(updateNumber()));
	connect(SETTINGS, SIGNAL(layoutOptionsChanged()), this, SLOT(updateLayout()));
	connect(imageLabel, SIGNAL(selected(ArrowWidget*)), this, SIGNAL(selected(ArrowWidget*)));

	setFocusPolicy(Qt::NoFocus);

	imageLabel->setToolTip(filePath);
	imageLabel->setStatusTip(imageLabel->toolTip());
	captionEdit->setToolTip(tr("Podpis zdjęcia"));
	captionEdit->setStatusTip(captionEdit->toolTip());
	captionEdit->setAcceptDrops(false);

	updatePixmap();
	updateNumber();
	updateLayout();
}

ImageWidget::~ImageWidget()
{
	delete gpsData;
}

void ImageWidget::serialize(QDataStream &stream) const
{
	stream << filePath << num << captionEdit->text() << (SETTINGS->retainOriginalSize ? sourceFile : scaledSourceFile()) << *imageLabel;
	gpsData->serialize(stream);
}

int ImageWidget::getNumber() const
{
	return num;
}

QString ImageWidget::getFileName() const
{
	return filePath.section(QDir::separator(), -1);
}

QWidget *ImageWidget::getCaptionEdit() const
{
	return captionEdit;
}

void ImageWidget::rotate(bool left)
{
	QPixmap pixmap = sourcePixmap().transformed(QMatrix().rotate(left ? -90 : 90));
	QBuffer buffer(&sourceFile);
	buffer.open(QIODevice::WriteOnly);
	pixmap.save(&buffer, "JPG");
	buffer.close();
	if (QFile::exists(filePath) && QMessageBox::question(this, tr("Obrót zdjęcia"), tr("Czy obrócić również oryginalne zdjęcie na dysku?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		pixmap.save(filePath, "JPG");
	updatePixmap();
}

bool ImageWidget::upload(AbstractUploader *uploader)
{
	if (!url.isEmpty())
		return true;

	captionEdit->setFocus();
	QPixmap pixmap = imageLabel->mergedPixmap();

	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	pixmap.save(&buffer, "JPG", SETTINGS->jpgQuality);
	buffer.close();

	url = uploader->uploadImage(filePath, &buffer);
	return !url.isEmpty();
}

QString ImageWidget::toForumCode() const
{
	QString link = url.isEmpty() ? tr("Upload zdjęcia %1 nieudany.\n").arg(filePath) : QString("[img]%1[/img]\n").arg(url);
	QString caption = captionEdit->text() + "\n";
	if (SETTINGS->numberImages)
		caption.prepend(numberLabel->text());

	QString space = SETTINGS->extraSpace ? "\n" : "";
	QString text = SETTINGS->captionsUnder ? link + space + caption : caption + space + link;
	return appendPrepend(text) + "\n";
}

QPixmap ImageWidget::sourcePixmap() const
{
	QPixmap pixmap;
	if (!pixmap.loadFromData(sourceFile))
		return QPixmap();

	return pixmap;
}

QPixmap ImageWidget::scaledSourcePixmap() const
{
	QPixmap pixmap = sourcePixmap();
	if (SETTINGS->dontScalePanoramas && pixmap.width() / (qreal)pixmap.height() > maxAspectRatio)
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

void ImageWidget::updatePixmap(bool makeCache)
{
	QPixmap photo;
	if (!QPixmapCache::find(filePath, &photo))
	{
		photo = scaledSourcePixmap();
		if (photo.isNull())
			THROW(tr("Nie można utworzyć pixmapy ze zdjęcia. Pamięć wyczerpana?"));

		if (makeCache)
			QPixmapCache::insert(filePath, photo);
	}

	if (brightness != BRIGHTNESS_DEFAULT || contrast != CONTRAST_DEFAULT || gamma != GAMMA_DEFAULT)
	{
		QImage img = photo.toImage();
		ImageManipulation::changeBrightness(img, brightness);
		ImageManipulation::changeContrast(img, contrast);
		ImageManipulation::changeGamma(img, gamma);
		photo = QPixmap::fromImage(img);
	}

	if (SETTINGS->addLogo)
	{
		QImage logo = SettingsDialog::object()->logoPixmap.value().toImage();
		QRect logoRect = alignedRect(logo.size(), photo.rect(), SETTINGS->logoCorner, SETTINGS->logoMargin);

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
	}

	if (SETTINGS->addImageMap && !gpsMap.isNull())
	{
		QRect mapRect = alignedRect(gpsMap.size(), photo.rect(), SETTINGS->imageMapCorner, SETTINGS->imageMapMargin);
		QPainter painter(&photo);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setOpacity(SETTINGS->imageMapOpacity);
		painter.drawPixmap(mapRect, gpsMap);
	}

	if (SETTINGS->addImageBorder)
	{
		QPainter painter(&photo);
		painter.setPen(QPen(Qt::black, 3));
		painter.drawRect(photo.rect().adjusted(1, 1, -2, -2));
	}

	imageLabel->setPixmap(photo);
	setMaximumWidth(photo.width() + 6);

	url = QString();
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

void ImageWidget::unselected()
{
	setStyleSheet(QString("QWidget#%1 { }").arg(objectName()));
	QPixmapCache::remove(filePath);
}

void ImageWidget::setBrightness(int value)
{
	brightness = value;
	updatePixmap(true);
}

void ImageWidget::setContrast(int value)
{
	contrast = value;
	updatePixmap(true);
}

void ImageWidget::setGamma(int value)
{
	gamma = value;
	updatePixmap(true);
}

int ImageWidget::getBrightness() const
{
	return brightness;
}

int ImageWidget::getContrast() const
{
	return contrast;
}

int ImageWidget::getGamma() const
{
	return gamma;
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
	setStyleSheet(QString("QWidget#%1 { background: rgba(20, 80, 200, 50); border: 3px dashed rgba(20, 80, 200, 250) }").arg(objectName()));
	emit selected(this);

	QWidget::mouseReleaseEvent(event);
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ImageWidget::mapDownloaded(QPixmap map)
{
	gpsMap = map;
	updatePixmap();
}

QRect ImageWidget::alignedRect(QSize size, QRect outerRect, SettingsDialog::Corner corner, int margin)
{
	QRect rect(QPoint(0, 0), size);
	outerRect.adjust(margin, margin, -margin, -margin);
	switch (corner)
	{
		case SettingsDialog::BottomRight: rect.moveBottomRight(outerRect.bottomRight()); break;
		case SettingsDialog::BottomLeft:  rect.moveBottomLeft (outerRect.bottomLeft());  break;
		case SettingsDialog::TopRight:    rect.moveTopRight   (outerRect.topRight());    break;
		case SettingsDialog::TopLeft:     rect.moveTopLeft    (outerRect.topLeft());     break;
	}
	return rect;
}





