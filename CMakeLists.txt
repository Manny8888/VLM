cmake_minimum_required(VERSION 3.13)

project(genera_alpha VERSION 0.1.0
                     DESCRIPTION "Alpha version of the emulator"
                     LANGUAGES C)


find_package(Threads REQUIRED)
find_package(X11 REQUIRED)


add_library(emulator_files 
    STATIC 
         utilities.c
         world_tools.c
         spy.c

         life-support/cold_load.c
         life-support/console.c
         life-support/disks.c
         life-support/genera-cptfont.xbm
         life-support/genera-icon-32.xbm
         life-support/initialization.c
         life-support/message_channels.c
         # life-support/network.c
         life-support/network-linux.c
         # life-support/network-tun-linux.c
         life-support/polling.c
         life-support/queues.c
         life-support/signals.c
         life-support/SystemComm.h

         emulator/traps.c
         emulator/memory.c
         emulator/traps.c
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






# message(STATUS "X11_FOUND = ${X11_FOUND}")
# message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
# message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

# include_directories(${X11_INCLUDE_DIR})


# # other c sources to build
# set(SRC_C
#         main.c 
        
#         utilities.c
#         world_tools.c
#         spy.c

#         life-support/cold_load.c
#         life-support/console.c
#         life-support/disks.c
#         life-support/genera-cptfont.xbm
#         life-support/genera-icon-32.xbm
#         life-support/initialization.c
#         life-support/message_channels.c
#         # life-support/network.c
#         life-support/network-linux.c
#         # life-support/network-tun-linux.c
#         life-support/polling.c
#         life-support/queues.c
#         life-support/signals.c
#         life-support/SystemComm.h

#         emulator/traps.c
#         emulator/memory.c
#         emulator/traps.c)



# include(CTest)
# enable_testing()

# target_link_libraries(alpha_emulator_files)
# # add_library(memory OBJECT ${SRC_C})
# # add_library(cold_load OBJECT ${SRC_C})
# # add_library(console OBJECT ${SRC_C})


# #set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# #set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# #include(CPack)

