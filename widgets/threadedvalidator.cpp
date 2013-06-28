#include "threadedvalidator.h"
#include "downloaders/throttlednetworkmanager.h"
#include "application.h"
#include <QMessageBox>
#include <QDebug>
#include <QAbstractMessageHandler>

class MessageHandler : public QAbstractMessageHandler
{
public:
	MessageHandler(ThreadedValidator *parent):
		validator(parent)
	{
	}
	
	void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
	{
		validator->message(type, description, identifier, sourceLocation);
	}
	
private:
	ThreadedValidator *validator;
};

ThreadedValidator::ThreadedValidator(QUrl schemaUrl) :
	handler(new MessageHandler(this)),
	validator(schema)
{
	static ThrottledNetworkManager manager;
	schema.setNetworkAccessManager(manager.manager());
	schema.load(schemaUrl);
	Q_ASSERT(schema.isValid());
	validator.setMessageHandler(handler);
}

ThreadedValidator::~ThreadedValidator()
{
	delete handler;
}

void ThreadedValidator::validate(QUrl fileUrl)
{
	this->fileUrl = fileUrl;
	start();
}

void ThreadedValidator::run()
{
	Application::busy();
	emit validated(validator.validate(fileUrl));
	Application::idle();
}
