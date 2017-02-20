option(GIT_HTTPS "Use https protocol to get git repositories." ON)
set(GIT_PREFIX "git@")
if (GIT_HTTPS)
    set(GIT_PREFIX "https://")
endif()

#   Various OpenMEEG options that will be forwarded.

include(BlasLapackOption)
option(ENABLE_PACKAGING "Enable Packaging" OFF)
option(ENABLE_PYTHON "Enable Python Wrapping" ON)
option(USE_OMP "Use OpenMP" OFF)
option(USE_GIFTI "Use GIFTI IO support" OFF)
option(USE_VTK "Use VTK" OFF)
option(USE_CGAL "Use CGAL meshing tools" OFF)
option(BUILD_TESTING "Build the testing tree" ON)
option(BUILD_DOCUMENTATION "Build the documentation" ON)

#   Various matio options that will be forwarded.

option(MATIO_BUILD_TESTING "Build matio tests" OFF)
option(MATLAB_TESTING "Enable matlab read tests (requires a function matlab)" OFF)
