cmake_minimum_required(VERSION 3.0.0)
project(genera_c VERSION 0.1.0)

include_directories(. life-support c-emulator /usr/include)
add_definitions(-D_C_EMULATOR_)

# other c sources to build
set(SRC_C life-support/cold_load.c life-support/console.c life-support/disks.c life-support/initialization.c life-support/message_channels.c life-support/network-linux.c life-support/polling.c life-support/queues.c life-support/signals.c c-emulator/traps.c life-support/signals.c c-emulator/dispatch.c  c-emulator/emulator.c c-emulator/initialization.c c-emulator/memory.c c-emulator/traps.c spy.c utilities.c world_tools.c)
add_library(c_emulator_files STATIC ${SRC_C})


include(CTest)
enable_testing()

target_link_libraries(c_emulator_files)
add_library(memory OBJECT ${SRC_C})
add_executable(genera_c main.c ${SRC_C})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
