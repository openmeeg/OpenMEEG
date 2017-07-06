
find_package(EXPAT ${FIND_MODE})
find_package(ZLIB ${FIND_MODE})
find_library(NIFTI_LIBRARY niftiio ${FIND_MODE} PATHS ${NIFTI_DIR}/lib ${NIFTI_ROOT}/lib)

find_library(ZNZ_LIBRARY znz ${FIND_MODE} PATHS ${NIFTI_DIR}/lib ${NIFTI_ROOT}/lib)
find_path(NIFTI_INCLUDE_PATH nifti1_io.h PATH_SUFFIXES nifti ${FIND_MODE} PATHS ${NIFTI_DIR}/include/nifti ${NIFTI_ROOT}/include/nifti)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NIFTI DEFAULT_MSG NIFTI_INCLUDE_PATH NIFTI_LIBRARY)
