#!/bin/bash

# checking working directory
if [ "SnapJVM" != "${PWD##*/}" ] ; then
	echo "Please run this testall from the SnapJVM directory directly"
	exit -1
fi

# may change these to specify location of tests cases/output
TEST_FOLDER="test"
TEST_OUT_FOLDER="test_out"
TEST_SRC_FOLDER="test_src"
TESTS_PASSED="tests_passed"
TESTS_FAILED="tests_failed"

# to keep count of success rate
echo 0 > ./$TEST_OUT_FOLDER/$TESTS_PASSED
echo 0 > ./$TEST_OUT_FOLDER/$TESTS_FAILED

echo -e "\033[33m========== TESTALL SNAPJVM =============\033[0m"

for test_file in $(ls ./$TEST_SRC_FOLDER/${1%.*}) ; do
(
    if [[ "${test_file: -5}" == ".java" ]] ; then
		echo -e "\033[33m***** Compiling ${test_file::-5}\033[0m"
	        javac -d ./$TEST_FOLDER ./$TEST_SRC_FOLDER/${test_file}
		continue
	fi
)
done

# all filenames (without extensions) in the test directory
for test_file in $(ls ./$TEST_FOLDER/${1%.*}) ; do
(
	# check for ".java" suffix
	if [[ "${test_file: -5}" == ".java" ]] ; then
		echo "-----"
		echo -e "\033[31mERROR: .java files belong in test_src\033[0m"
		echo -e "Move \033[35m$test_file\033[0m to ./$TEST_SRC_FOLDER/, compile, and move the .class file to ./$TEST_FOLDER/"
		continue
	fi

	# remove ".class" suffix from end of filename
	test_name=${test_file::-6}
	echo -e "------ \033[33mRunning $test_name\033[0m ---------"

	# remove old output
	rm -f ./$TEST_OUT_FOLDER/$test_name.java.out ./$TEST_OUT_FOLDER/$test_name.snap-jvm.out
	cd ./$TEST_FOLDER/

	# generated expected output
	java $test_name > ../$TEST_OUT_FOLDER/$test_name.java.out

	# generate snap-jvm output, printing when an opcode is unimplemented
	# NOTE: `snap-jvm` expects "filename" instead of "filename.class"
	../snap-jvm -t $test_name 1> ../$TEST_OUT_FOLDER/$test_name.snap-jvm.out

	# compare output, then make decision based on exit status of `diff`
	cd ../$TEST_OUT_FOLDER/
	diff ./$test_name.java.out ./$test_name.snap-jvm.out
	if [ $? -eq 0 ] ; then
		echo -e "\033[33m***** Test \033[35m$test_name \033[32mPASSED\033[0m"
		passed=`cat ./$TESTS_PASSED`
		passed=$((passed + 1))
		echo $passed > ./$TESTS_PASSED
	else
		echo -e "\033[33m***** Test \033[35m$test_name \033[31mFAILED\033[0m"
		failed=`cat ./$TESTS_FAILED`
		failed=$((failed + 1))
		echo $failed > ./$TESTS_FAILED
	fi
)
done

echo "`cat ./$TEST_OUT_FOLDER/$TESTS_PASSED` tests passed"
echo "`cat ./$TEST_OUT_FOLDER/$TESTS_FAILED` tests failed"
rm ./$TEST_OUT_FOLDER/*
rm ./$TEST_FOLDER/*

