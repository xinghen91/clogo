cmake_minimum_required( VERSION 2.8 )

set( PROJ_NAME "clogo" )
set( PROJ_EXE "cl" )

file( GLOB_RECURSE PROJ_SOURCES "src/*.c" )
file( GLOB PROJ_MAIN "src/main.c" )
list( REMOVE_ITEM PROJ_SOURCES ${PROJ_MAIN} )
set( PROJ_INCLUDES "${CMAKE_SOURCE_DIR}/include" )

project( ${PROJ_NAME} )

set( CMAKE_C_FLAGS "-std=c11 -Wall" )
set( CMAKE_C_FLAGS_RELEASE "-O3" )
set( CMAKE_C_FLAGS_DEBUG "-gdwarf-3" )

include_directories( ${PROJ_INCLUDES} )
add_library( ${PROJ_NAME} ${PROJ_SOURCES} )
add_executable( ${PROJ_EXE} ${PROJ_MAIN} )
target_link_libraries( ${PROJ_EXE} ${PROJ_NAME} )

