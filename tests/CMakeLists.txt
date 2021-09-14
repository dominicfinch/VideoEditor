cmake_minimum_required(VERSION 3.16)

project(video-editor-unittests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(PROJECT_SOURCE_DIR "./src")

set(LINK_LIBRARIES
        # FFMpeg Libs
        -lavutil
        -lavformat
        -lavcodec
        -lcppunit
        -lpthread -lstdc++fs)

file(GLOB PROJECT_SOURCES
        ${PROJECT_SOURCE_DIR}/**/*.cc
        ${PROJECT_SOURCE_DIR}/*.cc)

#message(STATUS "PROJECT_SOURCES: ${PROJECT_SOURCES}")
if("${QT_INCLUDE_PATH}" STREQUAL "")
    set(QT_INCLUDE_PATH "/usr/include/x86_64-linux-gnu/qt5")
endif()

include_directories(${PROJECT_SOURCE_DIR}
        ${QT_INCLUDE_PATH})

# Needed for Qt
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Build
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})
