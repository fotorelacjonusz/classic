#include "messagehandler.h"
#include <cstdio>

bool Suppress::suppress = false;

Suppress::Suppress()
{
	suppress = true;
}

//bool Suppress::operator <<(bool b)
//{
//	return b;
//}

Suppress::~Suppress()
{
	suppress = false;
}

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
//	Disable some old log suppressing (?) by KamilOst
//	if (type != QtFatalMsg && Suppress::suppress)
//		return;
	
	QByteArray localMsg(msg.toLocal8Bit());
	switch (type) {
		case QtDebugMsg:
			fprintf(stderr, "Debug: %s (%s:%u, %s)\n",    localMsg.constData(), context.file, context.line, context.function);
			break;
		case QtInfoMsg:
			fprintf(stderr, "Info: %s (%s:%u, %s)\n",     localMsg.constData(), context.file, context.line, context.function);
			break;
		case QtWarningMsg:
			fprintf(stderr, "Warning: %s (%s:%u, %s)\n",  localMsg.constData(), context.file, context.line, context.function);
			break;
		case QtCriticalMsg:
			fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
			break;
		case QtFatalMsg:
			fprintf(stderr, "Fatal: %s (%s:%u, %s)\n",    localMsg.constData(), context.file, context.line, context.function);
			break;
		}

#ifdef Q_OS_WIN32
	fflush(stderr);
#endif
}
