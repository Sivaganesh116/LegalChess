cmake_minimum_required(VERSION 3.10)

#project name and languages
project(Chessu LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/)

#Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/inc
)

add_library(LegalChess STATIC
    ${CMAKE_SOURCE_DIR}/src/MoveManager.cpp
)