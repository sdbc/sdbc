/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1992 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Functions for computing 32-bit CRC.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

/* RCSID("$OpenBSD: crc32.h,v 1.10 2001/03/02 18:54:31 deraadt Exp $"); */

#ifndef CRC32_H
#define CRC32_H

#include <sys/types.h>
/*
 * This computes a 32 bit CRC of the data in the buffer, and returns the CRC.
 * The polynomial used is 0xedb88320.
 */
#ifdef __cplusplus
extern "C" {
#endif

u_int32_t ssh_crc32(const u_int8_t *buf, u_int32_t len);

#ifdef __cplusplus
}
#endif
#endif				/* CRC32_H */
