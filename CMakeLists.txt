cmake_minimum_required(VERSION 3.15)
project(tianli)

include_directories(third_party)
include_directories(dependent_party)

set(ThridPatryDir ${CMAKE_SOURCE_DIR}/third_party)
set(DependentPartyDir ${CMAKE_SOURCE_DIR}/dependent_party)

# 统一生成到build目录中的bin和lib中
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

add_subdirectory(test)
add_subdirectory(source)