cmake_minimum_required(VERSION 3.12)
project(wf_net VERSION 1.0)


# find_package(ns3 3.41 COMPONENTS libcore)

find_package(okec 1.0 CONFIG REQUIRED)

message(STATUS "okec found: ${okec_FOUND}")

add_executable(wf_net src/wf_net.cc)
target_link_libraries(wf_net PRIVATE okec::okec)

add_executable(wf_discrete src/wf_discrete.cc)
target_link_libraries(wf_discrete PRIVATE okec::okec)

add_executable(rf_discrete src/rf_discrete.cc)
target_link_libraries(rf_discrete PRIVATE okec::okec)

add_executable(wf_async src/wf_async.cc)
target_compile_options(wf_async PRIVATE -Wall -g)
target_link_libraries(wf_async PRIVATE okec::okec)
