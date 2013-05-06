#include "arrowwidget.h"
#include "lineedit.h"
#include <QStyleOption>
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <cmath>

const int ArrowWidget::margin = 7;

ArrowWidget::ArrowWidget(QPoint start, QPoint end, QWidget *parent) :
//	QWidget(parent),
	SelectableWidget<ArrowWidget>(parent),
	color(Qt::black),
	start(start),
//	lineEdit(new LineEdit(this)),
	inverted(false)
{
	lineEdit->hide();
	setEnd(end);
//	static int instanceCount = 0;
//	setObjectName(QString("arrow%1").arg(instanceCount++));
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
	setGeometry(rect);
}

void ArrowWidget::showEdit(QString text)
{
//	if (!lineEdit)
//	qDebug() << "showEdit";
	if (!lineEdit->isVisible())
	{
		bool left = end.x() > start.x();
//		lineEdit = new LineEdit(this);
		lineEdit->show();
		lineEdit->setFont(QFont("Arial", 16));
		lineEdit->setAlignment(left ? Qt::AlignRight : Qt::AlignLeft);
		lineEdit->setMinimumWidth(20);
		if (!text.isNull())
			lineEdit->setText(text);
		else
		{
			lineEdit->setText("...");
			lineEdit->selectAll();
			lineEdit->setFocus();
//			mouseReleaseEvent(0);
		}
		select();
		connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateGeometries()));
		updateGeometries();
	}
	
	QString background = QString("rgba(%1, %1, %1, 150)").arg(qGray(color.rgb()) < 128 ? 255 : 0);
	lineEdit->setStyleSheet(QString("background: %2; border: transparent; color: %1").arg(color.name()).arg(background));
}

//void ArrowWidget::unselected()
//{
//	setStyleSheet(QString("QWidget#%1 { }").arg(objectName()));
//}

//void ArrowWidget::mouseReleaseEvent(QMouseEvent *event)
//{
//	setStyleSheet(QString("QWidget#%1 { background: rgba(20, 80, 200, 50); border: 3px dashed rgba(20, 80, 200, 250) }").arg(objectName()));
//	if (lineEdit)
//	{
//		lineEdit->setFocus();
//		lineEdit->selectAll();
//	}
//	emit selected(this);

//	if (event)
//		event->accept();
//}

void ArrowWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	// draw applied stylesheet
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QPen pen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

	p.setPen(pen);
	p.setBrush(Qt::red);
	p.setRenderHint(QPainter::Antialiasing);
	p.drawLine(mapFromParent(start), mapFromParent(end));

	if (lineEdit->isVisible())
		p.drawLine(lineEdit->geometry().bottomLeft() + QPoint(0, 1), lineEdit->geometry().bottomRight() + QPoint(0, 1));

	pen.setCapStyle(Qt::SquareCap);
	p.setPen(pen);

	qreal angle = atan2(end.y() - start.y(), end.x() - start.x());
	qreal r = 20, da = 15 * M_PI / 180,	a1 = angle + da, a2 = angle - da;

	QPointF point = mapFromParent(end);
	QPolygonF polygon = QPolygonF() << point - QPointF(r * cos(a1), r * sin(a1)) << point << point - QPointF(r * cos(a2), r * sin(a2));
	p.drawPolyline(polygon);
	//	p.drawPolygon(polygon);
}

//void ArrowWidget::focusOutEvent(QFocusEvent *e)
//{
//	qDebug() << objectName() << "focus out";
//	QWidget::focusOutEvent(e);
//}

void ArrowWidget::updateGeometries()
{
	const bool left = end.x() > start.x();
	const bool top = end.y() > start.y();

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

