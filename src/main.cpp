#include "application.h"
#include <QPixmapCache>
#include <cstdio>
#include "messagehandler.h"

#ifdef Q_OS_UNIX

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) 
{
	void *array[10];
	size_t size = backtrace(array, 10);
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

#endif

int main(int argc, char *argv[])
{

#ifdef Q_OS_UNIX
	signal(SIGSEGV, handler);
#endif

	if (argc >= 2 && QString(argv[1]) == "-v")
	{
		fprintf(stdout, "%s\n", PROGRAM_VERSION);
		return 0;
	}
	
	qInstallMessageHandler(messageHandler);
	
	QPixmapCache::setCacheLimit(524288);

	Application a(argc, argv);
	return a.showWindowAndExec();
}
