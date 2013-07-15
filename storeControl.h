#ifndef _STORE_CONTROL_H
#define _STORE_CONTROL_H

int isRegularFileByPath(const char *path, int flags);
int isRegularFileByFd(int fd);
int isOutgoing(int flags);
int isIncoming(int fd);

int isExclusionFile(const char *path);
int isExclusionDir(const char *path);

#endif  // _STORE_CONTROL_H
