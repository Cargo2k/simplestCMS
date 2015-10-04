cmake_minimum_required(VERSION 3.3)
project(simplestCMS)
set(SOURCES src/main.cpp src/markdown.cpp src/markdown-tokens.cpp)

#setup the binaries
add_executable(simplestCMS ${SOURCES})

# init the pkg-config system
find_package(PkgConfig REQUIRED)

#set the c++ version for non-msvc compilers
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

#find libxml2 dependences
pkg_search_module(LIBXML2 REQUIRED libxml-2.0)
if (LIBXML2_FOUND)
	include_directories(${LIBXML2_INCLUDE_DIRS})
	link_directories(${LIBXML2_LIBRARY_DIRS})
	add_definitions(${LIBXML2_CFLAGS_OTHER})
	target_link_libraries(simplestCMS ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)

#find the mustache dependences
find_package(mstch REQUIRED)
if (mstch_FOUND)
	include_directories("/usr/include")
	target_link_libraries(simplestCMS "-lmstch")
endif (mstch_FOUND)

#add the cpp-markdown dependences
target_link_libraries(simplestCMS "-lboost_regex")

#add the project include file
include_directories(include)