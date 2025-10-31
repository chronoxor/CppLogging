# nlohmann/json CMake module
#
# This module defines:
# NLOHMANN_JSON_FOUND - System has nlohmann/json
# NLOHMANN_JSON_INCLUDE_DIRS - The nlohmann/json include directories

find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/nlohmann_json/include
    $ENV{NLOHMANN_JSON_ROOT}/include
    /usr/local/include
    /usr/include
    /opt/local/include
    /opt/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nlohmann_json DEFAULT_MSG NLOHMANN_JSON_INCLUDE_DIR)

mark_as_advanced(NLOHMANN_JSON_INCLUDE_DIR)

if(NLOHMANN_JSON_FOUND)
    set(NLOHMANN_JSON_INCLUDE_DIRS ${NLOHMANN_JSON_INCLUDE_DIR})
endif()

if(NLOHMANN_JSON_FOUND AND NOT TARGET nlohmann_json)
    add_library(nlohmann_json INTERFACE IMPORTED)
    set_target_properties(nlohmann_json PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${NLOHMANN_JSON_INCLUDE_DIRS}
    )
endif()

# If nlohmann/json is not found, fetch it from GitHub
if(NOT NLOHMANN_JSON_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
    set(NLOHMANN_JSON_FOUND TRUE)
    set(NLOHMANN_JSON_INCLUDE_DIRS ${nlohmann_json_SOURCE_DIR}/include)
endif()