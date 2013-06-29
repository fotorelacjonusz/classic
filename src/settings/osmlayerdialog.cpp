#include "osmlayerdialog.h"
#include "ui_osmlayerdialog.h"
#include "ui_settingsdialog.h"
#include "downloaders/tilesdownloader.h"
#include "settings/settingsdialog.h"

#include <QComboBox>
#include <QPushButton>
#include <QInputDialog>
#include <QSettings>

bool OSMLayer::operator ==(const OSMLayer &other)
{
	return name == other.name && urlPattern == other.urlPattern && maxZoom == other.maxZoom;
}

OSMLayerDialog::OSMLayerDialog(QSettings &settings, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::OSMLayerDialog),
	settings(settings),
	comboBox1(0), comboBox2(0), removeButton(0), slider(0)
{
	ui->setupUi(this);
	int size = settings.beginReadArray("osm_layers");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		OSMLayer layer;
		layer.name = settings.value("name").toString();
		layer.urlPattern = settings.value("url").toString();
		layer.maxZoom = settings.value("max_zoom").toInt();
		layers.append(layer);
	}
	settings.endArray();
}

OSMLayerDialog::~OSMLayerDialog()
{
	settings.beginWriteArray("osm_layers");
	for (int i = 0; i < layers.size(); ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("name", layers[i].name);
		settings.setValue("url", layers[i].urlPattern);
		settings.setValue("max_zoom", layers[i].maxZoom);
	}
	settings.endArray();
	delete ui;
}

void OSMLayerDialog::init(Ui::SettingsDialog *settingsUi)
{
	comboBox1 = settingsUi->commonMapType;
	comboBox2 = settingsUi->imageMapType;
	removeButton = settingsUi->removeOSMLayer;
	slider = settingsUi->imageMapZoom;
	sliderMaxBak = slider->maximum();
	connect(comboBox2, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox2Changed(int)));
	connect(settingsUi->addOSMLayer, SIGNAL(clicked()), this, SLOT(exec()));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeOSMLayerClicked()));
	foreach	(OSMLayer layer, layers)
	{
		comboBox1->addItem(layer.name, QVariant::fromValue(layer));
		comboBox2->addItem(layer.name, QVariant::fromValue(layer));
	}
	removeButton->setEnabled(!layers.isEmpty());
}

OSMLayer OSMLayerDialog::currentLayer(bool common) const
{
	QComboBox *box = common ? comboBox1 : comboBox2;
	QVariant data = box->itemData(box->currentIndex());
	Q_ASSERT(data.canConvert<OSMLayer>());
	return data.value<OSMLayer>();
}

void OSMLayerDialog::comboBox2Changed(int currentIndex)
{
	QVariant data = comboBox2->itemData(currentIndex);
	slider->setMaximum(data.canConvert<OSMLayer>() ? data.value<OSMLayer>().maxZoom : sliderMaxBak);
}

void OSMLayerDialog::removeOSMLayerClicked()
{
	QStringList items;
	foreach (OSMLayer layer, layers)
		items.append(layer.name);
	bool ok;
	QString name = QInputDialog::getItem(this, tr("Którą warstwę usunąć?"), tr("Warstwy OSM"), items, 0, false, &ok, Qt::Dialog);
	if (ok)
		for (int i = 0; i < layers.size(); ++i)
			if (layers[i].name == name)
			{
				comboBox1->removeItem(i + SettingsDialog::OSMFirst);
				comboBox2->removeItem(i + SettingsDialog::OSMFirst);
				layers.removeAt(i);
			}
	removeButton->setEnabled(!layers.isEmpty());
}

void OSMLayerDialog::accept()
{
	if (TilesDownloader::validateUrlPattern(this, ui->url->text()))
	{
		OSMLayer layer;
		layer.name = ui->name->text();
		layer.urlPattern = ui->url->text();
		layer.maxZoom = ui->maxZoom->value();
		layers.append(layer);
		comboBox1->addItem(layer.name, QVariant::fromValue(layer));
		comboBox2->addItem(layer.name, QVariant::fromValue(layer));
		removeButton->setEnabled(true);
		QDialog::accept();
	}
}

