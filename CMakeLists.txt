cmake_minimum_required(VERSION 3.6)

# Global properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Project name
project(cpplogging)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN "doxygen")
  if(NOT TARGET ${DOXYGEN})
    add_custom_command(OUTPUT "Doxyfile" COMMAND ${DOXYGEN_EXECUTABLE} "Doxyfile" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/documents")
    add_custom_target(${DOXYGEN} DEPENDS "Doxyfile")
    set_target_properties(${DOXYGEN} PROPERTIES FOLDER "doxygen")
  endif()
endif()

# CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler features
include(SetCompilerFeatures)
include(SetCompilerWarnings)
include(SetPlatformFeatures)
include(SystemInformation)

# Modules
add_subdirectory("modules")

# Link libraries
list(APPEND LINKLIBS cppcommon)

# Support zlib/contrib/minizip
file(GLOB_RECURSE MINIZIP_FILES "source/logging/appenders/minizip/*.c")
if(NOT WIN32)
  list(REMOVE_ITEM MINIZIP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/logging/appenders/minizip/iowin32.c")
endif()

# System directories
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/modules")

# Library
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_INLINE_FILES "include/*.inl")
file(GLOB_RECURSE LIB_SOURCE_FILES "source/*.cpp")
add_library(cpplogging ${LIB_HEADER_FILES} ${LIB_INLINE_FILES} ${LIB_SOURCE_FILES} ${MINIZIP_FILES})
if(MSVC)
  # C4067: unexpected tokens following preprocessor directive - expected a newline
  # C4131: 'function' : uses old-style declarator
  # C4189: 'identifier' : local variable is initialized but not referenced
  # C4244: 'conversion' conversion from 'type1' to 'type2', possible loss of data
  # C4456: declaration of 'identifier' hides previous local declaration
  set_target_properties(cpplogging PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} /wd4067 /wd4131 /wd4189 /wd4244 /wd4456" FOLDER "libraries")
else()
  set_target_properties(cpplogging PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} -Wno-shadow -Wno-unused-variable" FOLDER "libraries")
endif()
if(CYGWIN OR MINGW)
  target_compile_definitions(cpplogging PRIVATE USE_FILE32API=1)
endif()
target_include_directories(cpplogging PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(cpplogging ${LINKLIBS} zlib)
list(APPEND INSTALL_TARGETS cpplogging)
list(APPEND LINKLIBS cpplogging)

# Additional module components: benchmarks, examples, plugins, tests, tools and install
if(NOT CPPLOGGING_MODULE)

  # Examples
  file(GLOB EXAMPLE_HEADER_FILES "examples/*.h")
  file(GLOB EXAMPLE_INLINE_FILES "examples/*.inl")
  file(GLOB EXAMPLE_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/examples" "examples/*.cpp")
  foreach(EXAMPLE_SOURCE_FILE ${EXAMPLE_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" EXAMPLE_NAME ${EXAMPLE_SOURCE_FILE})
    set(EXAMPLE_TARGET "cpplogging-example-${EXAMPLE_NAME}")
    add_executable(${EXAMPLE_TARGET} ${EXAMPLE_HEADER_FILES} ${EXAMPLE_INLINE_FILES} "examples/${EXAMPLE_SOURCE_FILE}")
    set_target_properties(${EXAMPLE_TARGET} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "examples")
    target_link_libraries(${EXAMPLE_TARGET} ${LINKLIBS})
    list(APPEND INSTALL_TARGETS ${EXAMPLE_TARGET})
    list(APPEND INSTALL_TARGETS_PDB ${EXAMPLE_TARGET})
  endforeach()

  # Benchmarks
  file(GLOB BENCHMARK_HEADER_FILES "performance/*.h")
  file(GLOB BENCHMARK_INLINE_FILES "performance/*.inl")
  file(GLOB BENCHMARK_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/performance" "performance/*.cpp")
  foreach(BENCHMARK_SOURCE_FILE ${BENCHMARK_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" BENCHMARK_NAME ${BENCHMARK_SOURCE_FILE})
    set(BENCHMARK_TARGET "cpplogging-performance-${BENCHMARK_NAME}")
    add_executable(${BENCHMARK_TARGET} ${BENCHMARK_HEADER_FILES} ${BENCHMARK_INLINE_FILES} "performance/${BENCHMARK_SOURCE_FILE}")
    set_target_properties(${BENCHMARK_TARGET} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "performance")
    target_link_libraries(${BENCHMARK_TARGET} ${LINKLIBS} cppbenchmark)
    list(APPEND INSTALL_TARGETS ${BENCHMARK_TARGET})
    list(APPEND INSTALL_TARGETS_PDB ${BENCHMARK_TARGET})
  endforeach()

  # Tools
  file(GLOB TOOLS_DIRS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/tools" "tools/*")
  foreach(TOOLS_DIR ${TOOLS_DIRS})
    file(GLOB_RECURSE TOOLS_HEADER_FILES "tools/${TOOLS_DIR}/*.h")
    file(GLOB_RECURSE TOOLS_INLINE_FILES "tools/${TOOLS_DIR}/*.inl")
    file(GLOB_RECURSE TOOLS_SOURCE_FILES "tools/${TOOLS_DIR}/*.cpp")
    set(TOOL_TARGET "${TOOLS_DIR}")
    add_executable(${TOOL_TARGET} ${TOOLS_HEADER_FILES} ${TOOLS_INLINE_FILES} ${TOOLS_SOURCE_FILES})
    set_target_properties(${TOOL_TARGET} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "tools")
    target_link_libraries(${TOOL_TARGET} ${LINKLIBS} cpp-optparse)
    list(APPEND INSTALL_TARGETS ${TOOL_TARGET})
    list(APPEND INSTALL_TARGETS_PDB ${TOOL_TARGET})
  endforeach()

  # Tests
  file(GLOB TESTS_HEADER_FILES "tests/*.h")
  file(GLOB TESTS_INLINE_FILES "tests/*.inl")
  file(GLOB TESTS_SOURCE_FILES "tests/*.cpp")
  add_executable(cpplogging-tests ${TESTS_HEADER_FILES} ${TESTS_INLINE_FILES} ${TESTS_SOURCE_FILES} ${Catch2})
  set_target_properties(cpplogging-tests PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "tests")
  target_include_directories(cpplogging-tests PRIVATE ${Catch2})
  target_link_libraries(cpplogging-tests ${LINKLIBS})
  list(APPEND INSTALL_TARGETS cpplogging-tests)
  list(APPEND INSTALL_TARGETS_PDB cpplogging-tests)

  # CTest
  enable_testing()
  add_test(cpplogging-tests cpplogging-tests --durations yes --order lex)

  # Install
  install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY DESTINATION "${PROJECT_SOURCE_DIR}/bin"
    ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/bin")

  # Install *.pdb files
  if(MSVC)
    foreach(INSTALL_TARGET_PDB ${INSTALL_TARGETS_PDB})
      install(FILES $<TARGET_PDB_FILE:${INSTALL_TARGET_PDB}> DESTINATION "${PROJECT_SOURCE_DIR}/bin")
    endforeach()
  endif()

endif()
