CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(sccli C)

SET(CMAKE_BUILD_TYPE type_of_build)
SET(CMAKE_C_FLAGS "-std=c99 -Wall -O3 -m64 -fPIC -g")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR})

SET(SRC_LIST client.c getls.c sqlc.c sqlinit.c getsvc.c netconnect.c get_clikey.c scpool.c N_get_tpl.c)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRC_LIST})
