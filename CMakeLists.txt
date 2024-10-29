# Copyright(c) 2023-2024 Gaoxing Li Distributed under the Apache License 2.0 (http://www.apache.org/licenses/)

cmake_minimum_required(VERSION 3.12)

# ---------------------------------------------------------------------------------------
# Start okec project
# ---------------------------------------------------------------------------------------
project(
    okec
    VERSION 1.0.1
    DESCRIPTION "An edge computing simulator"
    LANGUAGES CXX
)

message("\
  __  __ _  ____  ___ \n\
 /  \\(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)\n\
(  O ))  (  ) _)( (__  version ${PROJECT_VERSION}\n\
 \\__/(__\\_)(____)\\___) https://github.com/dxnu/okec\n\
")

# Find the dependencies
find_package(Torch REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ns3 3.41 COMPONENTS libcore REQUIRED)
find_package(PythonLibs REQUIRED)

# message(${TORCH_CXX_FLAGS})
message("Welcome to the installation wizard for OKEC")
message(STATUS "Found Torch: ${TORCH_FOUND}")
message(STATUS "Found nlohmann_json: ${nlohmann_json_FOUND}")
message(STATUS "Found ns3: ${ns3_FOUND}")
message(STATUS "Found python libs: ${PYTHON_INCLUDE_DIRS}")

# Found all source files
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cc")
list(LENGTH SOURCE_FILES SRC_FILES_SIZE)
message(STATUS "Found ${SRC_FILES_SIZE} source files of okec")
# foreach(source_file ${SOURCE_FILES})
#     message(STATUS "  ${source_file}")
# endforeach()

# Define a shared library target named `okec`
add_library(okec SHARED)

target_sources(okec PRIVATE ${SOURCE_FILES})

target_include_directories(okec PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${PYTHON_INCLUDE_DIRS}
)


target_link_libraries(okec PUBLIC
    ${TORCH_LIBRARIES}
    ${PYTHON_LIBRARIES}
    nlohmann_json::nlohmann_json
    ns3::libcore ns3::libinternet ns3::libpoint-to-point ns3::libcsma ns3::libwifi
)

target_compile_options(okec PRIVATE -Wall -Werror)
target_compile_features(okec PUBLIC cxx_std_23)

include(GNUInstallDirs)

install(TARGETS okec
        EXPORT okec-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT okec-targets
        NAMESPACE okec::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/okec/cmake
)

include(CMakePackageConfigHelpers)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/okec)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/okec-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/okec-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/okec/cmake
    PATH_VARS LIB_INSTALL_DIR
)

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/okec-config.cmake"
    "include(CMakeFindDependencyMacro)\nfind_dependency(nlohmann_json)\nfind_dependency(ns3)\nlist(APPEND CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})\nfind_package(Torch)"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/okec-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/okec/cmake
)

write_basic_package_version_file(
    "okec-config-version.cmake"
    COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/okec-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/okec/cmake
)