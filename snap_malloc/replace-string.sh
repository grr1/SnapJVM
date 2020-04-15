#!/bin/bash

for t in myMalloc.h myMalloc.c printing.c testing.c;
do
    cat $t | \
    sed s/size_state/size_and_state/g | \
    sed s/get_size/get_block_size/g | \
    sed s/set_size/set_block_size/g | \
    sed s/get_state/get_block_state/g | \
    sed s/set_state/set_block_state/g | \
    sed s/set_size_and_state/set_block_size_and_state/g > $t-final || echo error
    mv $t-final $t
done;

