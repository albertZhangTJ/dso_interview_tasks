cmake_minimum_required(VERSION 3.13.4)
project(InstCnt)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Add pass as a loadable module
add_library(InstCnt MODULE
  InstCnt.cpp
)

# Use C++14 to compile our pass
set_target_properties(InstCnt PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_FLAGS "-fno-rtti"
)

# Get proper shared-library behavior on all OSes
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(InstCnt PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup"
  )
endif()
