# Find alice

find_path( ALICE_INCLUDE_DIR
    NAMES alice/reader.hpp
    HINTS ${ALICE_ROOT}include ${CMAKE_INCLUDE_PATH}
)

find_library( ALICE_LIBRARIES
    NAMES alice-core-shared
    HINTS ${ALICE_ROOT}/lib ${CMAKE_LIBRARY_PATH}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Alice DEFAULT_MSG
    ALICE_LIBRARIES
    ALICE_INCLUDE_DIR
)

mark_as_advanced(
    ALICE_ROOT
    ALICE_LIBRARIES
    ALICE_INCLUDE_DIR
)