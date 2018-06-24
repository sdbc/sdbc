CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(pack C)

SET(CMAKE_BUILD_TYPE type_of_build)
SET(CMAKE_C_FLAGS "-Werror -O3 -m64 -fPIC -g")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR})

SET(SRC_LIST netvar.c mkvalues.c pack.c json_pack.c patt_copy.c colidx.c except.c SDBC_type.c mkfield.c setbind.c SRM.c SRM_json.c SRM_utility.c mk_where.c tpl_lib.c jerr.c)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRC_LIST})
