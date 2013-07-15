//#include <stdio.h>
#include <string.h>		// memset
#include <stdlib.h>		// exit
#include <unistd.h>		// daemon, sleep
#include <signal.h>		// sigaction
#include "debug.h"


extern int startStoreThread();  // storeTread.c
static int recv_abort;

static void signalHandler(int signo)
{
	// it is not safe to call I/O function in signal handler
	//DEBUG_PRINT("***** signalHandler signo = %d *****\n", signo); // not safe
	recv_abort = signo;
}

static void setSignalHandler()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signalHandler;
	sa.sa_flags |= SA_NODEFER;
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}


int main(int argc, char *argv[])
{
	int interval = 10;

	setSignalHandler();

	// run in the background
	//int rc = daemon(0, 0); // chdir to "/", close stdout/stderr
	int rc = daemon(1, 0); // not change currend working dir, close stdout/stderr
	if(rc != 0) {
		_PERROR("@@@daemon() error");
		exit(2);
	}

	if(startStoreThread() != 0) {
		ERROR_PRINT("@@@startStoreThread() error\n");
		exit(3);
	}

	sleep(1); // wait for thread

	while(1) {
		if (recv_abort) {
			DEBUG_PRINT("***** abort signal (%d)\n", recv_abort);
			// clean up here, close socket, unlink socket device file, etc...
			exit(4);
		}

		DEBUG_PRINT("***** main() sleep(%d)\n", interval);
		sleep(interval);
	}

	return 0;
}
