#ifndef OSMLAYERDIALOG_H
#define OSMLAYERDIALOG_H

#include <QDialog>
#include <QSettings>

class QComboBox;
class QSlider;

namespace Ui {
class OSMLayerDialog;
class SettingsDialog;
}

struct OSMLayer
{
	QString name, urlPattern;
	int maxZoom;
	bool operator == (const OSMLayer &other);
};
Q_DECLARE_METATYPE(OSMLayer)

class OSMLayerDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit OSMLayerDialog(QSettings &settings, QWidget *parent = 0);
	~OSMLayerDialog();
	void init(Ui::SettingsDialog *settingsUi);
	QString currentUrlPattern(bool common) const;
	
protected slots:
	void comboBox2Changed(int currentIndex);
	void removeOSMLayerClicked();
	
	void accept();
	
private:	
	Ui::OSMLayerDialog *ui;
	QSettings &settings;
	QComboBox *comboBox1, *comboBox2;
	QPushButton *removeButton;
	QSlider *slider;
	int sliderMaxBak;
	
	QList<OSMLayer> layers;
};

#endif // OSMLAYERDIALOG_H
