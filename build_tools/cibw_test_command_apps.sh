#!/bin/bash

set -e
if [[ "$1" == "" ]]; then
    echo "Usage: $0 <PROJECT_PATH>"
    exit 1
fi
ROOT=$1

set -e
ls -al $ROOT
echo
ls -alR $ROOT/installers
echo
ls -al $ROOT/installers/OpenMEEG-*-*.*
echo

if [[ "${RUNNER_OS}" == "Linux" ]]; then
    tar xzfv $ROOT/installers/OpenMEEG-*-*.tar.gz
    cd OpenMEEG-*
    ./bin/om_minverser --help
elif [[ "${RUNNER_OS}" == "macOS" ]]; then
    tar xzfv $ROOT/installers/OpenMEEG-*-*.tar.gz
    cd OpenMEEG-*
    ./bin/om_minverser --help
elif [[ "${RUNNER_OS}" == "Windows" ]]; then
    ROOT=$(cygpath -u $ROOT)
    tar xzfv $ROOT/installers/OpenMEEG-*-*.tar.gz
    cd OpenMEEG-*
    om_minverser --help
else
    echo "Unknown RUNNER_OS=\"${RUNNER_OS}\""
    exit 1
fi
