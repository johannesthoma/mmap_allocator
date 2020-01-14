# Specify the minimum version for CMake

cmake_minimum_required(VERSION 3.1)

# Project's name
project(mmap_allocator)
# We build using c++14
set(CMAKE_CXX_STANDARD 14)

# Use all standard-compliant optimizations
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -O3 -mcx16 -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -O3 -mcx16 -g")

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# set up our target executable and specify its dependencies and includes
add_library(mmap_allocator_objs OBJECT
  ${CMAKE_SOURCE_DIR}/mmap_file_pool.cpp)

set(mmap_allocator_INCLUDES
  ${CMAKE_SOURCE_DIR})

set(mmap_allocator_HEADERS
  ${CMAKE_SOURCE_DIR}/mmap_access_mode.h
  ${CMAKE_SOURCE_DIR}/mmap_allocator.h
  ${CMAKE_SOURCE_DIR}/mmap_exception.h
  ${CMAKE_SOURCE_DIR}/mmap_file_pool.h
  ${CMAKE_SOURCE_DIR}/mmappable_vector.h)

target_include_directories(mmap_allocator_objs PUBLIC ${mmap_allocator_INCLUDES})
set_target_properties(mmap_allocator_objs PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

add_library(libmmap_allocator_static STATIC $<TARGET_OBJECTS:mmap_allocator_objs>)
set_target_properties(libmmap_allocator_static PROPERTIES OUTPUT_NAME "mmap_allocator")
set_target_properties(libmmap_allocator_static PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
add_library(libmmap_allocator_shared SHARED $<TARGET_OBJECTS:mmap_allocator_objs>)
set_target_properties(libmmap_allocator_shared PROPERTIES OUTPUT_NAME "mmap_allocator")
set_target_properties(libmmap_allocator_shared PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

set_target_properties(libmmap_allocator_static PROPERTIES PUBLIC_HEADER "${mmap_allocator_HEADERS}")
set_target_properties(libmmap_allocator_shared PROPERTIES PUBLIC_HEADER "${mmap_allocator_HEADERS}")

add_executable(test_mmap_allocator
  $<TARGET_OBJECTS:mmap_allocator_objs>
  ${CMAKE_SOURCE_DIR}/test_allocator.cpp)
#set_target_properties(mmap_allocator PROPERTIES OUTPUT_NAME "mmap_allocator")

install(TARGETS test_mmap_allocator DESTINATION bin)
install(TARGETS libmmap_allocator_static ARCHIVE DESTINATION lib LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/mmap_allocator)
install(TARGETS libmmap_allocator_shared ARCHIVE DESTINATION lib LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/mmap_allocator)
