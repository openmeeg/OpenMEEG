#!/bin/bash -ef

# Go to the repo root
DIR=$(dirname "${BASH_SOURCE[0]}")
cd $DIR/..

if [[ "$VCPKG_DEFAULT_TRIPLET" == "" ]]; then
    export VCPKG_DEFAULT_TRIPLET="x64-windows"
fi

USE_CYPATH=1
if [[ "$VCPKG_DEFAULT_TRIPLET" == 'x64-mingw'* ]]; then
    export CMAKE_GENERATOR="MinGW Makefiles"
    export LINKER_OPT="-s"
elif [[ "$VCPKG_DEFAULT_TRIPLET" == 'x64-windows'* ]]; then
    if [[ "$CMAKE_GENERATOR" == "" ]]; then  # assume we're using an old version
        CMAKE_GENERATOR="Visual Studio 15 2017"
    fi
    if [[ "$CMAKE_GENERATOR_TOOLSET" == "" ]]; then
        if [[ "$CMAKE_GENERATOR" == "Visual Studio 15 2017" ]]; then
            CMAKE_GENERATOR_TOOLSET="v141"
        else
            CMAKE_GENERATOR_TOOLSET="v142"
        fi
    fi
    export CMAKE_GENERATOR="$CMAKE_GENERATOR"
    export CMAKE_GENERATOR_PLATFORM="x64"
    export TOOLSET_OPT="-DCMAKE_GENERATOR_TOOLSET=v141"
elif [[ "$VCPKG_DEFAULT_TRIPLET" == *'-osx'* ]] || [[ "$VCPKG_DEFAULT_TRIPLET" == 'x64-linux' ]]; then
    USE_CYGPATH=0
else
    echo "Unknown VCPKG_DEFAULT_TRIPLET: '${VCPKG_DEFAULT_TRIPLET}'"
    exit 1
fi

if [ ! -d vcpkg ]; then
    echo "Getting vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git --depth=1
    cd vcpkg
    git fetch origin 2022.05.10:use --depth=1
    git checkout use
    ./bootstrap-vcpkg.sh
    cd ..
fi
cp -v ./build_tools/vcpkg_triplets/*.cmake vcpkg/triplets
export VCPKG_INSTALLED_DIR="${PWD}/build/vcpkg_installed"
export VCPKG_INSTALL_OPTIONS="--x-install-root=$VCPKG_INSTALLED_DIR --triplet=$VCPKG_DEFAULT_TRIPLET"
export CMAKE_TOOLCHAIN_FILE="${PWD}/vcpkg/scripts/buildsystems/vcpkg.cmake"
if [[ "$VCPKG_DEFAULT_TRIPLET" != 'x64-mingw'* ]]; then  # for some reason this breaks things
    export VCPKG_TRIPLET_OPT="-DVCPKG_TARGET_TRIPLET=${VCPKG_DEFAULT_TRIPLET}"
fi

if [[ "$USE_CYGPATH" == "1" ]]; then
    export VCPKG_INSTALLED_DIR=$(cygpath -m "${VCPKG_INSTALLED_DIR}")
    export CMAKE_TOOLCHAIN_FILE=$(cygpath -m "${CMAKE_TOOLCHAIN_FILE}")
fi

if [[ "$GITHUB_ENV" != "" ]]; then
    echo "VCPKG_INSTALLED_DIR=$VCPKG_INSTALLED_DIR" >> $GITHUB_ENV
    echo "VCPKG_DEFAULT_TRIPLET=$VCPKG_DEFAULT_TRIPLET" >> $GITHUB_ENV
    echo "VCPKG_DEFAULT_HOST_TRIPLET=$VCPKG_DEFAULT_TRIPLET" >> $GITHUB_ENV
    echo "VCPKG_TRIPLET_OPT=$VCPKG_TRIPLET_OPT" >> $GITHUB_ENV
    echo "VCPKG_INSTALL_OPTIONS=$VCPKG_INSTALL_OPTIONS" >> $GITHUB_ENV
    echo "CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}" >> $GITHUB_ENV
    echo "STRIP_OPT=${STRIP_OPT}" >> $GITHUB_ENV
    echo "TOOLSET_OPT=${TOOLSET_OPT}" >> $GITHUB_ENV
    echo "LINKER_OPT=${LINKER_OPT}" >> $GITHUB_ENV
    echo "CMAKE_GENERATOR=${CMAKE_GENERATOR}" >> $GITHUB_ENV
    echo "CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}" >> $GITHUB_ENV
fi
CMAKE_TOOLCHAIN_CHECK=$CMAKE_TOOLCHAIN_FILE
if [[ "$USE_CYGPATH" == "1" ]]; then
    CMAKE_TOOLCHAIN_CHECK=$(cygpath -m "${CMAKE_TOOLCHAIN_CHECK}")
fi
echo "Checking for CMAKE_TOOLCHAIN_FILE=\"$CMAKE_TOOLCHAIN_CHECK\""
test -f "$CMAKE_TOOLCHAIN_CHECK"
