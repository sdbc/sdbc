/* libsqlora8-config.h
 * libsqlora8-config.h.  Generated from libsqlora8-config.h.in by configure.
 */

#ifndef LIBSQLORA8_CONFIG_H
#define LIBSQLORA8_CONFIG_H

#define LIBSQLORA8_MAJOR_VERSION 2
#define LIBSQLORA8_MINOR_VERSION 3
#define LIBSQLORA8_MICRO_VERSION 3
#define LIBSQLORA8_INTERFACE_AGE 1
#define LIBSQLORA8_BINARY_AGE    3

#define LIBSQLORA8_VERSION "2.3.3"

/* This is set to 1 if the library was compiled with a thread package */
#define LIBSQLORA8_THREADED 0

/* If LIBSQLORA8_THREADED is 1, this define tells you which package was used 
 * It is either "posix" or "oracle"
 */
#define LIBSQLORA8_THREADS ""

#ifdef __MINGW32__
#define _int64 __int64
#endif

#endif
