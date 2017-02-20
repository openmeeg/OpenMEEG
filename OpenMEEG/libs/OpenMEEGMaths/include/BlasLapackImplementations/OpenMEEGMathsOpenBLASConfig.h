#pragma once

#include <BlasLapackImplementations/FortranCInterface.h>

extern "C" {
    #include <cblas.h>
}
#include <lapacke.h>
#undef I // undefine this def due to complex.h that causes issues later

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

#include <BlasLapackImplementations/OpenMEEGMathsCBlasLapack.h>
