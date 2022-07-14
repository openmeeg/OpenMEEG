#!/bin/bash -ef

# Build and install (locally) OpenMEEG to prepare for SWIG-building the Python
# bindings separately

if [[ "$1" == "" ]]; then
    echo "Usage: $0 <PROJECT_PATH>"
    exit 1
fi
ROOT=$1
PLATFORM=$(python -c "import sys; print(sys.platform)")
echo "Using project root for platform \"${PLATFORM}\": \"${ROOT}\""
pip install cmake
if [[ "$PLATFORM" == "linux" ]]; then
    sudo apt -yq install liblapacke-dev libhdf5-dev libmatio-dev libopenblas-dev
elif [[ "$PLATFORM" == "darwin" ]]; then
    brew install hdf5 libmatio boost swig openblas
    BLAS_DIR=/usr/local/opt/openblas
    OPENBLAS_INCLUDE=$BLAS_DIR/include
    OPENBLAS_LIB=$BLAS_DIR/lib
    export CMAKE_CXX_FLAGS="-I$OPENBLAS_INCLUDE -L$OPENBLAS_LIB"
    export CMAKE_PREFIX_PATH="$BLAS_DIR"
elif [[ "$PLATFORM" == "win32" ]]; then
    source ${ROOT}/build_tools/setup_windows_compilation.sh
    source ${ROOT}/build_tools/download_openblas_windows.sh
    pip install delvewheel
else
    echo "Unknown platform: ${PLATFORM}"
    exit 1
fi
export PYTHON_OPT=-DENABLE_PYTHON=OFF
export STATIC_OPT=-DBLA_STATIC=ON
./cmake_configure.sh -DCMAKE_INSTALL_PREFIX=$ROOT/install
cmake --build build --target install
