/**
 * @file fdios.h
 * @author Daniel Starke
 * @copyright Copyright 2013-2023 Daniel Starke
 * @see fdios.c
 * @date 2013-06-15
 * @version 2017-12-03
 */
#ifndef __LIBPCF_FDIOS_H__
#define __LIBPCF_FDIOS_H__

#include <stdio.h>
#include <libpcf/fdio.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifndef LIBPCF_DECL
#define LIBPCF_DECL
#endif /* LIBPCF_DECL */

#ifndef LIBPCF_DLLPORT
#define LIBPCF_DLLPORT
#endif /* LIBPCF_DLLPORT */


LIBPCF_DLLPORT int LIBPCF_DECL fdios_getline(char ** strPtr, int * lenPtr, FILE * fd);
LIBPCF_DLLPORT tFdioPHandle * LIBPCF_DECL fdios_popen(const char * shellPath, const char ** shell, const char * command, FILE * input, const tFdioPMode mode);
LIBPCF_DLLPORT int LIBPCF_DECL fdios_pclose(tFdioPHandle * fd);


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_FDIOS_H__ */
