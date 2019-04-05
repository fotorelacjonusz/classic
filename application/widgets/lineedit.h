#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QPlainTextEdit>

class LineEdit : public QLineEdit
{
	Q_OBJECT
public:
	explicit LineEdit(QWidget *parent = nullptr);

	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);

signals:
	void focusIn();
	void focusOut();
};

class PlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit PlainTextEdit(QWidget *parent = nullptr);

	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);

signals:
	void focusIn();
	void focusOut();
};

#endif // LINEEDIT_H
