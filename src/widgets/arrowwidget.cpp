#include "arrowwidget.h"
#include "lineedit.h"
#include <QStyleOption>
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QtMath>

const int ArrowWidget::margin = 7;

ArrowWidget::ArrowWidget(QPoint start, QPoint end, QWidget *parent, QString initialText) :
	SelectableWidget<ArrowWidget>(parent),
	color(Qt::black),
	start(start),
	inverted(false),
	initialText(initialText)
{
	lineEdit->hide();
	setEnd(end);
	connect(lineEdit, SIGNAL(focusOut()), this, SLOT(lineEditFocusOut()));
}

void ArrowWidget::invert()
{
	inverted = !inverted;
	color = inverted ? Qt::white : Qt::black;
	showEdit();
	update();
}

void ArrowWidget::setColor(QColor newColor)
{
	if (!newColor.isValid())
		return;
	color = newColor;
	showEdit();
	update();
}

void ArrowWidget::setEnd(QPoint _end)
{
	end = _end;
	arrowSize = QSize(qAbs(end.x() - start.x()), qAbs(end.y() - start.y()));
	QRect rect(qMin(start.x(), end.x()), qMin(start.y(), end.y()), arrowSize.width(), arrowSize.height());
	rect.adjust(-margin, -margin, margin, margin);
	left = end.x() > start.x();
	top = end.y() > start.y();
	setGeometry(rect);
}

void ArrowWidget::showEdit(QString text)
{
	if (!lineEdit->isVisible())
	{
		bool left = end.x() > start.x();
		lineEdit->show();
		lineEdit->setFont(QFont("Arial", 14));
		lineEdit->setAlignment(left ? Qt::AlignRight : Qt::AlignLeft);
		lineEdit->setMinimumWidth(20);
		if (!text.isNull())
			lineEdit->setText(text);
		else
		{
			lineEdit->setText(initialText);
			lineEdit->selectAll();
			lineEdit->setFocus();
		}
		select();
		connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateGeometries()));
		updateGeometries();
	}

	QString background = QString("rgba(%1, %1, %1, 150)").arg(qGray(color.rgb()) < 128 ? 255 : 0);
	lineEdit->setStyleSheet(QString("background: %2; border: transparent; color: %1").arg(color.name()).arg(background));
}

void ArrowWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	// draw applied stylesheet
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	p.setRenderHint(QPainter::Antialiasing);

	QPen pen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
	p.setBrush(color);
	p.setPen(pen);

	qreal angle = atan2(end.y() - start.y(), end.x() - start.x());
	qreal r = 20, da = 7 * M_PI / 180, a1 = angle + da, a2 = angle - da;
	QPointF arrowTip = mapFromParent(end);

	QPolygonF polygon;


	if (lineEdit->isVisible())
		polygon << QPoint(0, 3) + (left ? lineEdit->geometry().bottomLeft() : lineEdit->geometry().bottomRight());

//		p.drawLine(lineEdit->geometry().bottomLeft() + QPoint(0, 3), lineEdit->geometry().bottomRight() + QPoint(0, 3));
//	p.drawLine(mapFromParent(start + QPoint(0, 2)), mapFromParent(end));
	polygon << mapFromParent(start + QPoint(0, 2)) << arrowTip;
	p.drawPolyline(polygon);
	polygon.clear();
	polygon << arrowTip - QPointF(r * cos(a1), r * sin(a1)) << arrowTip - QPointF(r * cos(a2), r * sin(a2)) << arrowTip;
	p.drawPolygon(polygon);
}

void ArrowWidget::updateGeometries()
{
	QFontMetrics metrics(lineEdit->font());
	const int width = qMax(metrics.boundingRect(lineEdit->text()).width() + 10, lineEdit->minimumWidth());

	QRect thisGeometry;
	thisGeometry.setX(left ? start.x() - width : end.x());
	thisGeometry.setY(top ? start.y() - lineEdit->height() : qMin(end.y(), start.y() - lineEdit->height()));
	thisGeometry.setWidth(width + arrowSize.width());
	thisGeometry.setHeight(top ? arrowSize.height() + lineEdit->height() : qMax(arrowSize.height(), lineEdit->height()));
	setGeometry(thisGeometry.adjusted(-margin, -margin, margin, margin));

	QRect lineEditGeometry;
	lineEditGeometry.setX(margin + (left ? 0 : arrowSize.width()));
	lineEditGeometry.setY(margin + (top ? 0 : qMax(arrowSize.height(), lineEdit->height()) - lineEdit->height()));
	lineEditGeometry.setWidth(width);
	lineEditGeometry.setHeight(lineEdit->height());
	lineEdit->setGeometry(lineEditGeometry);
}

void ArrowWidget::lineEditFocusOut()
{
	lineEdit->setVisible(!lineEdit->text().isEmpty());
}
