cmake_minimum_required(VERSION 3.2)
project(DistributedAlgorithms)

include(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
include_directories(${GTK2_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS})
add_definitions(${GTK2_CFLAGS_OTHER})

SET(GCC_COVERAGE_COMPILE_FLAGS "-lpthread -lm")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++")


set(DIRECTORY "Directory/directory.c")
set(CLIENT "Client/client.c")
set(REPLICA "Replica/replica.c")
set(FILEMANAGER "FileManager/filemanager.c")
set(GUI "Interface/Client/client.c")


add_executable(replica ${REPLICA})
target_link_libraries(replica ${GCC_COVERAGE_COMPILE_FLAGS} ${GLIB_LIBRARIES})

add_executable(directory ${DIRECTORY})
target_link_libraries(directory ${GCC_COVERAGE_COMPILE_FLAGS} ${GLIB_LIBRARIES})

add_executable(client ${CLIENT})
target_link_libraries(client ${GCC_COVERAGE_COMPILE_FLAGS} ${GLIB_LIBRARIES})

add_executable(filemanager ${FILEMANAGER})
target_link_libraries(filemanager ${GCC_COVERAGE_COMPILE_FLAGS} ${GLIB_LIBRARIES})

add_executable(gui ${GUI})
target_link_libraries(gui ${GCC_COVERAGE_COMPILE_FLAGS} ${GLIB_LIBRARIES}{GTK2_LIBRARIES})

