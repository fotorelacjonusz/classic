#ifndef SELECTABLEWIDGET_H
#define SELECTABLEWIDGET_H

#include <QWidget>
#include <QSet>
#include <QMouseEvent>
#include <QDebug>
#include <QColor>
#include "lineedit.h"

// This class is used only because Q_OBJECT cannot be used in template.
class SelectableWidgetListenerParent : public QObject
{
	Q_OBJECT

protected:
	SelectableWidgetListenerParent(QObject *parent = 0):
		QObject(parent)
	{}

signals:
	void selected(QWidget *widget);
};

/* Read SelectableWidget description first.
 * This is the class, which emits signals on selection change.
 * Typedefed as SelectableWidget<YourType>::Listener
 */
template <class Type>
class SelectableWidgetListener : public SelectableWidgetListenerParent
{
public:
	SelectableWidgetListener(QObject *parent = 0):
		SelectableWidgetListenerParent(parent)
	{
		listeners.insert(this);
	}

	virtual ~SelectableWidgetListener()
	{
		listeners.remove(this);
	}

protected:
	static void tellAllWidgetSelected(QWidget *widget)
	{
		foreach (SelectableWidgetListener<Type> *listener, listeners)
			emit listener->selected(widget);
	}

	static QSet<SelectableWidgetListener<Type> *> listeners;
	template <class T> friend class SelectableWidget;
};

template <class Type>
QSet<SelectableWidgetListener<Type> *> SelectableWidgetListener<Type>::listeners;

// This class is used only because Q_OBJECT cannot be used in template.
class SelectableWidgetParent : public QWidget
{
	Q_OBJECT

public:
	SelectableWidgetParent(QWidget *parent):
		QWidget(parent)
	{}

public slots:
	virtual void select() = 0;
	virtual void unselect(bool tellAll = true) = 0;
};

/* Subclass SelectableWidget<YourType> to make it react on mouse clicks.
 * Clicked widget gets border, signal from Listener is emitted.
 * Signal with 0 means no widget is selected.
 * In slot you have to use cast qobject_cast<YourType *>(widget)
 * There can be only one selected widget for all objects of YourType type.
 */
template <class Type>
class SelectableWidget : public SelectableWidgetParent
{

public:
	typedef SelectableWidgetListener<Type> Listener;

protected:
	explicit SelectableWidget(QWidget *parent = 0):
		SelectableWidgetParent(parent),
		borderColor(20, 80, 200)
	{
		lineEdit = new LineEdit(this);
		static int objectNumber = 0;
		setObjectName(Type::staticMetaObject.className() + QString::number(++objectNumber));
		connect(lineEdit, SIGNAL(focusIn()), this, SLOT(select()));
//		connect(lineEdit, SIGNAL(focusOut()), this, SLOT(unselect()));
	}

	virtual ~SelectableWidget()
	{
		if (isSelected())
			unselect();
	}

	void mousePressEvent(QMouseEvent *event)
	{
		pressPos = event->pos();
		QWidget::mousePressEvent(event);
	}

	void mouseReleaseEvent(QMouseEvent *event)
	{
		if (pressPos == event->pos())
			!isSelected() ? select() : unselect();
		else
			QWidget::mouseReleaseEvent(event);
	}

public:
	inline bool isSelected() const
	{
		return this == selectedWidget;
	}

	void select()
	{
		if (isSelected())
			return;
		if (selectedWidget)
			selectedWidget->unselect(false);
		selectedWidget = this;
		lineEdit->setFocus();

		setStyleSheet(QString("QWidget#%1 { border-width: 2px; border-style: dashed; border-color: %2; border-radius: 8px; }").arg(objectName()).arg(borderColor.name()));
//		setStyleSheet(QString("QWidget#%1 { border: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 white, stop: 0.4 gray, stop:1 green);}").arg(objectName()));
//		setStyleSheet(QString("QWidget#%1 { border-width: 3px; border-style: dashed; border-color: %2; }").arg(objectName()).arg(borderColor.name()));
//		setStyleSheet(QString("QWidget#%1 { box-shadow: 3px 3px 4px %2; -moz-box-shadow: 3px 3px 4px %2; -webkit-box-shadow: 3px 3px 4px %2; }").arg(objectName()).arg(borderColor.name()));
		Listener::tellAllWidgetSelected(this);
		selectEvent();
	}

	void unselect(bool tellAll = true)
	{
		if (!isSelected())
			return;
		selectedWidget = 0;
		lineEdit->clearFocus();
		setStyleSheet(QString("QWidget#%1 { }").arg(objectName()));
		if (tellAll)
			Listener::tellAllWidgetSelected(0);
		unselectEvent();
	}

protected:
	virtual void selectEvent() {}
	virtual void unselectEvent() {}

	LineEdit *lineEdit;
	QColor borderColor;
	QPoint pressPos;

private:
	static SelectableWidget<Type> *selectedWidget;
};

template <class Type>
SelectableWidget<Type> *SelectableWidget<Type>::selectedWidget = 0;


#endif // SELECTABLEWIDGET_H
