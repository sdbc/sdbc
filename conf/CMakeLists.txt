CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(conf C)

SET(CMAKE_BUILD_TYPE type_of_build)
SET(CMAKE_C_FLAGS "-Werror -O3 -m64 -fPIC -g")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../include/ldap)

SET(SRC_LIST envcfg.c isdir.c showlog.c ldap_auth.c openauth.c decode.c encryptproc.c)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRC_LIST})
