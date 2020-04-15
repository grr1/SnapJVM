#!/bin/bash

# arguments
testname=$1

# kill runaway processes and clean up pipes on exit or interrupt
trap 'killall $1 2> /dev/null; rm -rf testPipe expectedPipe' EXIT INT

# Create fifos
mkfifo testPipe
mkfifo expectedPipe

# Output test and solution output and error to the fifos
tests/$testname > testPipe 2>&1 &
tests/expected/$testname > expectedPipe 2>&1 &

# Diff the contents of the fifos and output the difference
timeout 5 diff testPipe expectedPipe || echo "Timeout Failure"
