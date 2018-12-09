#!/bin/bash

TMPDIR=/dev/shm

function print_help
{
    echo "Utility to run various tests"
    echo
    echo "Define variable PYTHON to point custom executable (if needed);"
    echo "by default standard python command is invoked"
    echo
    usage
}

function usage
{
    echo "$0 unit|unpickle|leaks|valgrind|mallocfaults|pycallfaults"
    echo
    echo "unit          - run default unit tests"
    echo "unpickle      - run unpickle tests, which depend on machine"
    echo "leaks         - recompile module with flag MEMORY_DEBUG,"
    echo "                then run unittests and check if there were memory leaks"
    echo "valgrind      - run unittests in valgrind and check if there are"
    echo "                any leaks from pyahocorasick"
    echo "mallocfaults  - recompile module with flag MEMORY_DEBUG,"
    echo "                then run unnitests injecting malloc faults"
    echo "pycallfaults  - recompile module with flag MEMORY_DEBUG,"
    echo "                then run unnitests injecting faults in python C-API calls"
}

######################################################################

ACTIONS="unit unpickle leaks valgrind mallocfaults pycallfaults"

if [[ $# != 1 || $1 == '-h' || $1 == '--help' ]]
then
    print_help
    exit 1
fi
 
ACTION=
REBUILD=0
arg=$1
for action in ${ACTIONS}
do
    if [[ ${arg} == ${action} ]]
    then
        ACTION=${arg}
        break
    fi
done

if [[ ${ACTION} == "" ]]
then
    echo "Unknown action '${arg}'"
    usage
    exit 2
fi

######################################################################

PYTHON=python

function rebuild
{
    ${PYTHON} setup.py build_ext --inplace
}

function force_rebuild
{
    if [[ ${REBUILD} == 1 ]]
    then
        rm -rf build 2> /dev/null
        rebuild
    fi
}

function handle_unit
{
    ${PYTHON} unittests.py
}

function handle_unpickle
{
    ${PYTHON} unpickle_test.py
}

function handle_leaks
{
    export CFLAGS="-DMEMORY_DEBUG"
    force_rebuild

    ${PYTHON} unittests.py > /dev/null
    if [[ $? != 0 ]]
    then
        echo "Unit tests failed!"
        exit 1
    fi

    ${PYTHON} tests/memdump_check.py
    if [[ $? == 0 ]]
    then
        echo "All OK"
    fi
}

function handle_valgrind
{
    export CFLAGS=""
    force_rebuild

    LOGFILE=${TMPDIR}/valgrind.log
    echo "Running valgrind..."
    valgrind --log-file=${LOGFILE} ${PYTHON} unittests.py
    ${PYTHON} tests/valgrind_check.py . ${LOGFILE}
}

function handle_mallocfaults
{
    echo "Not implemented yet"
}

function handle_pycallfaults
{
    echo "Not implemented yet"
}

case "${arg}"
in
    unit)
        handle_unit
        ;;

    unpickle)
        handle_unpickle
        ;;

    leaks)
        handle_leaks
        ;;

    valgrind)
        handle_valgrind
        ;;

    mallocfaults)
        handle_mallocfaults
        ;;

    pycallfaults)
        handle_pycallfaults
        ;;
esac
