#ifndef DISPLAYSLIDER_H
#define DISPLAYSLIDER_H

#include "settingsmanager.h"
#include <QSlider>

class DisplaySlider : public QSlider
{
	Q_OBJECT

public:
	explicit DisplaySlider(QWidget *parent = nullptr);
	void setFormat(QString format);

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void paintEvent(QPaintEvent *ev);

private:
	QString m_format;
	bool pressed;
};

template<>QVariant SettingsManager::Input<DisplaySlider>::toVariant() const;
template<>void SettingsManager::Input<DisplaySlider>::fromVariant(QVariant variant);

#endif // DISPLAYSLIDER_H
