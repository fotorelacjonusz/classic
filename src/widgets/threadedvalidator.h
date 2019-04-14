#ifndef THREADEDVALIDATOR_H
#define THREADEDVALIDATOR_H

#include <QThread>
#include <QUrl>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

class QAbstractMessageHandler;
class QSourceLocation;
class MessageHandler;

class ThreadedValidator : public QThread
{
	Q_OBJECT
public:
	explicit ThreadedValidator(QUrl schemaUrl);
	~ThreadedValidator();
	void validate(QUrl fileUrl);

protected:
	void run();

signals:
	void validated(bool valid);
	void message(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation);

private:
	QUrl fileUrl;
	QAbstractMessageHandler *handler;

	QXmlSchema schema;
	QXmlSchemaValidator validator;


	friend class MessageHandler;
};

#endif // THREADEDVALIDATOR_H
