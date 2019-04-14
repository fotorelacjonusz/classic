#include "lineedit.h"

LineEdit::LineEdit(QWidget *parent):
	QLineEdit(parent)
{
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
	emit focusIn();
	return QLineEdit::focusInEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
	emit focusOut();
	return QLineEdit::focusOutEvent(event);
}

PlainTextEdit::PlainTextEdit(QWidget *parent):
	QPlainTextEdit(parent)
{
}

void PlainTextEdit::focusInEvent(QFocusEvent *event)
{
	emit focusIn();
	return QPlainTextEdit::focusInEvent(event);
}

void PlainTextEdit::focusOutEvent(QFocusEvent *event)
{
	emit focusOut();
	return QPlainTextEdit::focusOutEvent(event);
}
