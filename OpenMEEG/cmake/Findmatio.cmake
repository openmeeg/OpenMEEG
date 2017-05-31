# Find the matio headers and library.
#
#  matio_INCLUDE_DIRS - where to find matio.h, etc.
#  matio_LIBRARIES    - List of libraries.
#  matio_FOUND        - True if matio found.

#   We provide a module in case matio has not been found in config mode.

if (NOT matio_LIBRARIES)

    find_package(HDF5 REQUIRED)

    # Look for the header file.

    find_path(matio_INCLUDE_DIR NAMES matio.h)
    mark_as_advanced(matio_INCLUDE_DIR)

    # Look for the library.

    find_library(matio_LIBRARY NAMES matio)
    mark_as_advanced(matio_LIBRARY)

    # handle the QUIETLY and REQUIRED arguments and set matio_FOUND to TRUE if 
    # all listed variables are TRUE

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(matio DEFAULT_MSG matio_LIBRARY matio_INCLUDE_DIR)
    
    if (MATIO_FOUND)
        set(matio_FOUND TRUE)
        set(matio_LIBRARIES ${matio_LIBRARY} ${HDF5_LIBRARIES})
        set(matio_INCLUDE_DIRS ${matio_INCLUDE_DIR} ${HDF5_INCLUDE_DIR})
        if (APPLE)
            set(LIBS)
            foreach(lib ${matio_LIBRARY} ${HDF5_LIBRARIES})
                get_filename_component(reallib ${lib} REALPATH)
                list(APPEND LIBS ${reallib})
            endforeach()
            install(FILES ${LIBS} DESTINATION lib
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ)
        endif()
    else()
        set(matio_LIBRARIES)
        set(matio_INCLUDE_DIRS)
    endif()

    include(matioVersion)
endif()
