#!/bin/bash

WORDS=250000
INPUT=words.gz
PICKLE=pickle.memusage
UNPICKLE=unpickle.memusage

function prepare_input
{
    if [[ ! -f ${INPUT} ]]
    then
        python3 tests/generate_random_words.py --max-words=${WORDS} | gzip > ${INPUT}
    fi
}

function measure_pickling
{
    rm -f ${PICKLE}
    python3 tests/pickle_stresstest.py --file-gz=${INPUT} --max-words=${WORDS} -p &
    statm -p $! -d 100 -o ${PICKLE}
}

function measure_unpickling
{
    rm -f ${UNPICKLE}
    python3 tests/pickle_stresstest.py --random -u &
    statm -p $! -d 20 -o ${UNPICKLE}
}

prepare_input

if true
then
    measure_pickling
    echo -n "pickle memory usage   : "
    awk '{print $7 * 4096}' < ${PICKLE} | sort -rn | head -n1
fi

if false
then
    measure_unpickling
    echo -n "unpickle memory usage : "
    awk '{print $7 * 4096}' < ${UNPICKLE} | sort -rn | head -n1
fi
