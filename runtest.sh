#!/bin/bash

TMPDIR=/dev/shm

if [[ ${PYTHON} == "" ]]
then
    PYTHON=python
fi

function print_help
{
    echo "Utility to run various tests"
    echo
    echo "Define variable PYTHON to point custom executable (if needed);"
    echo "by default standard python command is invoked."
    echo
    echo "Current settings:"
    echo "- Python interpreter:  '${PYTHON}'"
    echo "- CFLAGS:              '${CFLAGS}'"
    echo "- Selected unit tests: '${UNITTEST}' (empty value means 'all')"
    echo "  This flag is used just for 'mallocfaults' and 'pycallfaults'"
    echo "  as these tests might be really time consuming"
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
    echo "reallocfaults - recompile module with flag MEMORY_DEBUG,"
    echo "                then run unnitests injecting realloc faults"
    echo "pycallfaults  - recompile module with flag MEMORY_DEBUG,"
    echo "                then run unnitests injecting faults in python C-API calls"
    echo "coverage      - create coverage report in 'coverage' subdir"
    echo
    echo "release       - run unit, unpickle, leaks, mallocfaults and reallocfaults"
    echo "                meant to run before relese"
}

######################################################################

ACTIONS="unit unpickle leaks valgrind mallocfaults reallocfaults pycallfaults coverage release"

if [[ $# != 1 || $1 == '-h' || $1 == '--help' ]]
then
    print_help
    exit 1
fi

ACTION=
REBUILD=1

######################################################################

RED='\033[31m'
GREEN='\033[32m'
RESET='\033[0m'

MEMORY_DEBUG_PATH="${TMPDIR}/memory.dump"
MEMORY_DEBUG="-DMEMORY_DEBUG -DMEMORY_DUMP_PATH='\"${MEMORY_DEBUG_PATH}\"'"

function rebuild
{
    ${PYTHON} setup.py build_ext --inplace
    if [[ $? != 0 ]]
    then
        echo -e "${RED}Build failed${RESET}"
        exit 1
    fi
}

function force_rebuild
{
    if [[ ${REBUILD} == 1 ]]
    then
        rm -rf build 2> /dev/null
        rebuild
    fi
}

function run_unittests
{
    ${PYTHON} unittests.py ${UNITTEST}
    if [[ $? != 0 ]]
    then
        echo -e "${RED}Unit tests failed${RESET}"
        exit 1
    fi
}

function handle_unit
{
    force_rebuild
    run_unittests
}

function run_unpickletests
{
    ${PYTHON} unpickle_test.py
    if [[ $? != 0 ]]
    then
        echo -e "${RED}Unpickle tests failed${RESET}"
        exit 1
    fi
}

function handle_unpickle
{
    force_rebuild
    run_unpickletests
}

function run_leaktest
{
    ${PYTHON} tests/memdump_check.py ${MEMORY_DEBUG_PATH}
    if [[ $? != 0 ]]
    then
        echo -e "${RED}Memory leaks detected${RESET}"
        exit 1
    fi
}

function handle_leaks
{
    export CFLAGS="${CFLAGS} ${MEMORY_DEBUG}"
    force_rebuild

    run_unittests
    run_leaktest
}

function handle_valgrind
{
    force_rebuild

    local LOGFILE=${TMPDIR}/valgrind.log
    echo "Running valgrind..."
    valgrind --log-file=${LOGFILE} --leak-check=full --track-origins=yes ${PYTHON} unittests.py
    ${PYTHON} tests/valgrind_check.py . ${LOGFILE}
}

function run_mallocfaults
{
    # obtain max allocation number
    unset ALLOC_FAIL
    unset REALLOC_FAIL
    run_unittests

    local MINID=0
    echo ${MEMORY_DEBUG_PATH}
    local MAXID=$(${PYTHON} tests/memdump_maxalloc.py ${MEMORY_DEBUG_PATH})

    # simulate failures of all allocations
    for ID in `seq ${MINID} ${MAXID}`
    do
        echo "Checking memalloc fail ${ID} of ${MAXID}"
        mallocfault ${ID}
    done
}

function mallocfault
{
    export ALLOC_NODUMP=1
    export ALLOC_FAIL=$1

    local LOG=${TMPDIR}/mallocfault${ID}.log
    ${PYTHON} unittests.py ${UNITTEST} -q > ${LOG} 2>&1
    if [[ $? == 139 ]]
    then
        echo -e "${RED}SEGFAULT${RESET}"
        exit 1
    fi
    ${PYTHON} tests/unittestlog_check.py ${LOG}
    if [[ $? != 0 ]]
    then
        echo -e "${RED}Possible error${RESET}"
        echo "Inspect ${LOG}, there are errors other than expected MemoryError"
        exit 1
    fi
}

function handle_mallocfaults
{
    export CFLAGS=${MEMORY_DEBUG}
    force_rebuild

    run_mallocfaults
}

function run_reallocfaults
{
    # obtain max allocation number
    unset ALLOC_FAIL
    unset REALLOC_FAIL
    run_unittests

    local MINID=0
    echo ${MEMORY_DEBUG_PATH}
    local MAXID=$(${PYTHON} tests/memdump_maxrealloc.py ${MEMORY_DEBUG_PATH})

    # simulate failures of all allocations
    for ID in `seq ${MINID} ${MAXID}`
    do
        echo "Checking realloc fail ${ID} of ${MAXID}"
        reallocfault ${ID}
    done
}

function reallocfault
{
    export ALLOC_NODUMP=1
    export REALLOC_FAIL=$1

    local LOG=${TMPDIR}/reallocfault${ID}.log
    ${PYTHON} unittests.py ${UNITTEST} -q > ${LOG} 2>&1
    if [[ $? == 139 ]]
    then
        echo -e "${RED}SEGFAULT${RESET}"
        exit 1
    fi
    ${PYTHON} tests/unittestlog_check.py ${LOG}
    if [[ $? != 0 ]]
    then
        echo -e "${RED}Possible error${RESET}"
        echo "Inspect ${LOG}, there are errors other than expected MemoryError"
        exit 1
    fi
}

function handle_reallocfaults
{
    export CFLAGS=${MEMORY_DEBUG}
    force_rebuild

    run_reallocfaults
}

function handle_pycallfaults
{
    export CFLAGS="-DPYCALLS_INJECT_FAULTS"
    force_rebuild

    local TMP=${TMPDIR}/pycallfaults
    ${PYTHON} unittests.py ${UNITTEST} > ${TMP}

    local MINID=0
    local MAXID=$(awk '
                        /^Fail ID: / {if ($3 > max) max=$3}
                        END {print max}
    ' ${TMP})

    # simulate failures of all call to Python C-API
    for ID in `seq 0 ${MAXID}`
    do
        echo -n "Checking Python C-API fail ${ID} of ${MAXID}"
        local LOG=${TMPDIR}/pycallfaults${ID}.log
        export PYCALL_FAIL=${ID}
        ${PYTHON} unittests.py ${UNITTEST} > ${LOG} 2>&1
        echo " return code $?"
        ${PYTHON} tests/pyfault_check.py ${LOG}
    done
}

function handle_coverage
{
    local DIR=coverage
    local INDEX=pyahocorasick.html

    mkdir ${DIR} 2> /dev/null
    gcovr --html-details -o ${DIR}/${INDEX}
    echo "Navigate to ${DIR}/${INDEX}"
}

function handle_release
{
    unset ALLOC_FAIL
    unset UNITTEST
    unset CFLAGS

    # 1. build with default settings and run unit tests and unpickle tests
    if true
    then
        force_rebuild > /dev/null 2>&1

        run_unittests
        run_unpickletests
    fi

    # 2. build with memory debug and run unit tests and unpickle tests
    if true
    then
        export CFLAGS="${MEMORY_DEBUG}"
        force_rebuild > /dev/null 2>&1

        rm -f ${MEMORY_DUMP_PATH}
        run_unittests
        run_leaktest

        rm -f ${MEMORY_DUMP_PATH}
        run_unpickletests
        run_leaktest
    fi

    # 3. inject malloc faults
    if true
    then
        export CFLAGS="${MEMORY_DEBUG}"
        force_rebuild > /dev/null 2>&1

        run_mallocfaults
    fi

    echo -e "${GREEN}All OK${RESET}"
}

###################################################

arg=$1
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

    reallocfaults)
        handle_reallocfaults
        ;;

    pycallfaults)
        handle_pycallfaults
        ;;

    coverage)
        handle_coverage
        ;;

    release)
        handle_release
        ;;

    *)
        echo "Unknown action '${arg}'"
        usage
        exit 2
        ;;
esac
