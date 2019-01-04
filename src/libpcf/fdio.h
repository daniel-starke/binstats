/**
 * @file fdio.h
 * @author Daniel Starke
 * @copyright Copyright 2015-2019 Daniel Starke
 * @see fdios.h
 * @see fdious.h
 * @date 2015-02-15
 * @version 2017-12-03
 */
#ifndef __LIBPCF_FDIO_H__
#define __LIBPCF_FDIO_H__

#include <stdio.h>
#include <libpcf/target.h>
#ifdef PCF_IS_WIN
#include <windows.h>
#else /* ! PCF_IS_WIN */
#include <sys/types.h>
#include <unistd.h>
#endif /* PCF_IS_WIN */


#ifdef __cplusplus
extern "C" {
#endif


#ifndef LIBPCF_DECL
#define LIBPCF_DECL
#endif /* LIBPCF_DECL */

#ifndef LIBPCF_DLLPORT
#define LIBPCF_DLLPORT
#endif /* LIBPCF_DLLPORT */


/**
 * Defines a structure for a fdios_popen or fdious_popen handle.
 * Access field pipe for I/O operations with the child process.
 */
typedef struct tFdioPHandle {
	FILE * in; /**< write file descriptor for to the child process' standard input */
	FILE * out; /**< read file descriptor for to the child process' standard output */
	FILE * err; /**< read file descriptor for to the child process' standard error */
#ifdef PCF_IS_WIN
	HANDLE pid; /**< handle of the child process */
#else /* ! PCF_IS_WIN */
	pid_t pid; /**< handle of the child process */
#endif /* PCF_IS_WIN */
} tFdioPHandle;


/**
 * Defines modes for fdios_popen or fdious_popen.
 * Combine multiple modes via binary or.
 */
typedef enum tFdioPMode {
	FDIO_USE_STDIN   = 0x01, /**< open standard input of child process for writing */
	FDIO_USE_STDOUT  = 0x02, /**< open standard output of child process for reading */
	FDIO_USE_STDERR  = 0x04, /**< open standard error of child process for reading */
	FDIO_BINARY_PIPE = 0x08, /**< open pipes in binary mode */
	FDIO_COMBINE     = 0x10, /**< merge standard error in standard output of child process */
	FDIO_RAW_CMDLINE = 0x20  /**< do not escape command-line arguments (Windows only) */
} tFdioPMode;


/**
 * Defines modes for a file descriptor.
 */
typedef enum tFdioMode {
	FDIO_TEXT, /**< char mode */
	FDIO_WTEXT, /**< wchar_t mode, defaults to binary on other systems than Windows */
	FDIO_UTF8, /**< UTF-8 transformation, defaults to binary on other systems than Windows */
	FDIO_UTF16, /**< UTF-16 transformation, defaults to binary on other systems than Windows */
	FDIO_BINARY /**< binary mode */
} tFdioMode;


LIBPCF_DLLPORT int LIBPCF_DECL fdio_setMode(FILE * fd, tFdioMode mode);


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_FDIO_H__ */
