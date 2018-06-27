cmake_minimum_required(VERSION 3.10)
project(C10kProblem)

set(CMAKE_CXX_STANDARD 14)

find_package(
        Boost 1.53 REQUIRED
        COMPONENTS filesystem system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(gflags)
find_package(glog)

include_directories(${CMAKE_SOURCE_DIR})

SET(C10k_COMMON_HEADERS
        common/c10k_types.h
        interface/virtual_io.h
        )

# Select IO
SET(C10k_SELECT_SOURCES select/select_io.h select/select_io.cc)
# Poll IO
SET(C10k_POLL_SOURCES poll/poll_io.h poll/poll_io.cc)

add_executable(select_server
        ${C10k_COMMON_HEADERS}
        ${C10k_SELECT_SOURCES} interface/server.cc)

add_executable(select_client
        ${C10k_COMMON_HEADERS}
        ${C10k_SELECT_SOURCES} interface/client.cc)

target_link_libraries(select_server -lgflags -lglog)
target_link_libraries(select_client -lgflags -lglog)


add_executable(poll_server
        ${C10k_COMMON_HEADERS}
        ${C10k_POLL_SOURCES} poll/server.cc)

add_executable(poll_client
        ${C10k_COMMON_HEADERS}
        ${C10k_POLL_SOURCES} poll/client.cc)

target_link_libraries(poll_server -lgflags -lglog)
target_link_libraries(poll_client -lgflags -lglog)