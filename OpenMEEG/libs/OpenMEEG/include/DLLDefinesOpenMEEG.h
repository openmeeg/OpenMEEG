#pragma once

//  Unify with DLLDefinesOpenMEEGMaths.h ???

// Cmake will define OpenMEEG_EXPORTS on Windows when it
// configures to build a shared library. If you are going to use
// another build system on windows or create the visual studio
// projects by hand you need to define OpenMEEG_EXPORTS when
// building a DLL on windows.

#if defined (WIN32)
    #if defined(OpenMEEG_EXPORTS)
        #define  OPENMEEG_EXPORT __declspec(dllexport)
    #else
        #define  OPENMEEG_EXPORT __declspec(dllimport)
    #endif /* OpenMEEG_EXPORTS */
#else /* defined (WIN32) */
    #define OPENMEEG_EXPORT
#endif
