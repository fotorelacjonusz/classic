#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit : public QLineEdit
{
	Q_OBJECT
public:
	explicit LineEdit(QWidget *parent = 0);
	
	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);
	
signals:
	void focusIn();
	void focusOut();
	
};

#endif // LINEEDIT_H
