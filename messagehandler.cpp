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

void messageHandler(QtMsgType type, const char *msg)
{
	if (type != QtFatalMsg && Suppress::suppress)
		return;
	
	switch (type) 
	{
		case QtDebugMsg:
			fprintf(stderr, "%s\n", msg);
			break;
		case QtWarningMsg:
			fprintf(stderr, "%s\n", msg);
			break;
		case QtCriticalMsg:
			fprintf(stderr, "%s\n", msg);
			break;
		case QtFatalMsg:
			fprintf(stderr, "%s\n", msg);
			abort();
	}
}
