/**
 * @file fdios.c
 * @author Daniel Starke
 * @copyright Copyright 2016-2023 Daniel Starke
 * @see fdios.h
 * @date 2016-11-13
 * @version 2017-12-03
 */
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libpcf/fdios.h>


/**
 * Defines the size with which a buffer will be increased.
 */
#define BUFFER_INC_SIZE 128


/**
 * The function reads a line and sets the passed string pointer.
 * New space is allocated if the string can not hold more characters.
 *
 * @param[in,out] strPtr - points to the output string
 * @param[in,out] lenPtr - pointer to the size of the input string in characters
 * @param[in] fd - file descriptor to read from
 * @return number of characters read from file descriptor
 */
int fdios_getline(char ** strPtr, int * lenPtr, FILE * fd) {
	char * buf, * bufPos, * resStr;
	int totalLen, size, toRead, res;
	if (strPtr == NULL || lenPtr == NULL || fd == NULL) return -1;
	if (*strPtr == NULL || *lenPtr <= 0) {
		*strPtr = (char *)malloc(sizeof(char) * BUFFER_INC_SIZE);
		if (*strPtr == NULL) {
			return -1;
		}
		**strPtr = 0;
		*lenPtr = BUFFER_INC_SIZE;
	}
	buf = *strPtr;
	bufPos = buf;
	size = *lenPtr;
	toRead = *lenPtr;
	totalLen = 0;
	while ((resStr = fgets(bufPos, toRead, fd)) != NULL) {
		res = (int)strlen(bufPos);
		totalLen += res;
		if ((res + 1) == toRead && bufPos[res - 1] != '\n') {
			size += BUFFER_INC_SIZE;
			resStr = (char *)realloc(buf, sizeof(char) * (size_t)size);
			if (resStr == NULL) {
				if (buf != NULL) free(buf);
				*strPtr = NULL;
				*lenPtr = 0;
				return -1;
			}
			buf = resStr;
			toRead = BUFFER_INC_SIZE + 1;
			bufPos = buf + totalLen;
		} else {
			break;
		}
	}
	if (resStr == NULL) {
		if (feof(fd) != 0) {
			*buf = 0;
			totalLen = 0;
		} else {
			totalLen += (int)strlen(bufPos);
		}
	}
	*strPtr = buf;
	*lenPtr = size;
	return totalLen;
}


#define FPOPEN_FUNC fdios_popen
#define FPCLOSE_FUNC fdios_pclose
#undef FPOPEN_UNICODE

/* include template function */
#include "fdio.i"
