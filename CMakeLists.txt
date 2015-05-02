#------------------------------------------------------------
# Standard CMake Stuff
#------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( Alice-DevKit )

SET ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake )

MESSAGE ( STATUS "Source DIR is ${PROJECT_SOURCE_DIR}" )
MESSAGE ( STATUS "Binary DIR is ${PROJECT_BINARY_DIR}" )
MESSAGE ( STATUS "Build type is ${CMAKE_BUILD_TYPE}" )
MESSAGE ( STATUS "Libs extension is '${SUFFIXLIB}'. Bins extension is '${SUFFIXBIN}'." )
MESSAGE ( STATUS "Installation prefix directory is " ${CMAKE_INSTALL_PREFIX} )
MESSAGE ( STATUS "Host system is " ${CMAKE_HOST_SYSTEM} " with processor " ${CMAKE_HOST_SYSTEM_PROCESSOR} )
MESSAGE ( STATUS "Target system is " ${CMAKE_SYSTEM} " with processor " ${CMAKE_SYSTEM_PROCESSOR} )

# Build unit tests
OPTION (BUILD_TEST "Build unit tests, requires Boost.Test library" OFF)

# Set a custom htdocs path
OPTION (CUSTOM_HTDOCS_PATH "Custom htdocs path, default is INSTALL_PREFIX/share/alice/htdocs" OFF)

#------------------------------------------------------------
# Compiler Setup
#------------------------------------------------------------

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET ( CMAKE_CXX_FLAGS "-O3 --std=c++0x -fPIC -Wall --stdlib=libc++" )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  SET ( CMAKE_CXX_FLAGS "-g -O3 --std=c++0x -fPIC -Wall" )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  MESSAGE ( STATUS "Intel Compiler not tested yet, falling back to default compiler flags" )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # use default msvc flags
endif()


#------------------------------------------------------------
# Generate configuration
#------------------------------------------------------------

IF ( CUSTOM_HTDOCS_PATH )
    SET ( DEVKIT_HTDOCS ${CUSTOM_HTDOCS_PATH} )
ELSE ( CUSTOM_HTDOCS_PATH )
    SET ( DEVKIT_HTDOCS ${CMAKE_INSTALL_PREFIX}/share/alice/htdocs )
ENDIF ( CUSTOM_HTDOCS_PATH )

MESSAGE ( STATUS "Generating src/config.hpp" )
CONFIGURE_FILE (
    ${CMAKE_SOURCE_DIR}/src/config.hpp.in
    ${CMAKE_SOURCE_DIR}/src/config.hpp
)

#------------------------------------------------------------
# Include And Configure Libraries Used
#------------------------------------------------------------

IF ( UNIX )
    # Boost headers / librarys
    IF ( BUILD_TEST )
        find_package (Boost COMPONENTS system unit_test_framework REQUIRED)
    ELSE ( BUILD_TEST )
        find_package ( Boost COMPONENTS system REQUIRED )
    ENDIF ( BUILD_TEST )

    include( FindAlice )
    find_package ( Alice REQUIRED )

    include_directories( ${Boost_INCLUDE_DIRS} ${ALICE_INCLUDE_DIR} )
ELSE ( UNIX )
    # Boost headers / librarys
    IF ( EXISTS ${CMAKE_SOURCE_DIR}/deps/boost-1.55.0 )
        SET ( BOOST_ROOT ${CMAKE_SOURCE_DIR}/deps/boost-1.55.0 )
    ENDIF ( EXISTS ${CMAKE_SOURCE_DIR}/deps/boost-1.55.0 )

   IF ( BUILD_TEST )
        find_package (Boost COMPONENTS system unit_test_framework REQUIRED)
    ELSE ( BUILD_TEST )
        find_package ( Boost COMPONENTS system REQUIRED )
    ENDIF ( BUILD_TEST )

    # TODO: Set default path for windows installation on alice
    include( FindAlice )
    find_package ( Alice REQUIRED )

    include_directories( ${Boost_INCLUDE_DIRS} ${ALICE_INCLUDE_DIR} )
ENDIF ( UNIX )

IF ( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/http-parser/http_parser.c )
    MESSAGE( FATAL_ERROR "Please checkout the http-parser submodule." )
ENDIF ( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/http-parser/http_parser.c )

include_directories( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/deps/http-parser  )

#------------------------------------------------------------
# Build the DevKit
#------------------------------------------------------------

SET ( DEVKIT_SOURCES
    deps/http-parser/http_parser.c
    src/http_connection.cpp
    src/http_mime_type.cpp
    src/http_reply.cpp
    src/json.cpp
    src/devkit_http.cpp
    src/devkit.cpp
)

ADD_EXECUTABLE ( alice-devkit
    ${DEVKIT_SOURCES}
)

TARGET_LINK_LIBRARIES ( alice-devkit
    ${ALICE_LIBRARIES}
    ${Boost_LIBRARIES}
    protobuf
)

IF ( UNIX )
    # Boost.Asio required pthread on unix, TODO check windows
    TARGET_LINK_LIBRARIES ( alice-devkit pthread )
ENDIF ( UNIX )

INSTALL( TARGETS alice-devkit RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
INSTALL( DIRECTORY htdocs DESTINATION ${CMAKE_INSTALL_PREFIX}/share/alice )
