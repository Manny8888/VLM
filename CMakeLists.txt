cmake_minimum_required(VERSION 3.13)

project(genera_alpha VERSION 0.1.0
                     DESCRIPTION "Alpha version of the emulator")


find_package(Threads REQUIRED)
find_package(X11 REQUIRED)


add_library(emulator_files 
    STATIC 
         utilities.c
         world_tools.c
         spy.c

         emulator/traps.c
         emulator/memory.c


         life-support/cold_load.c
         life-support/console.c
         life-support/disks.c
         life-support/genera-cptfont.xbm
         life-support/genera-icon-32.xbm
         life-support/initialization.c
         life-support/message_channels.c
         life-support/polling.c
         life-support/queues.c
         life-support/signals.c
         life-support/SystemComm.h
)

target_include_directories(emulator_files
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/alpha-emulator
        ${CMAKE_CURRENT_SOURCE_DIR}/emulator
        ${CMAKE_CURRENT_SOURCE_DIR}/life-support
        ${CMAKE_CURRENT_SOURCE_DIR}/stub
        ${CMAKE_CURRENT_SOURCE_DIR}/x86_64-emulator
        /usr/include 
        /usr/include/X11
)




add_executable(genera_alpha main.c)
target_include_directories(genera_alpha
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/alpha-emulator
        ${CMAKE_CURRENT_SOURCE_DIR}/emulator
        ${CMAKE_CURRENT_SOURCE_DIR}/life-support
        ${CMAKE_CURRENT_SOURCE_DIR}/stub
        ${CMAKE_CURRENT_SOURCE_DIR}/x86_64-emulator
        /usr/include 
        /usr/include/X11
)

target_link_libraries(genera_alpha 
    PUBLIC
        emulator_files
        ${X11_LIBRARIES} 
        ${PTHREAD_LIBRARY}
)


add_executable(byteswap_world byteswap_world.c)
target_include_directories(byteswap_world
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/alpha-emulator
        ${CMAKE_CURRENT_SOURCE_DIR}/emulator
        ${CMAKE_CURRENT_SOURCE_DIR}/life-support
        ${CMAKE_CURRENT_SOURCE_DIR}/stub
        ${CMAKE_CURRENT_SOURCE_DIR}/x86_64-emulator
        /usr/include 
        /usr/include/X11
)

target_link_libraries(byteswap_world
    PUBLIC
        emulator_files
        ${X11_LIBRARIES} 
        ${PTHREAD_LIBRARY}
)

