#ifndef SCRYPT_H
#define SCRYPT_H

#include <bignum.h>
#include <crc32.h>

#ifdef __cplusplus
extern "C" {
#endif
		
char * prikey128(char *keybuf,u_int ind[4],u_int *family);
extern u_int family[];

int get_mac(char* out);
int fingerprint(char *out);

// Public functions of QuickLZ
size_t qlz_size_decompressed(const char *source);
size_t qlz_size_compressed(const char *source);
size_t qlz_compress(const void *source, char *destination, size_t size);
size_t qlz_decompress(const char *source, void *destination);
int qlz_get_setting(int setting);

#ifdef __cplusplus
}
#endif

#endif

