#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QUrl>
#include <QNetworkProxy>
#include "settingsmanager.h"
#include "uploaderfactory.h"
#include "osmlayerdialog.h"

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
	enum MapType { Roadmap, Satellite, Hybrid, Terrain, OSMFirst };
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

	Field<MapType> commonMapType;
	Field<bool> addImageMap;
	Field<MapType> imageMapType;
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
	QString mapTypeToString(MapType type) const;
	QString currentOsmUrlPattern(bool common) const;

signals:
	void pixmapOptionsChanged();
	void numberOptionsChanged();
	void layoutOptionsChanged();
	void commonMapOptionsChanged();
	void imageMapOptionsChanged();

protected:
	inline AbstractUploader *uploaderFunc() const;
	inline bool captionsUnderFunc() const;
	inline bool setImageWidthFunc() const;
	inline int imageLengthFunc() const;
	inline Corner logoCornerFunc() const;
	inline MapType commonMapTypeFunc() const;
	inline MapType imageMapTypeFunc() const;
	inline qreal imageMapOpacityFunc() const;
	inline Corner imageMapCornerFunc() const;
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
	OSMLayerDialog osmDialog;
};

#endif // SETTINGSDIALOG_H
