#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QUrl>
#include <QNetworkProxy>
#include "settingsmanager.h"
#include "uploaderfactory.h"

#define SETTINGS SettingsDialog::object()
#define SSC_HOST "http://www.skyscrapercity.com"

class AbstractUploader;
class Overlay;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog, public SettingsManager
{
	Q_OBJECT

public:
	enum Corner { BottomRight, BottomLeft, TopRight, TopLeft };
	enum MapType { Roadmap, Satellite, Hybrid, Terrain };
	Q_ENUMS(MapType)

	explicit SettingsDialog(QWidget *parent, QSettings &m_settings);
	~SettingsDialog();
	static void copyDescriptions(QWidget *parent);

	Field<AbstractUploader *> uploader;
	Field<QUrl> homeUrl;
	Field<bool> captionsUnder;
	Field<bool> extraSpace;
	Field<bool> numberImages;
	Field<int> startingNumber;
	Field<bool> addImageBorder;
	Field<bool> addTBC;
	Field<int> imagesPerPost;
	Field<int> postSpace;
	Field<QString> extraTags;

	Field<bool> setImageWidth;
	Field<int> imageLength;
	Field<bool> retainOriginalSize;
	Field<int> jpgQuality;
	Field<bool> dontScalePanoramas;
	Field<bool> addLogo;

	Field<QPixmap> logoPixmap;
	Field<Corner> logoCorner;
	Field<int> logoMargin;
	Field<bool> logoInvert;
	Field<bool> addCommonMap;

	Field<QString> commonMapType;
	Field<bool> addImageMap;
	Field<QString> imageMapType;
	Field<QColor> imageMapColor;
	Field<qreal> imageMapOpacity;
	Field<int> imageMapZoom;
	Field<bool> imageMapCircle;
	Field<Corner> imageMapCorner;
	Field<int> imageMapMargin;
	Field<int> imageMapSize;

	Field<bool> useOverlays;
	
	Field<bool> useProxy;
	Field<QString> proxyHost;
	Field<int> proxyPort;
	Field<QString> proxyUser;
	Field<QString> proxyPass;

	QPixmap overlayMakeMap(qreal lon, qreal lat) const;
	static SettingsDialog *object();
	QSettings &settings();

signals:
	void pixmapOptionsChanged();
	void numberOptionsChanged();
	void layoutOptionsChanged();
	void commonMapOptionsChanged();
	void imageMapOptionsChanged();

protected:
	AbstractUploader *uploaderFunc() const;
	bool captionsUnderFunc() const;
	bool setImageWidthFunc() const;
	int imageLengthFunc() const;
	Corner logoCornerFunc() const;
	QString commonMapTypeFunc() const;
	QString imageMapTypeFunc() const;
	qreal imageMapOpacityFunc() const;
	Corner imageMapCornerFunc() const;
	QUrl homeUrlFunc() const;

protected slots:
	void accept();
	void reject();

private slots:
	void on_logoLoad_clicked();
	void on_uploadMethodComboBox_currentIndexChanged(int index);
	void on_imageMapColor_clicked();
	void proxyOptionsChanged();
	
private:
	Ui::SettingsDialog *ui;
	AbstractUploader *m_uploader;

	QSettings &m_settings;

	QPixmap logo;
	QColor m_imageMapColor;
	QString tosFormat;
	UploaderFactory factory;

	static SettingsDialog *objectInstance;
	
	QNetworkProxy proxy;
};

#endif // SETTINGSDIALOG_H
