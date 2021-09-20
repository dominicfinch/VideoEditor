
cmake_minimum_required(VERSION 3.16)
project(video-editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(PROJECT_SOURCE_DIR "./src")
set(PROJECT_UNITTEST_DIR "./tests")
set(LINK_LIBRARIES
        -lavutil
        -lavformat
        -lavcodec
        #-lx264
        -lgstreamer-1.0
        -lgobject-2.0
        -lglib-2.0
        -lQt5Core
        -lQt5Widgets
        -lpthread -lstdc++fs)

file(GLOB PROJECT_SOURCES
        ${PROJECT_SOURCE_DIR}/*.cc
        ${PROJECT_SOURCE_DIR}/**/*.cc
        ${PROJECT_SOURCE_DIR}/**/**/*.cc)

#message(STATUS "PROJECT_SOURCES: ${PROJECT_SOURCES}")
if("${QT_INCLUDE_PATH}" STREQUAL "")
    set(QT_INCLUDE_PATH "/usr/include/x86_64-linux-gnu/qt5")
endif()

if("${GS_INCLUDE_PATH}" STREQUAL "")
    set(GS_INCLUDE_PATH "/usr/include/gstreamer-1.0")
endif()

if("${GLIB_INCLUDE_PATH}" STREQUAL "")
    set(GLIB_INCLUDE_PATH "/usr/include/glib-2.0")
    set(GLIB_CONF_INCLUDE_PATH "/usr/lib/x86_64-linux-gnu/glib-2.0/include")
endif()

include_directories(${PROJECT_SOURCE_DIR}
        ${QT_INCLUDE_PATH}
        ${GS_INCLUDE_PATH}
        ${GLIB_INCLUDE_PATH}
        ${GLIB_CONF_INCLUDE_PATH})

# Needed for Qt
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Build
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})

# Unit Tests
# add_subdirectory(${PROJECT_UNITTEST_DIR})
