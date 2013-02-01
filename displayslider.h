#ifndef DISPLAYSLIDER_H
#define DISPLAYSLIDER_H

#include "settingsmanager.h"
#include <QSlider>

class DisplaySlider : public QSlider
{
	Q_OBJECT
public:
	explicit DisplaySlider(QWidget *parent = 0);
	void setFormat(QString format);

signals:
	
public slots:

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
