#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QtGlobal>

class Suppress
{
public:
	Suppress();
//	bool operator <<(bool b);
	~Suppress();
	
private:
	static bool suppress;
	friend void messageHandler(QtMsgType type, const char *msg);
};

void messageHandler(QtMsgType type, const char *msg);

#endif // MESSAGEHANDLER_H
