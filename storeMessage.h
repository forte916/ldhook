#ifndef _STORE_MESSAGE_H
#define _STORE_MESSAGE_H

#include <sys/types.h>	// pid_t
#include <unistd.h>		// pid_t

#ifndef MAX_PATH_LEN
#  define MAX_PATH_LEN   1024	/* max pathname length */
#endif

#define NAME_LEN 255		// max file name
#define CMD_LEN  1200		// max message size. at least bigger than struct store_msg
#define SUN_PATH "/tmp/localudp"  // path to Unix domain socket


#define STORE_RESOURCE   1
#define STORE_ADD_PATH   2
#define STORE_DEL_PATH   3
#define STORE_RESULT     4
#define STORE_ADD_RESULT 5
#define STORE_DEL_RESULT 6


struct store_msg {
	int cmd;
	pid_t pid;
	int fd;
	char path[MAX_PATH_LEN];
	//ino_t inode;  // if needed
	int result;
};


int storeMsgToString(struct store_msg *sm);

#endif // _STORE_MESSAGE_H
