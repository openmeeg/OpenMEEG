#pragma once

#if defined (WIN32)
    #define FC_GLOBAL(x,X) x ## _
#else
    #include <FC.h>
#endif

//  We do dot use lapacke.h because it defines lapack interfaces with pointers.
//  Instead, we use our own interfaces with leverage C++ references.

extern "C" {
    #include <cblas.h>
    #include <lapacke.h>
}

#define BLAS(x,X) cblas_ ## x
#define LAPACK(x,X) LAPACKE_ ## x

#define CLAPACK_INTERFACE

#define DLANGE(X1,X2,X3,X4,X5,X6)       LAPACK(dlange,DLANGE)(LAPACK_COL_MAJOR,X1,X2,X3,X4,X5)

#define DSPTRF(X1,X2,X3,X4,X5)          LAPACK(dsptrf,DSPTRF)(LAPACK_COL_MAJOR,X1,X2,X3,X4)
#define DSPTRS(X1,X2,X3,X4,X5,X6,X7,X8) LAPACK(dsptrs,DSPTRS)(LAPACK_COL_MAJOR,X1,X2,X3,X4,X5,X6,X7)
#define DSPTRI(X1,X2,X3,X4,X5,X6)       LAPACK(dsptri,DSPTRI)(LAPACK_COL_MAJOR,X1,X2,X3,X4)
#define DPPTRF(X1,X2,X3,X4)             LAPACK(dpptrf,DPPTRF)(LAPACK_COL_MAJOR,X1,X2,X3)
#define DPPTRI(X1,X2,X3,X4)             LAPACK(dpptri,DPPTRI)(LAPACK_COL_MAJOR,X1,X2,X3)
#define DGETRF(X1,X2,X3,X4,X5)          LAPACK(dgetrf,DGETRF)(LAPACK_COL_MAJOR,X1,X2,X3,X4,X5)
#define DGETRI(X1,X2,X3,X4)             LAPACK(dgetri,DGETRI)(LAPACK_COL_MAJOR,X1,X2,X3,X4)

#define DGESDD(X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,X13,X14) LAPACK(dgesdd,DGESDD)(LAPACK_COL_MAJOR,X1,X2,X3,X4,X5,X6,X7,X8,X9,X10)

//#define LAPACK(x,X) FC_GLOBAL(x,X)

//#include <BlasLapackImplementations/lapack.h>
#include <BlasLapackImplementations/OM_C_BlasLapack.h>
