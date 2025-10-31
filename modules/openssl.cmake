# OpenSSL CMake module
#
# This module defines:
# OPENSSL_FOUND - System has OpenSSL
# OPENSSL_INCLUDE_DIRS - The OpenSSL include directories
# OPENSSL_LIBRARIES - The libraries needed to use OpenSSL
# OPENSSL_VERSION - OpenSSL version

find_package(OpenSSL QUIET)

if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}, ${OPENSSL_INCLUDE_DIR}, ${OPENSSL_LIBRARIES}")
else()
    message(STATUS "OpenSSL not found, will fetch from GitHub")
    
    include(FetchContent)
    FetchContent_Declare(
        openssl
        GIT_REPOSITORY https://github.com/openssl/openssl.git
        GIT_TAG OpenSSL_1_1_1u
        GIT_SHALLOW TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )
    
    FetchContent_GetProperties(openssl)
    if(NOT openssl_POPULATED)
        FetchContent_Populate(openssl)
        
        # Configure OpenSSL
        set(OPENSSL_CONFIGURE_COMMAND "")
        if(WIN32)
            # Windows configuration
            set(OPENSSL_CONFIGURE_COMMAND "perl Configure VC-WIN64A --prefix=${openssl_BINARY_DIR}/install")
            set(OPENSSL_BUILD_COMMAND "nmake")
            set(OPENSSL_INSTALL_COMMAND "nmake install")
        elseif(UNIX)
            # Unix/Linux configuration
            set(OPENSSL_CONFIGURE_COMMAND "./config --prefix=${openssl_BINARY_DIR}/install")
            set(OPENSSL_BUILD_COMMAND "make")
            set(OPENSSL_INSTALL_COMMAND "make install")
        endif()
        
        # Build OpenSSL
        execute_process(
            COMMAND ${OPENSSL_CONFIGURE_COMMAND}
            WORKING_DIRECTORY ${openssl_SOURCE_DIR}
            RESULT_VARIABLE OPENSSL_CONFIGURE_RESULT
        )
        
        if(OPENSSL_CONFIGURE_RESULT EQUAL 0)
            execute_process(
                COMMAND ${OPENSSL_BUILD_COMMAND}
                WORKING_DIRECTORY ${openssl_SOURCE_DIR}
                RESULT_VARIABLE OPENSSL_BUILD_RESULT
            )
            
            if(OPENSSL_BUILD_RESULT EQUAL 0)
                execute_process(
                    COMMAND ${OPENSSL_INSTALL_COMMAND}
                    WORKING_DIRECTORY ${openssl_SOURCE_DIR}
                    RESULT_VARIABLE OPENSSL_INSTALL_RESULT
                )
                
                if(OPENSSL_INSTALL_RESULT EQUAL 0)
                    # Set OpenSSL variables
                    set(OPENSSL_INCLUDE_DIR ${openssl_BINARY_DIR}/install/include)
                    set(OPENSSL_LIBRARY_DIR ${openssl_BINARY_DIR}/install/lib)
                    
                    # Find OpenSSL libraries
                    find_library(OPENSSL_SSL_LIBRARY ssl NAMES ssl libssl PATHS ${OPENSSL_LIBRARY_DIR} NO_DEFAULT_PATH)
                    find_library(OPENSSL_CRYPTO_LIBRARY crypto NAMES crypto libcrypto PATHS ${OPENSSL_LIBRARY_DIR} NO_DEFAULT_PATH)
                    
                    if(OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
                        set(OPENSSL_FOUND TRUE)
                        set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
                        set(OPENSSL_VERSION "1.1.1u")
                        message(STATUS "Built OpenSSL: ${OPENSSL_VERSION}, ${OPENSSL_INCLUDE_DIR}, ${OPENSSL_LIBRARIES}")
                    endif()
                endif()
            endif()
        endif()
    endif()
endif()

if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "Failed to find or build OpenSSL")
endif()

mark_as_advanced(OPENSSL_INCLUDE_DIR OPENSSL_LIBRARIES OPENSSL_VERSION)