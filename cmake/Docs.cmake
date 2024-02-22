find_package(Doxygen)

# Add a target to generate API documentation with Doxygen

if(DOXYGEN_FOUND)
    add_custom_target(
        docs
        ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
    )
endif()
