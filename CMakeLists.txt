cmake_minimum_required(VERSION 3.0.0)
project(genera_c VERSION 0.1.0)

include_directories(. life-support c-emulator environment /usr/include /usr/include/X11)

find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

include_directories(${X11_INCLUDE_DIR})

add_definitions(-D_C_EMULATOR_)

# other c sources to build
set(SRC_C
        life-support/cold_load.c
        life-support/console.c
        life-support/disks.c
        life-support/genera-cptfont.c
        life-support/genera-icon-32.c
        life-support/initialization.c
        life-support/message_channels.c
        #    life-support/network.c
        life-support/network-linux.c
        #    life-support/network-tun-linux.c
        life-support/polling.c
        life-support/queues.c
        life-support/signals.c
        life-support/SystemComm.c

        c-emulator/traps.c
        c-emulator/dispatch.c
        c-emulator/emulator.c
        c-emulator/initialization.c
        c-emulator/memory.c
        c-emulator/traps.c

        environment/introspection.c

        utilities.c
        world_tools.c)


add_library(c_emulator_files STATIC ${SRC_C})

include(CTest)
enable_testing()

target_link_libraries(c_emulator_files)
# add_library(memory OBJECT ${SRC_C})
# add_library(cold_load OBJECT ${SRC_C})
# add_library(console OBJECT ${SRC_C})

add_executable(genera_c main.c ${SRC_C})
target_link_libraries(genera_c ${X11_LIBRARIES} ${PTHREAD_LIBRARY} pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
