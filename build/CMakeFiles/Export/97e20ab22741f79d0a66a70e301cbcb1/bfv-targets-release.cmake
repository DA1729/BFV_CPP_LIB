#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "bfv::bfv" for configuration "Release"
set_property(TARGET bfv::bfv APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(bfv::bfv PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libbfv.a"
  )

list(APPEND _cmake_import_check_targets bfv::bfv )
list(APPEND _cmake_import_check_files_for_bfv::bfv "${_IMPORT_PREFIX}/lib/libbfv.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
