#!/bin/bash

set -e
if [[ "$1" == "" ]]; then
    echo "Usage: $0 <PROJECT_PATH>"
    exit 1
fi
ROOT=$1
echo "Using project root \"${ROOT}\" on RUNNER_OS=\"${RUNNER_OS}\""
cd $ROOT
pwd

# Build the Python bindings on Windows
ls -al
rm -Rf build
cp -a build_nopython build
which python
python --version
python -m pip install oldest-supported-numpy
cmake -B build -DENABLE_PYTHON=ON -DPython3_EXECUTABLE="$(which python)" .
cmake --build build
python -m pip uninstall numpy
cp -av build/wrapping/python/openmeeg/*.pyd build/wrapping/python/openmeeg/openmeeg.py wrapping/python/openmeeg/
rm -Rf build
