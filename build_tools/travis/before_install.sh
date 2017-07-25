if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

    # Install some custom requirements on OS X
    brew tap homebrew/science # a lot of cool formulae for scientific tools
    brew tap homebrew/python # numpy, scipy, matplotlib, ...
    brew update && brew upgrade

    # Install some custom requirements on OS X
    if [[ "$USE_PROJECT" == "0" || "$USE_SYSTEM" == "1" ]]; then
        brew install hdf5
        brew install libmatio
    fi

    # install a brewed python
    # To use Python of
    if [[ "$USE_PYTHON" == "1" ]]; then
        brew install python
        brew install numpy
        brew install swig
    fi

    if [[ "$BLASLAPACK_IMPLEMENTATION" == "OpenBLAS" || "$BLASLAPACK_IMPLEMENTATION" == "Auto" ]]; then
        brew install openblas
        brew link openblas --force  # required as link is not automatic
    fi

    if [[ "$USE_VTK" == "1" && "$ENABLE_PACKAGING" != "1" ]]; then
        brew install vtk
    fi

    if [[ "$USE_OMP" == 1 ]]; then
        brew install llvm
        export OMP_NUM_THREADS=4
        export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
        export DYLD_LIBRARY_PATH="/usr/local/opt/llvm/lib:$DYLD_LIBRARY_PATH"
        export CC="/usr/local/opt/llvm/bin/clang"
        export CXX="/usr/local/opt/llvm/bin/clang++"
        export CPPFLAGS="-I/usr/local/opt/llvm/include -fopenmp $CPPFLAGS"
        export CFLAGS="-I/usr/local/opt/llvm/include -fopenmp $CFLAGS"
    fi

    if [[ "$USE_CGAL" == 1 ]]; then
        brew install cgal
    fi

    if [[ "$BUILD_DOCUMENTATION" == "ON" ]]; then
        brew install Doxygen Graphviz # For building documentation
    fi

else
    # Install some custom requirements on Linux
    export CXX="g++-4.8"; 

    # clang 3.4
    if [ "$CXX" == "clang++" ]; then
        export CXX="clang++-3.4";
    fi

    if [[ "$USE_PROJECT" == "0" || "$USE_SYSTEM" == "1" ]]; then
        sudo apt-get install -y libhdf5-serial-dev libmatio-dev
    fi

    if [[ "$USE_CGAL" == 1 ]]; then
        sudo apt-get install -y libcgal-dev
    fi

    if [[ "$USE_GIFTI" == 1 ]]; then
        sudo apt-get install -y libnifti-dev libgiftiio-dev
    fi

    if [[ "$BLASLAPACK_IMPLEMENTATION" == "Atlas" ]]; then
        sudo apt-get install -y libatlas-dev libatlas-base-dev
    elif [[ "$BLASLAPACK_IMPLEMENTATION" == "LAPACK" ]]; then
        if [[ "$USE_PROJECT" == "0" || "$USE_SYSTEM" == "1" ]]; then
            sudo apt-get install -y liblapack-dev libblas-dev
        fi
    elif [[ "$BLASLAPACK_IMPLEMENTATION" == "OpenBLAS" ]]; then
        sudo apt-get install -y libopenblas-dev liblapacke-dev
    fi

    if [[ "$USE_PYTHON" == "1" ]]; then
        sudo apt-get install -y swig python-dev python-numpy
    fi

    if [[ "$USE_VTK" == "1" && "$STANDALONE" != "1" ]]; then
        sudo apt-get install libvtk5-dev
    fi

    if [[ "$BUILD_DOCUMENTATION" == "ON" ]]; then
        sudo apt-get install -y doxygen graphviz gnuplot
    fi

    if [[ "$USE_COVERAGE" == "1" ]]; then
        sudo apt-get install -y lcov
    fi
fi
