cmake_minimum_required(VERSION 3.0.0)
project(genera_c VERSION 0.1.0)

include_directories(life-support c-emulator)
add_definitions(-D_C_EMULATOR_)

# other c sources to build
set(SRC_C c-emulator/memory.c)
add_library(c_emulator_files STATIC ${SRC_C})


include(CTest)
enable_testing()

add_executable(genera_c main.c)
target_link_libraries(c_emulator_files)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
