﻿# CMakeList.txt : CMake project for RedEye, include source and define
# project specific logic here.

cmake_minimum_required (VERSION 3.28)

# Set the global C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

project ("RedEye")
add_subdirectory(source)

enable_testing()
add_subdirectory(tests)