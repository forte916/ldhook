#include <stdio.h>
#include <string.h>		// gnu basename
#include <fcntl.h>		// open, fcntl
#include <sys/types.h>	// stat
#include <sys/stat.h>	// stat
#include <unistd.h>		// close, read, write, fcntl, stat

#include "debug.h"
#include "storeMessage.h"
#include "storeControl.h"

// exclude at least these files
char *excludeFileList[] = {".", "README", "ldhook.c", "stored.c", NULL};

// exclude at least these directories
char *excludeDirList[] = {"/var/log", "/proc", NULL};


/**
 *  @brief isRegularFileByPath
 *  @param path_of_file
 *  @param file status flags
 *  @return 1:regular file or 0:not a regular file
 */
int isRegularFileByPath(const char *path, int flags)
{
	DEBUG_FUNCIN("\n");
	struct stat buf;

	DEBUG_PRINT("  ##path = %s, flags = 0%o\n", path, flags);
	if (stat(path, &buf) < 0) {
		_PERROR("@@@stat");
		DEBUG_PRINT("  ##flags = 0%o, O_CREAT = 0%o\n", flags, O_CREAT);
		if (flags & O_CREAT) {
			DEBUG_PRINT("  ##creat file!!\n");
			return 1;
		} else {
			DEBUG_PRINT("  ##not creat file!!\n");
			return 0;
		}
	}

	if (S_ISREG(buf.st_mode)) {
		DEBUG_PRINT("  ##regular file!!\n");
		return 1;
	}

	DEBUG_PRINT("  ##not a regular file!!\n");
	DEBUG_FUNCOUT("\n");
	return 0;
}


/**
 *  @brief isRegularFileByFd
 *  @param file descriptor
 *  @return 1:regular file or 0:not a regular file
 */
int isRegularFileByFd(int fd)
{
	DEBUG_FUNCIN("\n");
	struct stat buf;

	DEBUG_PRINT("  ##fd = %d\n", fd);
	if (fstat(fd, &buf) < 0) {
		_PERROR("@@@fstat");
		return 0;
	}

	if (S_ISREG(buf.st_mode)) {
		DEBUG_PRINT("  ##regular file!!\n");
		return 1;
	}

	DEBUG_PRINT("  ##not a regular file!!\n");
	DEBUG_FUNCOUT("\n");
	return 0;
}


/**
 *  @brief if flags includes O_RDONLY, it regards as outgoing
 *  @param file status flags
 *  @return 1:outgoing or 0:not outgoing
 */
int isOutgoing(int flags)
{
	DEBUG_FUNCIN("\n");

	DEBUG_PRINT("  ##flags = 0%o, O_ACCMODE = 0%o, O_RDONLY = 0%o\n", flags, O_ACCMODE, O_RDONLY);
	if ((flags & O_ACCMODE) == O_RDONLY) {
		DEBUG_PRINT("  ##outgoing file!!\n");
		return 1;
	}

	DEBUG_PRINT("  ##not outgoing file!!\n");
	DEBUG_FUNCOUT("\n");
	return 0;
}


/**
 *  @brief if flags don't includes O_RDONLY, it regards as incoming
 *  @param file descriptor
 *  @return 1:incoming or 0:not incoming
 */
int isIncoming(int fd)
{
	int flags;

	DEBUG_FUNCIN("\n");
	DEBUG_PRINT("  ##fd = %d\n", fd);

	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		_PERROR("@@@fcntl");
		return 0;
	}

	DEBUG_PRINT("  ##flags = 0%o, O_ACCMODE = 0%o, O_RDONLY = 0%o\n", flags, O_ACCMODE, O_RDONLY);
	if ((flags & O_ACCMODE) != O_RDONLY) {
		DEBUG_PRINT("  ##incoming file!!\n");
		return 1;
	}

	DEBUG_PRINT("  ##not incoming file!!\n");
	DEBUG_FUNCOUT("\n");
	return 0;
}


/**
 *  @brief isExclusionFile
 *         don't store specified files
 *  @param path_of_file
 *  @return 1:need to store or 0:need not store
 */
int isExclusionFile(const char *path)
{
	DEBUG_FUNCIN("\n");
	char *bname = NULL;
	int i = 0;

	bname = basename(path);
	DEBUG_PRINT("  basename = %s, path = %s\n", bname, path);

	for (i = 0; excludeFileList[i] != NULL; i++) {
		//DEBUG_PRINT("  excludeFileList[%d] = %s\n", i, excludeFileList[i]);
		if (strcmp(bname, excludeFileList[i]) == 0) {
			return 0;
		}
	}

	DEBUG_FUNCOUT("\n");
	return 1;
}


/**
 *  @brief isExclusionDir
 *         don't store specified directories
 *  @param fullpath
 *  @return 1:need to store or 0:need not store
 */
int isExclusionDir(const char *path)
{
	DEBUG_FUNCIN("\n");
	char buf[MAX_PATH_LEN];
	char *dname = NULL;
	int i = 0;

	strcpy(buf, path);
	dname = dirname(buf);
	DEBUG_PRINT("  dirname = %s, path = %s\n", dname, path);

	for (i = 0; excludeDirList[i] != NULL; i++) {
		DEBUG_PRINT("  excludeDirList[%d] = %s\n", i, excludeDirList[i]);
		if (strcmp(dname, excludeDirList[i]) == 0) {
			return 0;
		}
	}

	DEBUG_FUNCOUT("\n");
	return 1;
}


