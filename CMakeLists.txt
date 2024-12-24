# CMakeList.txt : CMake project for RedEye, include source and define
# project specific logic here.

cmake_minimum_required (VERSION 3.28)

# Set the global C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Enable code coverage
option(CODE_COVERAGE "Enable code coverage reporting" OFF)

if(CODE_COVERAGE)
  message(STATUS "Enabling code coverage support")

  # Add coverage flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} --coverage")

  # Add debug symbols
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

  # Disable optimizations
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif()

project ("RedEye")
add_subdirectory(source)

enable_testing()
add_subdirectory(tests)
