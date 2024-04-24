#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "okec::okec" for configuration ""
set_property(TARGET okec::okec APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(okec::okec PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libokec.so"
  IMPORTED_SONAME_NOCONFIG "libokec.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS okec::okec )
list(APPEND _IMPORT_CHECK_FILES_FOR_okec::okec "${_IMPORT_PREFIX}/lib/libokec.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
