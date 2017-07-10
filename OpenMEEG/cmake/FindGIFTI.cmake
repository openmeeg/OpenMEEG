
find_library(GIFTI_LIBRARY giftiio ${FIND_MODE} PATHS ${GIFTI_DIR}/lib)
find_path(GIFTI_INCLUDE_PATH gifti_io.h PATH_SUFFIXES gifti ${FIND_MODE} PATHS ${GIFTI_DIR}/include/gifti)

if (GIFTI_FIND_REQUIRED)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(GIFTI DEFAULT_MSG GIFTI_INCLUDE_PATH GIFTI_LIBRARY)
endif()
