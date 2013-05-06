#include "settingsmanager.h"
#include "secrets.h"

#include <QSettings>

#include <QButtonGroup>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDial>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimeEdit>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>

SettingsManager::AbstractInput::AbstractInput(QString name, QObject *receiver, const char *member, QVariant defaultVal):
	key(name), receiver(receiver), member(member), defaultVal(defaultVal),
	pasKey(QByteArray::fromHex(PASSWORD_RAW_KEY))
{
}

SettingsManager::AbstractInput::~AbstractInput()
{
}

QByteArray SettingsManager::AbstractInput::encode(QString pass) const
{
	return qCompress(pass.toAscii()) ^ pasKey;
}

QString SettingsManager::AbstractInput::decode(QByteArray pass) const
{
	return pass.isEmpty() ? "" : qUncompress(pass ^ pasKey);
}

SettingsManager::SettingsManager(QSettings &settings):
	settings(settings)
{
}

SettingsManager::~SettingsManager()
{
	qDeleteAll(inputs);
}

void SettingsManager::load(AbstractInput *singleInput)
{
	foreach (AbstractInput *input, singleInput ? QList<AbstractInput *>() << singleInput : inputs)
		input->fromVariant(settings.value(input->key, input->defaultVal));
}

void SettingsManager::save(AbstractInput *singleInput)
{
	typedef QPair<QObject *, const char *> Listener;
	QSet<Listener> listeners; // call each listener only once when multiple options changed
	
	foreach (AbstractInput *input, singleInput ? QList<AbstractInput *>() << singleInput : inputs)
	{
		if (input->receiver && input->member)
		{
			if (input->toVariant() != settings.value(input->key, input->defaultVal))
			{
				input->wasChanged = true;
//				QMetaObject::invokeMethod(input->receiver, input->member);
				listeners.insert(qMakePair(input->receiver, input->member));
			}
			else
			{
				input->wasChanged = false;
				continue;
			}
		}
		settings.setValue(input->key, input->toVariant());
	}
	
	foreach (Listener listener, listeners)
		QMetaObject::invokeMethod(listener.first, listener.second);
}

QByteArray operator^(const QByteArray &a1, const QByteArray &a2)
{
	QByteArray res = a1;
	for (int i = 0, j = 0; i < res.size(); ++i, ++j %= a2.size())
		res.data()[i] ^= a2.data()[j];
	return res;
}

template<>QVariant SettingsManager::Input<QButtonGroup>::toVariant() const
{
	return object->checkedId();
}

template<>void SettingsManager::Input<QButtonGroup>::fromVariant(QVariant variant)
{
	if (object->button(variant.toInt()))
		object->button(variant.toInt())->setChecked(true);
}

template<>QVariant SettingsManager::Input<QCalendarWidget>::toVariant() const
{
	return object->selectedDate();
}

template<>void SettingsManager::Input<QCalendarWidget>::fromVariant(QVariant variant)
{
	object->setSelectedDate(variant.toDate());
}

template<>QVariant SettingsManager::Input<QCheckBox>::toVariant() const
{
	return object->isChecked();
}

template<>void SettingsManager::Input<QCheckBox>::fromVariant(QVariant variant)
{
	object->setChecked(variant.toBool());
}

template<>QVariant SettingsManager::Input<QComboBox>::toVariant() const
{
	return object->currentIndex();
}

template<>void SettingsManager::Input<QComboBox>::fromVariant(QVariant variant)
{
	object->setCurrentIndex(variant.toInt());
}

template<>QVariant SettingsManager::Input<QDateEdit>::toVariant() const
{
	return object->date();
}

template<>void SettingsManager::Input<QDateEdit>::fromVariant(QVariant variant)
{
	object->setDate(variant.toDate());
}

template<>QVariant SettingsManager::Input<QDateTimeEdit>::toVariant() const
{
	return object->dateTime();
}

template<>void SettingsManager::Input<QDateTimeEdit>::fromVariant(QVariant variant)
{
	object->setDateTime(variant.toDateTime());
}

template<>QVariant SettingsManager::Input<QDial>::toVariant() const
{
	return object->value();
}

template<>void SettingsManager::Input<QDial>::fromVariant(QVariant variant)
{
	object->setValue(variant.toInt());
}

template<>QVariant SettingsManager::Input<QDoubleSpinBox>::toVariant() const
{
	return object->value();
}

template<>void SettingsManager::Input<QDoubleSpinBox>::fromVariant(QVariant variant)
{
	object->setValue(variant.toDouble());
}

template<>QVariant SettingsManager::Input<QFontComboBox>::toVariant() const
{
	return object->currentFont();
}

template<>void SettingsManager::Input<QFontComboBox>::fromVariant(QVariant variant)
{
	object->setCurrentFont(variant.value<QFont>());
}

template<>QVariant SettingsManager::Input<QGroupBox>::toVariant() const
{
	return object->isChecked();
}

template<>void SettingsManager::Input<QGroupBox>::fromVariant(QVariant variant)
{
	object->setChecked(variant.toBool());
}

template<>QVariant SettingsManager::Input<QLineEdit>::toVariant() const
{
	return (object->echoMode() == QLineEdit::Password) ? QVariant(encode(object->text())) : QVariant(object->text());
}

template<>void SettingsManager::Input<QLineEdit>::fromVariant(QVariant variant)
{
	object->setText((object->echoMode() == QLineEdit::Password) ? decode(variant.toByteArray()) : variant.toString());
}

template<>QVariant SettingsManager::Input<QPlainTextEdit>::toVariant() const
{
	return object->toPlainText();
}

template<>void SettingsManager::Input<QPlainTextEdit>::fromVariant(QVariant variant)
{
	object->setPlainText(variant.toString());
}

template<>QVariant SettingsManager::Input<QSlider>::toVariant() const
{
	return object->value();
}

template<>void SettingsManager::Input<QSlider>::fromVariant(QVariant variant)
{
	object->setValue(variant.toInt());
}

template<>QVariant SettingsManager::Input<QSpinBox>::toVariant() const
{
	return object->value();
}

template<>void SettingsManager::Input<QSpinBox>::fromVariant(QVariant variant)
{
	object->setValue(variant.toInt());
}

template<>QVariant SettingsManager::Input<QTextEdit>::toVariant() const
{
	return object->toHtml();
}

template<>void SettingsManager::Input<QTextEdit>::fromVariant(QVariant variant)
{
	object->setHtml(variant.toString());
}

template<>QVariant SettingsManager::Input<QTimeEdit>::toVariant() const
{
	return object->time();
}

template<>void SettingsManager::Input<QTimeEdit>::fromVariant(QVariant variant)
{
	object->setTime(variant.toTime());
}

template<>QVariant SettingsManager::Input<QWidget>::toVariant() const
{
	return object->saveGeometry();
}

template<>void SettingsManager::Input<QWidget>::fromVariant(QVariant variant)
{
	object->restoreGeometry(variant.toByteArray());
}

template<>QVariant SettingsManager::Input<QMainWindow>::toVariant() const
{
	return object->saveState();
}

template<>void SettingsManager::Input<QMainWindow>::fromVariant(QVariant variant)
{
	object->restoreState(variant.toByteArray());
}
