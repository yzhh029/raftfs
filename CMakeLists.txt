cmake_minimum_required(VERSION 3.4)
project(raftfs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(. SOURCE_FILES)

find_package(Threads)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.55.0 COMPONENTS program_options)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
add_executable(raftfs ${SOURCE_FILES})
target_link_libraries(raftfs ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})