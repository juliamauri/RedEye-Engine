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

  # Add coverage flags for GCC/Clang
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g -O0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -g -O0")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

  # Ensure gcov is used as coverage generator
  find_program(GCOV_PATH gcov)
  if(NOT GCOV_PATH)
    message(FATAL_ERROR "gcov not found! Please install gcov.")
  endif()

  # Add coverage to the build output
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

project ("RedEye")
add_subdirectory(source)

enable_testing()
add_subdirectory(tests)
