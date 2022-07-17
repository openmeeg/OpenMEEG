#!/bin/bash

# Build and install (locally) OpenMEEG to prepare for SWIG-building the Python
# bindings separately

set -e
if [[ "$1" == "" ]]; then
    echo "Usage: $0 <PROJECT_PATH>"
    exit 1
fi
ROOT=$1
echo "Using project root \"${ROOT}\" on RUNNER_OS=\"${RUNNER_OS}\""
cd $ROOT
pwd

# Let's have NumPy help us out
curl -L https://github.com/numpy/numpy/archive/refs/tags/v1.23.1.tar.gz | tar xz numpy-1.23.1
mv numpy-1.23.1/tools .
mv numpy-1.23.1/numpy .  # on Windows, _distributor_init gets modified
echo "Running NumPy tools/wheels/cibw_before_build.sh $1"
chmod +x ./tools/wheels/cibw_before_build.sh
./tools/wheels/cibw_before_build.sh $1
PLATFORM=$(PYTHONPATH=tools python -c "import openblas_support; print(openblas_support.get_plat())")
rm -Rf numpy numpy-1.23.1 tools
echo "Using NumPy PLATFORM=\"${PLATFORM}\""

# PLATFORM can be:
# linux-x86_64
# macosx-x86_64
# win-amd64

if [[ "$PLATFORM" == "linux-x86_64" ]]; then
    dnf -y install epel-release
    dnf -y install hdf5-devel matio-devel
    export OPENBLAS_INCLUDE=/usr/local/include
    export OPENBLAS_LIB=/usr/local/lib
    ls -al $OPENBLAS_LIB
    # source ./build_tools/download_openblas.sh linux
    # BLAS_LIBRARIES_OPT="-DBLAS_LIBRARIES=$OPENBLAS_LIB/libopenblas.so"
    # LAPACK_LIBRARIES_OPT="-DLAPACK_LIBRARIES=$OPENBLAS_LIB/libopenblas.so"
    export CMAKE_CXX_FLAGS="-lgfortran -I$OPENBLAS_INCLUDE"
elif [[ "$PLATFORM" == "macosx-x86_64" ]]; then
    #brew install hdf5 libmatio boost swig openblas
    #BLAS_DIR=/usr/local/opt/openblas
    brew install boost swig
    BLAS_DIR=/usr/local
    OPENBLAS_INCLUDE=$BLAS_DIR/include
    OPENBLAS_LIB=$BLAS_DIR/lib
    export CMAKE_CXX_FLAGS="-I$OPENBLAS_INCLUDE -L$OPENBLAS_LIB"
    export CMAKE_PREFIX_PATH="$BLAS_DIR"
    echo "MACOSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET"
    if [[ "${MACOSX_DEPLOYMENT_TARGET}" == "" ]]; then
        export MACOSX_DEPLOYMENT_TARGET="10.9"
    fi
    # TODO: Need to add arm64 target here, probably via arm64-osx
    export VCPKG_DEFAULT_TRIPLET="x64-osx"
    # https://github.com/microsoft/vcpkg/issues/10038
    export VCPKG_OSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}"
    source ./build_tools/setup_vcpkg_compilation.sh
    VCPKG_BUILD_TYPE_OPT="-DVCPKG_BUILD_TYPE=release"
    VCPKG_BUILD_C_FLAGS_OPT="-DVCPKG_C_FLAGS=-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"
    VCPKG_BUILD_CXX_FLAGS_OPT="-DVCPKG_CXX_FLAGS=-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"
elif [[ "$PLATFORM" == "win-amd64" ]]; then
    export VCPKG_DEFAULT_TRIPLET="x64-windows"
    export CMAKE_GENERATOR="Visual Studio 16 2019"
    source ./build_tools/setup_vcpkg_compilation.sh
    source ./build_tools/download_openblas.sh windows  # NumPy doesn't install the headers for Windows
    pip install delvewheel
    VCPKG_BUILD_TYPE_OPT="-DVCPKG_BUILD_TYPE=release"
    SYSTEM_VERSION_OPT="-DCMAKE_SYSTEM_VERSION=7"
else
    echo "Unknown platform: ${PLATFORM}"
    exit 1
fi
export PYTHON_OPT="-DENABLE_PYTHON=OFF"
export BLA_IMPLEMENTATION="OpenBLAS"
export DISABLE_CCACHE=1
pip install cmake
./build_tools/cmake_configure.sh -DCMAKE_INSTALL_PREFIX=${ROOT}/install ${VCPKG_BUILD_TYPE_OPT} ${VCPKG_BUILD_C_FLAGS_OPT} ${VCPKG_BUILD_CXX_FLAGS_OPT} ${SYSTEM_VERSION_OPT} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_UCRT_LIBRARIES=TRUE ${BLAS_LIBRARIES_OPT} ${LAPACK_LIBRARIES_OPT}
cmake --build build --target install
# make life easier for auditwheel/delocate/delvewheel
if [[ "$PLATFORM" == 'linux'* ]]; then
    ls -al install/lib/*.so*
    cp install/lib/*.so* /usr/local/lib/
elif [[ "$PLATFORM" == 'macosx'* ]]; then
    ls -al install/lib/*.dylib*
    sudo mkdir -p /usr/local/lib
    sudo cp install/lib/*.dylib* /usr/local/lib/
else
    ls -al $PWD/install/bin/*.dll*
    cp $PWD/install/bin/*.dll* .
fi
