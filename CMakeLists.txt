cmake_minimum_required(VERSION 3.0.0)
project(Qt5ShadowMaps)

if (MSVC)
  set(Your_Qt5_DIR "Qt5-NOT_FOUND" CACHE PATH "")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${Your_Qt5_DIR})
endif()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(OpenGL REQUIRED)

if (MSVC)
  set(GLEW_DIR "GLEW-NOT_FOUND" CACHE PATH "")
  if (NOT EXISTS "${GLEW_DIR}/include/GL/glew.h")
    message(FATAL_ERROR "GLEW not found!!")
  endif()
  set(GLEW_INCLUDE_DIRS "${GLEW_DIR}/include")
  set(GLEW_LIBRARIES "${GLEW_DIR}/lib/glew32.lib")
endif()

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
endif()

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output")

add_subdirectory(sources)
