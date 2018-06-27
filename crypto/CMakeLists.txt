CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(crypto C)

SET(CMAKE_BUILD_TYPE type_of_build)
SET(CMAKE_C_FLAGS "-Werror -O3 -m64 -fPIC -g")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR})

SET(SRC_LIST bignum.c crc32.c expmod.c fcrypt.c md5_dgst.c a64n.c crc.c family.c fcrypt_b.c md5_one.c mulmod.c set_key.c enigma.c enigma2.c prikey128.c quicklz.c mac.c fingerprint.c)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRC_LIST})
