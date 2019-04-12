cmake_minimum_required(VERSION 3.0.0)
project(genera_c VERSION 0.1.0)

include_directories(life-support c-emulator /usr/include)
add_definitions(-D_C_EMULATOR_)

# other c sources to build
set(SRC_C utilities.c c-emulator/memory.c)
add_library(c_emulator_files STATIC ${SRC_C})


include(CTest)
enable_testing()

target_link_libraries(c_emulator_files)
add_library(memory OBJECT ${SRC_C})
add_executable(genera_c main.c ${SRC_C})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
