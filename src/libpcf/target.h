/**
 * @file target.h
 * @author Daniel Starke
 * @copyright Copyright 2017-2019 Daniel Starke
 * @date 2017-12-02
 * @version 2017-12-05
 */
#ifndef __LIBPCF_TARGET_H__
#define __LIBPCF_TARGET_H__


#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>


#ifndef PCF_PI
/** Defines double PI constant. */
#define PCF_PI 3.14159265358979323846
#endif


#ifndef PCF_DEG_TO_RAD
/** Converts the given value from degrees to radians in the specified type. */
#define PCF_DEG_TO_RAD(type, x) (type(x) * type(PCF_PI) / type(180))
#endif


#ifndef PCF_RAD_TO_DEG
/** Converts the given value from radians to degrees in the specified type. */
#define PCF_RAD_TO_DEG(type, x) (type(x) * type(180) / type(PCF_PI))
#endif


#ifndef PCF_TO_STR
/** Converts the passed argument to a string literal. */
# define PCF_TO_STR(x) #x
#endif /* to string */


#if defined(__WIN32__) || defined(__WIN64__) || defined(WIN32) \
 || defined(WINNT) || defined(_WIN32) || defined(__WIN32) || defined(__WINNT) \
 || defined(__MINGW32__) || defined(__MINGW64__)
# ifndef PCF_IS_WIN
/** Defined if compiler target is windows. */
#  define PCF_IS_WIN 1
# endif
# undef PCF_IS_NO_WIN
#else /* no windows */
# ifndef PCF_IS_NO_WIN
/** Defined if compiler target is _not_ windows. */
#  define PCF_IS_NO_WIN 1
# endif
# undef PCF_IS_WIN
#endif /* windows */


#if !defined(PCF_IS_WIN) && (defined(unix) || defined(__unix) || defined(__unix__) \
 || defined(__gnu_linux__) || defined(linux) || defined(__linux) \
 || defined(__linux__))
# ifndef PCF_IS_LINUX
/** Defined if compiler target is linux/unix. */
# define PCF_IS_LINUX 1
# endif
# undef PCF_IS_NO_LINUX
#else /* no linux */
# ifndef PCF_IS_NO_LINUX
/** Defined if compiler target is _not_ linux/unix. */
#  define PCF_IS_NO_LINUX 1
# endif
# undef PCF_IS_LINUX
#endif /* linux */


#if defined(_X86_) || defined(i386) || defined(__i386__)
# ifndef PCF_IS_X86
/** Defined if compiler target is x86. */
# define PCF_IS_X86
# endif
#else /* no x86 */
# ifndef PCF_IS_NO_X86
/** Defined if compiler target _not_ is x86. */
# define PCF_IS_NO_X86
# endif
#endif /* x86 */


#if defined(__amd64__) || defined(__x86_64__)
# ifndef PCF_IS_X64
/** Defined if compiler target is x64. */
# define PCF_IS_X64
# endif
#else /* no x64 */
# ifndef PCF_IS_NO_X64
/** Defined if compiler target _not_ is x64. */
# define PCF_IS_NO_X64
# endif
#endif /* x64 */


#ifdef PCF_PATH_SEP
# undef PCF_PATH_SEP
#endif
#ifdef PCF_IS_WIN
/** Defines the Windows path separator. */
# define PCF_PATH_SEP "\\"
# define PCF_PATH_SEPU L"\\"
#else /* PCF_IS_NO_WIN */
/** Defines the non-Windows path separator. */
# define PCF_PATH_SEP "/"
# define PCF_PATH_SEPU L"/"
#endif /* PCF_IS_WIN */


/* check whether type is 32-bit aligned */
#ifndef PCF_UNALIGNED_P32
# if __GNUC__ >= 2
#  define PCF_UNALIGNED_P32(p) (((uintptr_t)p) % __alignof__(uint32_t) != 0)
# elif _MSC_VER
#  define PCF_UNALIGNED_P32(p) (((uintptr_t)p) % __alignof(uint32_t) != 0)
# else
#  define alignof(type) offsetof(struct {char c; type x;}, x)
#  define PCF_UNALIGNED_P32(p) (((int)p) % alignof(uint32_t) != 0)
# endif
#endif /* PCF_UNALIGNED_P32 */


/* check whether type is 64-bit aligned */
#ifndef PCF_UNALIGNED_P64
# if __GNUC__ >= 2
#  define PCF_UNALIGNED_P64(p) (((uintptr_t)p) % __alignof__(uint64_t) != 0)
# elif _MSC_VER
#  define PCF_UNALIGNED_P64(p) (((uintptr_t)p) % __alignof(uint64_t) != 0)
# else
#  define alignof(type) offsetof(struct {char c; type x;}, x)
#  define PCF_UNALIGNED_P64(p) (((int)p) % alignof(uint64_t) != 0)
# endif
#endif /* PCF_UNALIGNED_P64 */


/* restrict pointer excess, see C99 restrict */
#ifndef PCF_RESTRICT
#if defined(__clang__)
# define PCF_RESTRICT __restrict
#elif defined(__ICC) && __ICC > 1110
# define PCF_RESTRICT __restrict
#elif defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
# define PCF_RESTRICT __restrict
#elif defined(_MSC_VER) && _MSC_VER >= 1400
# define PCF_RESTRICT __restrict
#else
# define PCF_RESTRICT
#endif
#endif /* PCF_RESTRICT */


/* align variable like: float PCF_ALIGNAS(8) myVar64Aligned; */
#ifndef PCF_ALIGNAS
#if defined(__clang__)
# define PCF_ALIGNAS(bytes) __attribute__((aligned(bytes * 8)))
#elif defined(__ICC) && __ICC > 1110
# define PCF_ALIGNAS(bytes) __declspec(align(bytes * 8))
#elif defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
# define PCF_ALIGNAS(bytes) __attribute__((aligned(bytes * 8)))
#elif defined(_MSC_VER) && _MSC_VER >= 1400
# define PCF_ALIGNAS(bytes) __declspec(align(bytes * 8))
#else
# define PCF_ALIGNAS(bytes)
#endif
#endif /* PCF_ALIGNAS */


/* assume passed pointer is aligned to given size */
#ifndef PCF_ASSUME_ALIGNED_AS
#if defined(__clang__) && ((__clang_major__ > 3) || (__clang_major__ == 3 && __clang_minor__ >= 6))
# define PCF_ASSUME_ALIGNED_AS(src, bytes) __builtin_assume(((ptrdiff_t)(src) % (ptrdiff_t)(bytes)) == 0)
#elif defined(__ICC) && __ICC > 1110
# define PCF_ASSUME_ALIGNED_AS(src, bytes) __assume_aligned(src, bytes)
#elif defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
/* broken in gcc <= 5.0.0 as of 2015-01-09 */
# define PCF_ASSUME_ALIGNED_AS(src, bytes) if ((ptrdiff_t)(src) % (ptrdiff_t)(bytes) != 0) __builtin_unreachable()
#elif defined(_MSC_VER) && _MSC_VER >= 1400
# define PCF_ASSUME_ALIGNED_AS(src, bytes) __assume(((ptrdiff_t)(src) % (ptrdiff_t)(bytes)) == 0)
#else
# define PCF_ASSUME_ALIGNED_AS(src, bytes)
#endif
#endif /* PCF_ASSUME_ALIGNED_AS */


/* assume passed expression */
#ifndef PCF_ASSUME
#if defined(__clang__) && ((__clang_major__ > 3) || (__clang_major__ == 3 && __clang_minor__ >= 6))
# define PCF_ASSUME(expr) __builtin_assume(expr)
#elif defined(__ICC) && __ICC > 1110
# define PCF_ASSUME(expr) __assume(expr)
#elif defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
# define PCF_ASSUME(expr) if ( ! (expr) ) __builtin_unreachable();
#elif defined(_MSC_VER) && _MSC_VER >= 1400
# define PCF_ASSUME(expr) __assume(expr)
#else
# define PCF_ASSUME(expr)
#endif
#endif /* PCF_ASSUME */


/* packed structure */
#ifndef PCF_PACKED_START
#if defined(__clang__) || defined(__GNUC__)
# define PCF_PACKED_START(x) x
# define PCF_PACKED_END __attribute__((__packed__)) ;
#elif defined(_MSC_VER)
# define PCF_PACKED_START(x) __pragma(pack(push, 1)) x
# define PCF_PACKED_END ; __pragma(pack(pop))
#else
# define PCF_PACKED_START(x) x
# define PCF_PACKED_END ;
#endif
#endif /* PCF_PACKED_START */


/* conditional OpenMP pragma */
#ifndef PCF_DO_OMP
#ifdef _OPENMP
# if defined(__clang__)
#  define PCF_DO_OMP(x) _Pragma(TO_STR(omp x))
# elif defined(__ICC) && __ICC > 1110
#  define PCF_DO_OMP(x) _Pragma(TO_STR(omp x))
# elif defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#  define PCF_DO_OMP(x) _Pragma(TO_STR(omp x))
# elif defined(_MSC_VER) && _MSC_VER >= 1400
#  define PCF_DO_OMP(x) __pragma(omp x)
# else
#  define PCF_DO_OMP(x)
# endif
/* version specific OpenMP pragmas */
# if _OPENMP >= 199810 /* 1.0 */
#  define PCF_DO_OMPv10(x) PCF_DO_OMP(x)
# endif
# if _OPENMP >= 200203 /* 2.0 */
#  define PCF_DO_OMPv20(x) PCF_DO_OMP(x)
# endif
# if _OPENMP >= 200505 /* 2.5 */
#  define PCF_DO_OMPv25(x) PCF_DO_OMP(x)
# endif
# if _OPENMP >= 200805 /* 3.0 */
#  define PCF_DO_OMPv30(x) PCF_DO_OMP(x)
# endif
# if _OPENMP >= 201107 /* 3.1 */
#  define PCF_DO_OMPv31(x) PCF_DO_OMP(x)
# endif
# if _OPENMP >= 201307 /* 4.0 */
#  define PCF_DO_OMPv40(x) PCF_DO_OMP(x)
# endif
#else
# define PCF_DO_OMP(x)
#endif
#endif /* PCF_DO_OMP */

/* version specific OpenMP pragmas */
#ifndef PCF_DO_OMPv10
# define PCF_DO_OMPv10(x)
#endif
#ifndef PCF_DO_OMPv20
# define PCF_DO_OMPv20(x)
#endif
#ifndef PCF_DO_OMPv25
# define PCF_DO_OMPv25(x)
#endif
#ifndef PCF_DO_OMPv30
# define PCF_DO_OMPv30(x)
#endif
#ifndef PCF_DO_OMPv31
# define PCF_DO_OMPv31(x)
#endif
#ifndef PCF_DO_OMPv40
# define PCF_DO_OMPv40(x)
#endif


/* byte swapping 32-bit */
#ifndef PCF_BYTE_SWAP32
#if __GNUC__ >= 2 && (defined(PCF_IS_X86) || defined(PCF_IS_X64))
static inline uint32_t byteSwap32(uint32_t x) {
	__asm__("bswapl %0" : "=r" (x) : "0" (x));
	return x;
}
#define PCF_BYTE_SWAP32(n) byteSwap32(n)
#else
#define PCF_BYTE_SWAP32(n) (((n) << 24) | (((n) & 0xFF00) << 8) | (((n) >> 8) & 0xFF00) | ((n) >> 24))
#endif
#endif /* PCF_BYTE_SWAP32 */


/* byte swapping 64-bit */
#ifndef PCF_BYTE_SWAP64
#if __GNUC__ > 2 && (defined(PCF_IS_X86) || defined(PCF_IS_X64))
#if defined(PCF_IS_X64)
static inline uint64_t byteSwap64(uint64_t x) {
	__asm__("bswapq %0" : "=r" (x) : "0" (x));
	return x;
}
#else /* no x64 */
static inline uint64_t byteSwap64(const uint64_t x) {
	uint32_t l, h;
	__asm__("bswapl %0" : "=r" (l) : "0" ((const uint32_t)(x & 0xFFFFFFFF)));
	__asm__("bswapl %0" : "=r" (h) : "0" ((const uint32_t)(x >> 32)));
	return ((uint64_t)h) | (((uint64_t)l) << 32);
}
#endif
#define PCF_BYTE_SWAP64(n) byteSwap64(n)
#else
/* reference: http://blogs.sun.com/DanX/entry/optimizing_byte_swapping_for_fun */
#define PCF_BYTE_SWAP64(n) (((n) << 56) | (((n) << 40) & UINT64_C(0xFF000000000000)) | \
	(((n) << 24) & UINT64_C(0xFF0000000000)) | (((n) << 8) & UINT64_C(0xFF00000000)) | \
	(((n) >> 8) & UINT64_C(0xFF000000)) | (((n) >> 24) & UINT64_C(0xFF0000)) | \
	(((n) >> 40) & UINT64_C(0xFF00)) | ((n) >> 56))
#endif
#endif /* PCF_BYTE_SWAP64 */


/* most significant bit 32-bit */
#ifndef PCF_LEADING_ZEROS32
#if __GNUC__ > 2 && defined(PCF_IS_X86) && ( ! defined(LIBPCF_PORTABLE) )
static inline uint32_t leadingZeros32(const uint32_t x) {
	uint32_t r;
	if (x == 0) return 32;
	__asm__("bsrl %1, %0" : "=r" (r) : "r" (x));
	return 31 - r;
}
#else
/* reference: http://stackoverflow.com/questions/671815/what-is-the-fastest-most-efficient-way-to-find-the-highest-set-bit-msb-in-an-i */
static inline uint32_t leadingZeros32(const uint32_t x) {
	uint32_t r;
	if (x == 0) return 32;
	static const uint32_t lookupTable[32] = {
		 0,  1, 28,  2, 29, 14, 24,  3,
		30, 22, 20, 15, 25, 17,  4,  8,
		31, 27, 13, 23, 21, 19, 16,  7,
		26, 12, 18,  6, 11,  5, 10,  9
	};
	r = x | (x >> 1);
	r |= (r >> 2);
	r |= (r >> 4);
	r |= (r >> 8);
	r |= (r >> 16);
	r = (r >> 1) + 1;
	return 31 - lookupTable[(r * UINT32_C(0x077CB531)) >> 27];
}
#endif
#define PCF_LEADING_ZEROS32(n) leadingZeros32(n)
#endif /* PCF_LEADING_ZEROS32 */


/* most significant bit 64-bit */
#ifndef PCF_LEADING_ZEROS64
#if __GNUC__ > 2 && defined(PCF_IS_X64) && ( ! defined(LIBPCF_PORTABLE) )
static inline uint64_t leadingZeros64(const uint64_t x) {
	uint64_t r;
	if (x == 0) return 64;
	__asm__("bsrq %1, %0" : "=r" (r) : "r" (x));
	return 63 - r;
}
#else
/* reference: https://chessprogramming.wikispaces.com/De+Bruijn+Sequence+Generator */
static inline uint64_t leadingZeros64(const uint64_t x) {
	uint64_t r;
	if (x == 0) return 64;
	static const uint64_t lookupTable[64] = {
		 0,  1,  2, 53,  3,  7, 54, 27,
		 4, 38, 41,  8, 34, 55, 48, 28,
		62,  5, 39, 46, 44, 42, 22,  9,
		24, 35, 59, 56, 49, 18, 29, 11,
		63, 52,  6, 26, 37, 40, 33, 47,
		61, 45, 43, 21, 23, 58, 17, 10,
		51, 25, 36, 32, 60, 20, 57, 16,
		50, 31, 19, 15, 30, 14, 13, 12
	};
	r = x | (x >> 1);
	r |= (r >> 2);
	r |= (r >> 4);
	r |= (r >> 8);
	r |= (r >> 16);
	r |= (r >> 32);
	r = (r >> 1) + 1;
	return 63 - lookupTable[(r * UINT64_C(0x022FDD63CC95386D)) >> 58];
}
#endif
#define PCF_LEADING_ZEROS64(n) leadingZeros64(n)
#endif /* PCF_LEADING_ZEROS64 */


/* bit rotation to the left 32-bit */
#ifndef PCF_ROTATE_LEFT32
#if __GNUC__ > 2 && (defined(PCF_IS_X86) || defined(PCF_IS_X64))
static inline uint32_t rotateLeft32(uint32_t x, const uint8_t n) {
	__asm__("roll %1,%0" : "+r" (x) : "c" (n));
	return x;
}
#define PCF_ROTATE_LEFT32(x, n) rotateLeft32(x, n)
#else
#define PCF_ROTATE_LEFT32(x, n) (((uint32_t)((x) << (n))) | ((uint32_t)((x) >> (32 - (n)))))
#endif
#endif /* PCF_ROTATE_LEFT32 */


/* bit rotation to the left 64-bit */
#ifndef PCF_ROTATE_LEFT64
#if __GNUC__ > 2 && defined(PCF_IS_X64)
static inline uint64_t rotateLeft64(uint64_t x, const uint8_t n) {
	__asm__("rolq %1,%0" : "+r" (x) : "c" (n));
	return x;
}
#define PCF_ROTATE_LEFT64(x, n)  rotateLeft64(x, n)
#else
#define PCF_ROTATE_LEFT64(x, n) (((uint64_t)((x) << (n))) | ((uint64_t)((x) >> (64 - (n)))))
#endif
#endif /* PCF_ROTATE_LEFT64 */


/* bit rotate to the right 32-bit */
#ifndef PCF_ROTATE_RIGHT32
#if __GNUC__ > 2 && (defined(PCF_IS_X86) || defined(PCF_IS_X64))
static inline uint32_t rotateRight32(uint32_t x, const uint8_t n) {
	__asm__("rorl %1,%0" : "+r" (x) : "c" (n));
	return x;
}
#define PCF_ROTATE_RIGHT32(x, n) rotateRight32(x, n)
#else
#define PCF_ROTATE_RIGHT32(x, n) (((uint32_t)((x) >> (n))) | ((uint32_t)((x) << (32 - (n)))))
#endif
#endif /* PCF_ROTATE_RIGHT32 */


/* bit rotate to the right 64-bit */
#ifndef PCF_ROTATE_RIGHT64
#if __GNUC__ > 2 && defined(PCF_IS_X64)
static inline uint64_t rotateRight64(uint64_t x, const uint8_t n) {
	__asm__("rorq %1,%0" : "+r" (x) : "c" (n));
	return x;
}
#define PCF_ROTATE_RIGHT64(x, n) rotateRight64(x, n)
#else
#define PCF_ROTATE_RIGHT64(x, n) (((uint64_t)((x) >> (n))) | ((uint64_t)((x) << (64 - (n)))))
#endif
#endif /* PCF_ROTATE_RIGHT64 */


#define PCF_MIN(x, y) ((x) > (y) ? (y) : (x))
#define PCF_MAX(x, y) ((x) >= (y) ? (x) : (y))


/* suppress unused parameter warning */
#ifdef _MSC_VER
# define PCF_UNUSED(x)
#else /* not _MSC_VER */
# define PCF_UNUSED(x) (void)x;
#endif /* not _MSC_VER */


/* Windows workarounds */
#ifdef PCF_IS_WIN
# define fileno _fileno
# define fdopen _fdopen
# define setmode _setmode
# define get_osfhandle _get_osfhandle
# define open_osfhandle _open_osfhandle
# define O_TEXT _O_TEXT
# define O_WTEXT _O_WTEXT
# define O_U8TEXT _O_U8TEXT
# define O_U16TEXT _O_U16TEXT
# define O_BINARY _O_BINARY
# define O_RDONLY _O_RDONLY
#endif


/* MSVS workarounds */
#ifdef _MSC_VER
# define snprintf _snprintf
# define snwprintf _snwprintf
# define vsnprintf _vsnprintf
# define vsnwprintf _vsnwprintf
# define va_copy(dest, src) (dest = src)
#endif /* _MSC_VER */


/* Cygwin workaround */
#ifdef __CYGWIN__
# define O_U8TEXT 0x00040000
# define O_U16TEXT 0x00020000
#endif /* __CYGWIN__ */


#ifndef HAS_STRICMP
#define HAS_STRICMP
static int stricmp(const char * lhs, const char * rhs) {
	for (; *lhs && tolower(*lhs) == tolower(*rhs); lhs++, rhs++)
	if (*lhs == 0) return 0;
	return *((const unsigned char *)lhs) - *((const unsigned char *)rhs);
}
#endif /* HAS_NO_STRICMP */


#endif /* __LIBPCF_TARGET_H__ */
