#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QBuffer>

#include "abstractimage.h"
#include "settings/settingsdialog.h"
#include "exception.h"
#include "selectablewidget.h"

class QGridLayout;
class QLabel;
class QLineEdit;
class ArrowWidget;
class ImageLabel;
class AbstractUploader;
class GpsData;
class QCheckBox;
class GpxDialog;

class ImageWidget : public SelectableWidget<ImageWidget>, public AbstractImage
{
	Q_OBJECT

public:
	explicit ImageWidget(QWidget *parent, QString _filePath, QDataStream *stream = 0) throw (Exception);
	~ImageWidget();

	virtual QString fileName() const;
	virtual QString caption() const;
	virtual int number() const;
	virtual void serialize(QDataStream &stream) const;
	virtual	void write(QIODevice *device) const;

	void setPosition(GpxDialog *gpxDialog);
	void removePosition();

	QWidget *firstWidget() const;
	QWidget *lastWidget() const;
	void rotate(bool left);

	QPixmap sourcePixmap() const;
	QPixmap scaledSourcePixmap() const;
	QByteArray scaledSourceFile() const;

public slots:
	void imageSizeChanged();
	void updatePixmap();
	void updateNumber(int now = -1);
	void updateLayout();
//	void unselectEvent();

	void setBrightness(int value);
	void setContrast(int value);
	void setGamma(int value);
	int brightness() const;
	int contrast() const;
	int gamma() const;

	static bool isPanoramic(QSize size);

protected:
	void paintEvent(QPaintEvent *event);

private slots:
	void mapDownloaded(QImage map);

private:
	static QRect alignedRectangle(QSize size, QRect outerRect, SettingsDialog::Corner corner, int margin);
	static QRect expandedRectangle(QRect &photo, QRect &map, SettingsDialog::Corner corner, int margin);

	QString filePath;
	int num;

	QGridLayout *gridLayout;
	QLabel *numberLabel;
	QLineEdit *captionEdit;
	ImageLabel *imageLabel;

	int m_brightness, m_contrast, m_gamma;
	QByteArray sourceFile;
	QImage gpsMap;
	GpsData *gpsData;
};

#endif // IMAGEWIDGET_H
