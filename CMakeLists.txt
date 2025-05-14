cmake_minimum_required(VERSION 3.10)
project(FileFlow)

set(CMAKE_C_STANDARD 11)
set(SRC_DIR src)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MINIZIP REQUIRED minizip)
pkg_check_modules(READLINE REQUIRED readline)

include_directories(${SRC_DIR})
include_directories(${MINIZIP_INCLUDE_DIRS})

set(SOURCES
    ${SRC_DIR}/main.c
    ${SRC_DIR}/utils.c
    ${SRC_DIR}/tokenizer.c
    ${SRC_DIR}/hostNameUtils.c
    ${SRC_DIR}/commands.c
)

add_executable(FileFlow ${SOURCES})

# Link Minizip
target_link_libraries(FileFlow ${MINIZIP_LIBRARIES} ${READLINE_LIBRARIES})