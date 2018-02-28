/**
 * @file fdio.c
 * @author Daniel Starke
 * @copyright Copyright 2015-2018 Daniel Starke
 * @see fdio.h
 * @date 2015-02-28
 * @version 2018-02-26
 */
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <libpcf/target.h>
#include <libpcf/fdio.h>

#ifdef PCF_IS_WIN
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else /* ! PCF_IS_WIN */
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#endif /* ! PCF_IS_WIN */


/**
 * Sets the binary/text mode of a given file descriptor.
 *
 * @param[in,out] fd - file descriptor to change
 * @param[in] mode - set this mode
 * @return 1 on success, else 0
 * @see tFdioMode
 */
int fdio_setMode(FILE * fd, tFdioMode mode) {
#if defined(PCF_IS_WIN) && !defined(__CYGWIN__)
	switch (mode) {
	case FDIO_TEXT:
		return (setmode(fileno(fd), O_TEXT) == -1) ? 0 : 1;
		break;
	case FDIO_WTEXT:
		return (setmode(fileno(fd), O_WTEXT) == -1) ? 0 : 1;
		break;
	case FDIO_UTF8:
		return (setmode(fileno(fd), O_U8TEXT) == -1) ? 0 : 1;
		break;
	case FDIO_UTF16:
		return (setmode(fileno(fd), O_U16TEXT) == -1) ? 0 : 1;
		break;
	case FDIO_BINARY:
		return (setmode(fileno(fd), O_BINARY) == -1) ? 0 : 1;
		break;
	}
#else /* ! PCF_IS_WIN */
	/* it is always binary mode on Linux */
	PCF_UNUSED(fd);
	PCF_UNUSED(mode);
	return 1;
#endif /* PCF_IS_WIN */
	return 0;
}


#ifdef PCF_IS_WIN
/**
 * Global mutex to avoid handle inheritance when calling CreateProcess.
 * 
 * @internal
 */
volatile HANDLE _LIBPCF_CREATEPROCESS_MUTEX;
#else /* ! PCF_IS_WIN */
/**
 * Closes file descriptor equal or greater than 3.
 * 
 * @return 1 on success, else 0
 * @internal
 */
int fdio_closeNonDefFds() {
#ifdef F_CLOSEM
	int ret;
	do {
		errno = 0;
		ret = fcntl(3, F_CLOSEM);
	} while (ret == -1 && errno == EINTR);
	return 1;
#else /* ! F_CLOSEM */
#ifdef HAS_CLOSEFROM
	closefrom(3);
	return 1;
#else /* ! HAS_CLOSEFROM */
	DIR * d;
	if ((d = opendir("/proc/self/fd")) == NULL) {
		d = opendir("/dev/fd");
	}
	if (d != NULL) {
		struct dirent * de;
		while ((de = readdir(d))) {
			long l;
			char * e = NULL;
			if (de->d_name[0] == '.') continue;
			errno = 0;
			l = strtol(de->d_name, &e, 10);
			if (errno != 0 || !e || *e) continue;
			const int fd = (int)l;
			if (((long)fd) != l) continue;
			if (fd < 3) continue;
			if (fd == dirfd(d)) continue;
			close(fd);
		}
		closedir(d);
	} else {
		/* fallback */
		int i;
		const int maxFdCount = (int)sysconf(_SC_OPEN_MAX);
		for (i = 3; i < maxFdCount; i++) close(i);
	}
	return 1;
#endif /* ! HAS_CLOSEFROM */
#endif /* ! F_CLOSEM */
}
#endif /* ! PCF_IS_WIN */
