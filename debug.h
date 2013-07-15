#ifndef _DEBUG_H
#define _DEBUG_H


#include <stdio.h>			// perror
#include <errno.h>			// int errno
#include <strings.h>		// strerror
#include <sys/types.h>		// getpid
#include <unistd.h>			// getpid
#include <syslog.h>			// syslog


#ifdef DEBUG
  #define _LOG_TRACE(fmt, ...) syslog(LOG_DEBUG, "[%d] " fmt, getpid(), ## __VA_ARGS__)
  #define _LOG_FUNCIN(fmt, ...) syslog(LOG_DEBUG, "[%d] >>>%s start>>> " fmt, getpid(), __func__, ## __VA_ARGS__)
  #define _LOG_FUNCOUT(fmt, ...) syslog(LOG_DEBUG, "[%d] <<<%s end<<< " fmt, getpid(), __func__, ## __VA_ARGS__)
#else
  #define _LOG_TRACE(fmt, ...)
  #define _LOG_FUNCIN(fmt, ...)
  #define _LOG_FUNCOUT(fmt, ...)
#endif

#define _LOG_NOTICE(fmt, ...) syslog(LOG_NOTICE, "[%d] " fmt, getpid(), ## __VA_ARGS__)
#define _LOG_ERROR(fmt, ...) syslog(LOG_ERR, "[%d] " fmt, getpid(), ## __VA_ARGS__)
#define _LOG_PERROR(msg) syslog(LOG_ERR, "[%d] " msg ": %s", getpid(), strerror(errno))


#ifdef DEBUG
  #ifdef DEBUG_LOG
    #define DEBUG_PRINT(...)  _LOG_TRACE(__VA_ARGS__)
    #define DEBUG_TRACE(fmt, ...) _LOG_TRACE(fmt, ## __VA_ARGS__)
    #define DEBUG_FUNCIN(fmt, ...) _LOG_FUNCIN(fmt, ## __VA_ARGS__)
    #define DEBUG_FUNCOUT(fmt, ...) _LOG_FUNCOUT(fmt, ## __VA_ARGS__)
  #else
    #define DEBUG_PRINT(...)  printf(__VA_ARGS__)
    #define DEBUG_TRACE(fmt, ...) printf("[%d] " fmt, getpid(), ## __VA_ARGS__)
    #define DEBUG_FUNCIN(fmt, ...) printf("[%d] >>>%s start>>> " fmt, getpid(), __func__, ## __VA_ARGS__)
    #define DEBUG_FUNCOUT(fmt, ...) printf("[%d] <<<%s end<<< " fmt, getpid(), __func__, ## __VA_ARGS__)
  #endif
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_TRACE(fmt, ...)
  #define DEBUG_FUNCIN(fmt, ...)
  #define DEBUG_FUNCOUT(fmt, ...)
#endif

#ifdef DEBUG_LOG
  #define ERROR_PRINT(...)  _LOG_ERROR(__VA_ARGS__)
  #define _PERROR(msg)  _LOG_PERROR(msg)
#else
  #define ERROR_PRINT(...)  fprintf(stderr, __VA_ARGS__)
  #define _PERROR(msg)  perror(msg)
#endif


// perror(s) is the same as fprintf(stderr, "%s: %s\n", s, strerror(errno))


#endif
