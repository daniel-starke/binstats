/**
 * @file fdious.h
 * @author Daniel Starke
 * @copyright Copyright 2013-2018 Daniel Starke
 * @see fdious.c
 * @date 2013-06-16
 * @version 2017-12-03
 */
#ifndef __LIBPCF_FDIOUS_H__
#define __LIBPCF_FDIOUS_H__

#include <stdio.h>
#include <wchar.h>
#include <libpcf/fdio.h>
#include <libpcf/target.h>


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
 * Defines a enumeration of Unicode types.
 */
typedef enum tFdiousUnicode {
	FDIOUS_UTF8, /**< UTF-8 */
	FDIOUS_UTF16LE, /**< UTF-16 little endian */
	FDIOUS_UTF16BE, /**< UTF-16 big endian */
	FDIOUS_UTF32LE, /**< UTF-32 little endian */
	FDIOUS_UTF32BE, /**< UTF-32 big endian */
	FDIOUS_UCS2 /**< UCS-2 */
} tFdiousUnicode;


LIBPCF_DLLPORT int LIBPCF_DECL fdious_getline(wchar_t ** strPtr, int * lenPtr, FILE * fd);
/*LIBPCF_DLLPORT int LIBPCF_DECL fdious_getlineT(wchar_t ** strPtr, int * lenPtr, FILE * fd, const tFdiousUnicode type);*/

#ifdef PCF_IS_WIN
LIBPCF_DLLPORT tFdioPHandle * LIBPCF_DECL fdious_popen(const wchar_t * shellPath, const wchar_t ** shell, const wchar_t * command, FILE * input, const tFdioPMode mode);
LIBPCF_DLLPORT int LIBPCF_DECL fdious_pclose(tFdioPHandle * fd);
#endif /* PCF_IS_WIN */


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_FDIOUS_H__ */
