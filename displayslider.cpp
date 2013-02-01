#include "displayslider.h"
#include <QPainter>

DisplaySlider::DisplaySlider(QWidget *parent) :
	QSlider(parent),
	m_format("%1"),
	pressed(false)
{
}

void DisplaySlider::setFormat(QString format)
{
	m_format = format;
}

void DisplaySlider::mousePressEvent(QMouseEvent *ev)
{
	pressed = true;
	QSlider::mousePressEvent(ev);
	update();
}

void DisplaySlider::mouseReleaseEvent(QMouseEvent *ev)
{
	pressed = false;
	QSlider::mouseReleaseEvent(ev);
	update();
}

void DisplaySlider::paintEvent(QPaintEvent *ev)
{
	QSlider::paintEvent(ev);
	if (pressed)
	{
		QPainter p(this);
		QRectF bounding;
		p.drawText(QRect(QPoint(0, 0), size()), Qt::AlignCenter | Qt::AlignHCenter, QString(m_format).arg(value()), &bounding);
		bounding.adjust(-5, -2, 5, 2);
		p.setBrush(QColor(245, 235, 150));
		p.drawRoundedRect(bounding, 5, 5);
		p.drawText(QRect(QPoint(0, 0), size()), Qt::AlignCenter | Qt::AlignHCenter, QString(m_format).arg(value()), &bounding);
	}
}

template<>QVariant SettingsManager::Input<DisplaySlider>::toVariant() const
{
	return object->value();
}

template<>void SettingsManager::Input<DisplaySlider>::fromVariant(QVariant variant)
{
	object->setValue(variant.toInt());
}
