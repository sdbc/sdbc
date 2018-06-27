CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(scsrv C)

SET(CMAKE_BUILD_TYPE type_of_build)
SET(CMAKE_C_FLAGS "-Werror -O3 -m64 -fPIC -g")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR})

SET(SRC_LIST chkexec.c echo.c filelist.c rexec.c PutEvent.c return_error.c get_srvname.c PPC_srv.c TPC_srv.c clikey.c set_showid.c Function.c tpool.c mod_sc.c SIO_fd.c dmapp.c dm_app.c)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRC_LIST})
