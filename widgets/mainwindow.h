#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "settings/settingsdialog.h"
#include "settings/settingsmanager.h"
#include "exception.h"
#include "downloaders/gpsdata.h"
#include "recentthreadsmenu.h"

class AbstractImage;
class ImageWidget;
class ArrowWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_action_open_photorelation_triggered();
	void on_action_save_photorelation_triggered();
	void on_action_add_photos_triggered();
	void on_action_settings_triggered();
	void on_action_send_to_SSC_triggered();

	void on_action_about_triggered();
	void on_action_Qt_information_triggered();

	void moveImage(int number);
	void on_action_move_top_triggered();
	void on_action_move_up_triggered();
	void on_action_move_down_triggered();
	void on_action_move_bottom_triggered();

	void on_action_rotate_left_triggered();
	void on_action_rotate_right_triggered();
	void on_action_remove_image_triggered();

	void on_action_invert_colors_triggered();
	void on_action_choose_color_triggered();
	void on_action_remove_arrow_triggered();

	void imageWidgetSelected(QWidget *widget);
	void arrowWidgetSelected(QWidget *widget);

	void updateCommonMap();
	void commonMapReady(QImage map);

protected:
	void processEvents() const;
	ImageWidget *newImage(QString filePath, QDataStream *stream) throw(Exception);
	ImageWidget *imageAt(int i) const;
	QList<AbstractImage *> imageList() const;
	void closeEvent(QCloseEvent *event);
	bool isImageExtension(QString filePath);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private:
	Ui::MainWindow *ui;
	QSettings settings;
	SettingsManager manager;
	SettingsDialog settingsDialog;
	ImageWidget *selectedImage;
	ArrowWidget *selectedArrow;
	QString dirName;
	static QByteArray phrFileHeader;
	GpsData gpsData;
	RecentThreadsMenu recentThreads;
};

#endif // MAINWINDOW_H
