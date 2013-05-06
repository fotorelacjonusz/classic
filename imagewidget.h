#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QBuffer>

#include "abstractimage.h"
#include "settingsdialog.h"
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

class ImageWidget : public SelectableWidget<ImageWidget>, public AbstractImage // public QWidget, public AbstractImage
{
	Q_OBJECT

public:
	explicit ImageWidget(QWidget *parent, QString _filePath, QDataStream *stream = 0) throw (Exception);
	~ImageWidget();
	void serialize(QDataStream &stream) const;
	int getNumber() const;
	QString getFileName() const;
	QWidget *getFirstWidget() const;
	QWidget *getLastWidget() const;
	void rotate(bool left);
	bool upload(AbstractUploader *uploader);
	QString toForumCode() const;
	QPixmap sourcePixmap() const;
	QPixmap scaledSourcePixmap() const;
	QByteArray scaledSourceFile() const;

public slots:
	void updatePixmap(bool makeCache = false);
	void updateNumber(int now = -1);
	void updateLayout();
	void unselectEvent();

	void setBrightness(int value);
	void setContrast(int value);
	void setGamma(int value);
	int getBrightness() const;
	int getContrast() const;
	int getGamma() const;

signals:
//	void selected(ImageWidget *widget);
//	void selected(ArrowWidget *widget);

protected:
//	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

private slots:
	void mapDownloaded(QPixmap map);

private:
	static QRect alignedRect(QSize size, QRect outerRect, SettingsDialog::Corner corner, int margin);
	static const qreal maxAspectRatio;
//	static int currentId;
//	const int id;

	QString filePath;
	int num;

	QGridLayout *gridLayout;
	QLabel *numberLabel;
	QLineEdit *captionEdit;
	ImageLabel *imageLabel;

	int brightness, contrast, gamma;
	QByteArray sourceFile;
	QPixmap gpsMap;
	GpsData *gpsData;
};


#endif // IMAGEWIDGET_H
