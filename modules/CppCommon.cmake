if(NOT TARGET cppcommon)

  # Set module flag
  set(CPPCOMMON_MODULE Y)

  # Add module subdirectory
  add_subdirectory("CppCommon")

  # Set module folder
  set_target_properties(cppcommon PROPERTIES FOLDER modules/CppCommon)

endif()
