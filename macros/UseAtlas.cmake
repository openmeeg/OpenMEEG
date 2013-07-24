IF (WIN32)
#        OPTION(USE_ATLAS "Build the project using ATLAS" OFF)
        OPTION(USE_MKL "Build the project with MKL" ON)
        MARK_AS_ADVANCED(USE_MKL)
#        MARK_AS_ADVANCED(USE_ATLAS)
        IF (NOT BUILD_SHARED_LIBS)
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.dll")
        ENDIF()
ELSE()
    OPTION(USE_ATLAS "Build the project using ATLAS" ON)
    OPTION(USE_MKL "Build the project with MKL" OFF)
    IF (APPLE)   # MACOSX
        IF (NOT BUILD_SHARED_LIBS)
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so;.dylib")
        ENDIF()
    ELSE() # LINUX
        IF (NOT BUILD_SHARED_LIBS)
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
        ENDIF()
    ENDIF()
ENDIF()

IF (USE_MKL)
        FIND_PACKAGE(MKL)
	IF (MKL_FOUND)
		INCLUDE_DIRECTORIES(${MKL_INCLUDE_DIR})
		SET(LAPACK_LIBRARIES ${MKL_LIBRARIES})
        #MESSAGE(${LAPACK_LIBRARIES}) # for debug
		IF(UNIX AND NOT APPLE) # MKL on linux requires to link with the pthread library
			SET(LAPACK_LIBRARIES ${LAPACK_LIBRARIES} pthread)
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR "MKL not found. Please set environment variable MKLDIR")
	ENDIF()
ENDIF()

IF (NOT USE_MKL)
    #   ATLAS OR LAPACK/BLAS
    IF (UNIX AND NOT APPLE)
        IF (USE_ATLAS)
            SET(ATLAS_LIB_SEARCHPATH
                /usr/lib64/
                /usr/lib64/atlas
                /usr/lib64/atlas/sse2
                /usr/lib/atlas/sse2
                /usr/lib/sse2
                /usr/lib64/atlas/sse3
                /usr/lib/atlas/sse3
                /usr/lib/sse3
                /usr/lib/
                /usr/lib/atlas
                /usr/lib/atlas-base
                /usr/lib64/atlas-base
                )
            SET(ATLAS_LIBS atlas cblas f77blas clapack lapack blas)

            FIND_PATH(ATLAS_INCLUDE_PATH clapack.h /usr/include/atlas /usr/include/ NO_DEFAULT_PATH)
            FIND_PATH(ATLAS_INCLUDE_PATH clapack.h)
            MARK_AS_ADVANCED(ATLAS_INCLUDE_PATH)
            INCLUDE_DIRECTORIES(${ATLAS_INCLUDE_PATH})
            FOREACH (LIB ${ATLAS_LIBS})
                SET(LIBNAMES ${LIB})
                IF (${LIB} STREQUAL "clapack")
                    SET(LIBNAMES ${LIB} lapack_atlas)
                ENDIF()
                FIND_LIBRARY(${LIB}_PATH
                    NAMES ${LIBNAMES}
                    PATHS ${ATLAS_LIB_SEARCHPATH}
                    NO_DEFAULT_PATH
                    NO_CMAKE_ENVIRONMENT_PATH
                    NO_CMAKE_PATH
                    NO_SYSTEM_ENVIRONMENT_PATH
                    NO_CMAKE_SYSTEM_PATH)
                IF(${LIB}_PATH)
                    SET(LAPACK_LIBRARIES ${LAPACK_LIBRARIES} ${${LIB}_PATH})
                    MARK_AS_ADVANCED(${LIB}_PATH)
                ELSE()
                    MESSAGE(WARNING "Could not find ${LIB}")
                ENDIF()
            ENDFOREACH()
        ELSE()
            FIND_PACKAGE(lapack QUIET PATHS /usr/lib64/ /usr/lib/
                         NO_DEFAULT_PATH
                         NO_CMAKE_ENVIRONMENT_PATH
                         NO_CMAKE_PATH
                         NO_SYSTEM_ENVIRONMENT_PATH
                         NO_CMAKE_SYSTEM_PATH)
        ENDIF()

        IF (NOT BUILD_SHARED_LIBS)
            FILE(GLOB GCC_FILES "/usr/lib/gcc/*/*")
            FIND_FILE(GFORTRAN_LIB libgfortran.a ${GCC_FILES})
            SET(LAPACK_LIBRARIES ${LAPACK_LIBRARIES} ${GFORTRAN_LIB})
        ENDIF()
    ELSE() # Assume APPLE or local lapack/blas (treated in contrib)
        IF (APPLE)
            SET(LAPACK_LIBRARIES "-framework vecLib")
            INCLUDE_DIRECTORIES(/System/Library/Frameworks/vecLib.framework/Headers)
        ENDIF()
    ENDIF()
ENDIF()

IF (NOT LAPACK_LIBRARIES)
    SET(NEED_CLAPACK TRUE)
    SET(LAPACK_LIBRARIES lapack blas f2c)
ENDIF()

SET(HAVE_LAPACK TRUE)
SET(HAVE_BLAS TRUE)
