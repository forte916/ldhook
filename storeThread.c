#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "debug.h"
#include "storeMessage.h"
#include "storeControl.h"
#include "storeSqlite.h"

#define MAX_EVENTS 5
#define BACKLOG 5


/**
 *  @brief prepare Unix domain socket
 *  @return file descriptor or -1
 */
static int setupSocket()
{
	int sock_fd;
	struct sockaddr_un sun;

	DEBUG_FUNCIN("\n");
	if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		_PERROR("@@@socket");
		return -1;
	}
	DEBUG_PRINT("socket success");

	int reuse = 1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	DEBUG_PRINT("setsockopt success");

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, SUN_PATH);

	unlink(SUN_PATH);
	if(bind(sock_fd, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
		close(sock_fd);
		_PERROR("@@@bind");
		return -1;
	}
	DEBUG_PRINT("bind success");

	// permit write access for samba and ftp user.
	if(chmod(SUN_PATH, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) < 0) {
		close(sock_fd);
		_PERROR("@@@chmod");
		return -1;
	}
	DEBUG_PRINT("chmod success");

	if(listen(sock_fd, BACKLOG) < 0) {
		close(sock_fd);
		_PERROR("@@@listen");
		return -1;
	}
	DEBUG_PRINT("listen success");

	DEBUG_PRINT("sock_fd = %d\n", sock_fd);
	DEBUG_FUNCOUT("\n");
	return sock_fd;
}

static void do_store_cmd(int sock_fd, char *data, sqlite3* db)
{
	DEBUG_FUNCIN("\n");
	struct store_msg *sm = (struct store_msg *)data;

	DEBUG_PRINT("----- before kick cmd -----\n");
	storeMsgToString(sm);

	switch (sm->cmd) {
		case STORE_ADD_PATH:
			putOpenSessionDB(db, sm);
			sm->result = 0;
			break;
		case STORE_DEL_PATH:
			delOpenSessionDB(db, sm);
			sm->result = 0;
			break;
		default:
			break;
	}

	DEBUG_PRINT("----- after kick cmd -----\n");
	storeMsgToString(sm);

	DEBUG_FUNCOUT("\n");
}


// TODO:: condider how to receive store result. what is return as a result
/**
 *  @brief send a result to hook.so
 *  @param file descriptor of client socket
 *  @param struct store_msg
 *  @return 0:success or -1:error
 */
static int sendStoreResult(int sock_fd, char *data)
{
	DEBUG_FUNCIN("\n");
	size_t len;
	ssize_t rc;
	struct store_msg *sm = (struct store_msg *)data;
	struct store_msg res_sm;

	res_sm.cmd = STORE_RESULT;
	res_sm.pid = sm->pid;
	res_sm.fd = sm->fd;
	strcpy(res_sm.path, sm->path);
	res_sm.result = sm->result;


	DEBUG_PRINT("----- before write to client -----\n");
	storeMsgToString(&res_sm);

	len = sizeof(struct store_msg);
	rc = write(sock_fd, &res_sm, len);
	if(rc < 0) {
		_PERROR("@@@write");
		DEBUG_PRINT("write error len = %d rc = %d\n", len, rc);
		return -1;
	}
	DEBUG_PRINT("write to fd = %d, len = %d rc = %d\n", sock_fd, len, rc);

	DEBUG_FUNCOUT("\n");
	return 0;
}


static void * storeThread(void *arg)
{
	DEBUG_FUNCIN("\n");
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];
	char buf[CMD_LEN];
	int epfd;
	int server_fd;
	sqlite3* db;

	server_fd = setupSocket();
	if(server_fd < 0) {
		return NULL;
	}

	if((epfd = epoll_create(MAX_EVENTS)) < 0) {
		_PERROR("@@@epoll_create");
		return NULL;
	}

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

	db = initOpenSessionDB();

	while(1) {
		DEBUG_PRINT("----- event loop\n");
		int i = 0;
		ssize_t rc = 0;
		int client_fd = -1;
		int nfd = epoll_wait(epfd, events, MAX_EVENTS, -1);  // -1=Blocking
		DEBUG_PRINT("----- event came. nfd = %d\n", nfd);
		for(i = 0; i < nfd; i++){
			if(events[i].data.fd == server_fd) {
				DEBUG_PRINT("storeThread server_fd = %d\n", server_fd);
				struct sockaddr_un client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
				if (client_fd < 0) {
					_PERROR("@@@accept");
					continue;
				}
				DEBUG_PRINT("client might be connected. client_fd = %d\n", client_fd);

				rc = read(client_fd, buf, CMD_LEN);
				DEBUG_PRINT("read from fd = %d, rc = %d\n", client_fd, rc);
				if(rc <= 0) {
					if(rc < 0) {
						_PERROR("@@@read");
					}
					continue;
				}else{
					DEBUG_PRINT("read from fd = %d, rc = %d, buf = %s\n", client_fd, rc, buf);
					do_store_cmd(client_fd, buf, db);
					sendStoreResult(client_fd, buf);

					// add client socket to wait event
					memset(&ev, 0, sizeof(ev));
					ev.events = EPOLLIN;
					ev.data.fd = client_fd;
					epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
				}

			} else {
				DEBUG_PRINT("storeThread not server fd = %d\n", events[i].data.fd);
				client_fd = events[i].data.fd;

				if (events[i].events & EPOLLIN) {
					rc = read(client_fd, buf, CMD_LEN);
					DEBUG_PRINT("read from fd = %d, rc = %d\n", client_fd, rc);
					if(rc <= 0) {
						if(rc < 0) {
							_PERROR("@@@read");
						}
						DEBUG_PRINT("client might be closed. client_fd = %d\n", client_fd);

						// del client socket from waiting list
						epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
						close(client_fd);
					} else {
						DEBUG_PRINT("read from fd = %d, rc = %d, buf = %s\n", client_fd, rc, buf);
						do_store_cmd(client_fd, buf, db);
						sendStoreResult(client_fd, buf);

					}
				} else if (events[i].events & EPOLLOUT) {
					DEBUG_PRINT("EPOLLOUT try to write() here.\n");

				}
			}
		}
	}

	DEBUG_PRINT("----- out of event loop\n");  // never reach
	close(server_fd);
	unlink(SUN_PATH);
	quitOpenSessionDB(db);

	DEBUG_FUNCOUT("\n");
	return NULL;
}

int startStoreThread()
{
	DEBUG_FUNCIN("\n");
	pthread_t t;

	int rc = pthread_create(&t, NULL, storeThread, NULL);
	if(rc != 0) {
		ERROR_PRINT("@@@pthread_create error %d\n", rc);
	}

	DEBUG_FUNCOUT("\n");
	return rc;
}

