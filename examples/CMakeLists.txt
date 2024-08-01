cmake_minimum_required(VERSION 3.12)
project(examples VERSION 1.0)

find_package(okec 1.0 CONFIG REQUIRED)

message(STATUS "okec found: ${okec_FOUND}")

# add_executable(sixth "src/sixth.cc")
# target_link_libraries(sixth PRIVATE okec::okec)
# target_compile_options(sixth PRIVATE -Wall -g)

file(GLOB SOURCE_FILES src/*.cc)
list(LENGTH SOURCE_FILES SRC_FILES_SIZE)
message(STATUS "Found ${SRC_FILES_SIZE} source files of examples")
foreach(source_file ${SOURCE_FILES})
    get_filename_component(file_name ${source_file} NAME)
    string(REPLACE ".cc" "" name ${file_name})
    message(STATUS " ${name}")
    add_executable(${name} "src/${name}.cc")
    target_link_libraries(${name} PRIVATE okec::okec)
    target_compile_options(${name} PRIVATE -Wall -g)
endforeach()
