#include <stdio.h>
//#define _GNU_SOURCE   // RTLD_NEXT needs to define _GNU_SOURCE before include <dlfcn.h>
#include <dlfcn.h>		// dlsym
#include <string.h>		// strlen, strcpy
#include <sys/types.h>	// open, socket
#include <unistd.h>		// close, read, write, fsync, getcwd
#include <sys/socket.h>	// socket
#include <sys/un.h>		// sockaddr_un
#include <arpa/tftp.h>	// EACCESS
#include <errno.h>		// errno

#include "debug.h"
#include "storeMessage.h"
#include "storeControl.h"


typedef int (*fp_open) (__const char *__file, int __oflag, mode_t __mode);
typedef int (*fp_open64) (__const char *__file, int __oflag, mode_t __mode);
typedef int (*fp_openat) (int __fd, __const char *__file, int __oflag, ...);
typedef int (*fp_openat64) (int __fd, __const char *__file, int __oflag, ...);
typedef int (*fp_close) (int __fd);

fp_open org_open = NULL;
fp_open64 org_open64 = NULL;
fp_openat org_openat = NULL;
fp_openat64 org_openat64 = NULL;
fp_close org_close = NULL;

#ifdef FOPEN_FCLOSE
typedef FILE* (*fp_fopen64) (__const char *__restrict __filename, __const char *__restrict __modes);
typedef FILE* (*fp_fopen) (__const char *__restrict __filename, __const char *__restrict __modes);
typedef int (*fp_fclose) (FILE *__stream);
fp_fopen64 org_fopen64 = NULL;
fp_fopen org_fopen = NULL;
fp_fclose org_fclose = NULL;
#endif



__attribute__((constructor))
static void initialize_hook_library()
{
	DEBUG_PRINT("[Enter] initialize_hook_library\n");
	org_open = (fp_open)dlsym(RTLD_NEXT, "open");
	org_open64 = (fp_open64)dlsym(RTLD_NEXT, "open64");
	org_openat = (fp_openat)dlsym(RTLD_NEXT, "openat");
	org_openat64 = (fp_openat64)dlsym(RTLD_NEXT, "openat64");
	org_close = (fp_close)dlsym(RTLD_NEXT, "close");

#ifdef FOPEN_FCLOSE
	org_fopen64 = (fp_fopen64)dlsym(RTLD_NEXT, "fopen64");
	org_fopen = (fp_fopen)dlsym(RTLD_NEXT, "fopen");
	org_fclose = (fp_fclose)dlsym(RTLD_NEXT, "fclose");
#endif
	DEBUG_PRINT("[Leave] initialize_hook_library\n");
}

__attribute__((destructor))
static void deinitialize_hook_library()
{
	DEBUG_PRINT("[Enter] deinitialize_hook_library\n");
	org_open = NULL;
	org_open64 = NULL;
	org_openat = NULL;
	org_openat64 = NULL;
	org_close = NULL;

#ifdef FOPEN_FCLOSE
	org_fopen64 = NULL;
	org_fopen = NULL;
	org_fclose = NULL;
#endif
	DEBUG_PRINT("[Leave] deinitialize_hook_library\n");
}



/**
 *  @brief connect to stored daemon
 *  @param file descriptor
 *  @return file descriptor or -1
 */
int connectSocket()
{
    int sock_fd;
    struct sockaddr_un sun;

	DEBUG_FUNCIN("\n");
    if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        _PERROR("@@@socket");
        return -1;
    }
    DEBUG_PRINT("socket success. sock_fd = %d\n", sock_fd);

    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, SUN_PATH);

    if(connect(sock_fd, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
        _PERROR("@@@connect");
		DEBUG_PRINT("Maybe, stored is not running.\n");
        org_close(sock_fd);
        return -1;
    }
    DEBUG_PRINT("connect success. sock_fd = %d\n", sock_fd);

	DEBUG_FUNCOUT("\n");
    return sock_fd;
}

/**
 *  @brief send a message to stored and then receive a result
 *  @param struct store_msg
 *  @return 0:cmd success, -1:error, the other: store result
 */
int sendStoreMsg(struct store_msg *sm)
{
	int fd;
	size_t len;
	ssize_t rc;
    char buf[CMD_LEN];
	struct store_msg *rcv_sm;
	int result;

	DEBUG_FUNCIN("\n");
	fd = connectSocket();
	if (fd < 0) {
		return -1;
	}

	DEBUG_PRINT("----- before write to server -----\n");
	storeMsgToString(sm);

	// send store message
	len = sizeof(struct store_msg);
    rc = write(fd, sm, len);
    if(rc  < 0) {
		_PERROR("@@@write");
        DEBUG_PRINT("write error len = %d, rc = %d\n", len, rc);
        return -1;
    }
    DEBUG_PRINT("write to fd = %d, len = %d, rc = %d\n", fd, len, rc);

	// MEMO:: read() blocks until receiving the result from server.
	//        which means it is not aynchronous design.
	// receive store result
	rc = read(fd, buf, CMD_LEN);
	if(rc < 0) {
		_PERROR("@@@read");
		DEBUG_PRINT("read from fd = %d, rc = %d\n", fd, rc);
        return -1;
	}
	DEBUG_PRINT("read from fd = %d, rc = %d, buf = %s\n", fd, rc, buf);

	// TODO:: condider how to receive store result. what is return as a result
	rcv_sm = (struct store_msg *)buf;
	result = rcv_sm->result;
	DEBUG_PRINT("store result  = %d\n", result);

	DEBUG_PRINT("----- after read from server -----\n");
	storeMsgToString(rcv_sm);

    org_close(fd);

	DEBUG_FUNCOUT("\n");
	return result;
}

int addPath2Store(const char *__file, int __fd)
{
	DEBUG_FUNCIN("\n");
	int ret;
	struct store_msg sm;

	sm.cmd = STORE_ADD_PATH;
	pid_t pid = getpid();
	sm.pid = pid;
	sm.fd = __fd;
	strcpy(sm.path, __file);
	sm.result = -1;

	ret = sendStoreMsg(&sm);

	DEBUG_FUNCOUT("\n");
	return ret;
}

int delPath2Store(int __fd)
{
	DEBUG_FUNCIN("\n");
	int ret;
	struct store_msg sm;

	sm.cmd = STORE_DEL_PATH;
	pid_t pid = getpid();
	sm.pid = pid;
	sm.fd = __fd;
	sm.path[0] = '\0';
	sm.result = -1;

	ret = sendStoreMsg(&sm);

	DEBUG_FUNCOUT("\n");
	return ret;
}

/**
 *  @brief set absolute path to dest.
 *  @param[out] dest
 *  @param[in] relative path
 *  @return 0 on success, -1 on error
 */
int getFullpath(char *dest, const char *relpath)
{
	DEBUG_FUNCIN("\n");
	char curdir[MAX_PATH_LEN];

	if (getcwd(curdir, MAX_PATH_LEN - 1) == NULL) {
		strcpy(dest, relpath);
		_PERROR("@@@getcwd");
		return -1;
	} else {
		DEBUG_PRINT("  curdir = %s\n", curdir);
		snprintf(dest, MAX_PATH_LEN - 1, "%s/%s", curdir, relpath);
		DEBUG_PRINT("  fullpath = %s\n", dest);
	}

	DEBUG_FUNCOUT("\n");
	return 0;
}


/************************************************
 * hooked open
 ************************************************/

/**
 *  @brief hooked open
 *  @param __file file path
 *  @param __oflag file status flags
 *  @param __mode file permissions
 *                If O_CREAT is not specified, then __mode is indetermination.
 *  @return file descriptor on success, -1 on error
 */
int open(__const char *__file, int __oflag, mode_t __mode)
{
	int ret = 0;
	int org_ret = 0;
	int regflag = 0;
	char fullpath[MAX_PATH_LEN];

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__file=[%s] __oflag=[0%x] __mode=[0%o]\n", __file, __oflag, __mode);
	DEBUG_FUNCIN("__file length = %d\n", strlen(__file));

	if (*__file == '/') {
		strcpy(fullpath, __file);
		DEBUG_PRINT("__file is absolute path. fullpath = %s\n", fullpath);
	} else {
		getFullpath(fullpath, __file);
		DEBUG_PRINT("__file is relative path. __file = %s, fullpath = %s\n", __file, fullpath);
	}

	regflag = isRegularFileByPath(__file, __oflag);

	org_ret = org_open(__file, __oflag, __mode);

	// keep file descriptor to be called close
	if (regflag && org_ret >= 0) {
		addPath2Store(fullpath, org_ret);
	}

	DEBUG_FUNCOUT("org_ret=[%d] __file=[%s] __oflag=[0%x] __mode=[0%o]\n", org_ret, __file, __oflag, __mode);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}

/**
 *  @brief hooked open
 *  @param __file file path
 *  @param __oflag file status flags
 *  @param __mode file permissions
 *                If O_CREAT is not specified, then __mode is indetermination.
 *  @return file descriptor on success, -1 on error
 */
int open64(__const char *__file, int __oflag, mode_t __mode)
{
	int ret = 0;
	int org_ret = 0;
	int regflag = 0;
	char fullpath[MAX_PATH_LEN];

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__file=[%s] __oflag=[0%x] __mode=[0%o]\n", __file, __oflag, __mode);
	DEBUG_FUNCIN("__file length = %d\n", strlen(__file));

	if (*__file == '/') {
		strcpy(fullpath, __file);
		DEBUG_PRINT("__file is absolute path. fullpath = %s\n", fullpath);
	} else {
		getFullpath(fullpath, __file);
		DEBUG_PRINT("__file is relative path. __file = %s, fullpath = %s\n", __file, fullpath);
	}

	regflag = isRegularFileByPath(__file, __oflag);

	org_ret = org_open64(__file, __oflag, __mode);

	// keep file descriptor to be called close
	if (regflag && org_ret >= 0) {
		addPath2Store(fullpath, org_ret);
	}

	DEBUG_FUNCOUT("org_ret=[%d] __file=[%s] __oflag=[0%x] __mode=[0%o]\n", org_ret, __file, __oflag, __mode);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}

int openat(int __fd, __const char *__file, int __oflag)
{
	int ret = 0;
	int org_ret = 0;

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__fd=[%d], __file=[%s] __oflag=[0%x]\n", __fd, __file, __oflag);

	org_ret = org_openat(__fd, __file, __oflag);

	DEBUG_FUNCOUT("org_ret=[%d]\n", org_ret);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}

int openat64(int __fd, __const char *__file, int __oflag)
{
	int ret = 0;
	int org_ret = 0;

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__fd=[%d], __file=[%s] __oflag=[0%x]\n", __fd, __file, __oflag);

	org_ret = org_openat64(__fd, __file, __oflag);

	DEBUG_FUNCOUT("org_ret=[%d]\n", org_ret);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}


/************************************************
 * hooked close
 ************************************************/
int close(int __fd)
{
	int ret = 0;
	int org_ret = 0;
	int regflag = 0;
	int inflag = 0;

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__fd=[%d]\n", __fd);

	regflag = isRegularFileByFd(__fd);
	inflag  = isIncoming(__fd);

	if (fsync(__fd) < 0) {  // disk flush
		_PERROR("@@@fsync");
	}

	// delete file descriptor from server
	if (regflag) {
		delPath2Store(__fd);
	}

	// always call original close() to release file descriptor.
	org_ret = org_close(__fd);

	DEBUG_FUNCOUT("org_ret=[%d] __fd=[%d]\n", org_ret, __fd);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}


#ifdef FOPEN_FCLOSE
FILE* fopen(__const char *__restrict __filename, __const char *__restrict __modes)
{
	FILE* org_ret = NULL;

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__filename=[%s] __modes=[%s]\n", __filename, __modes);

	//DEBUG_PRINT("--[before org_fopen] __filename=[%s] __modes=[%s]\n", __filename, __modes);
	org_ret = org_fopen(__filename, __modes);
	//DEBUG_PRINT("--[after  org_fopen] org_ret=[%p]\n", org_ret);

	DEBUG_FUNCOUT("org_ret=[%p]\n", org_ret);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}

FILE* fopen64(__const char *__restrict __filename, __const char *__restrict __modes)
{
	FILE* org_ret = NULL;

	DEBUG_FUNCIN("********************\n");
	DEBUG_FUNCIN("__filename=[%s] __modes=[%s]\n", __filename, __modes);

	//DEBUG_PRINT("--[before org_fopen] __filename=[%s] __modes=[%s]\n", __filename, __modes);
	org_ret = org_fopen64(__filename, __modes);
	//DEBUG_PRINT("--[after  org_fopen] org_ret=[%p]\n", org_ret);

	DEBUG_FUNCOUT("org_ret=[%p]\n", org_ret);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}

int fclose(FILE *__stream)
{
	int org_ret = 0;

	DEBUG_FUNCIN("********************\n");
	DEBUG_PRINT("--[before org_fclose] __stream=[%p]\n", __stream);
	org_ret = org_fclose(__stream);
	DEBUG_PRINT("--[after  org_fclose] org_ret=[%d]\n", org_ret);
	DEBUG_FUNCOUT("********************\n");
	return org_ret;
}
#endif  // FOPEN_FCLOSE

