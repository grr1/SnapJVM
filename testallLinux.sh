#!/bin/bash

export CLASSFILES="Hello Loop Compare"
export EXPERIMENTALFILES="Add Increment staticTest"

echo ========== TESTALL SNAPJVM =============

echo 0 > test/tests_passed
echo 0 > test/tests_failed

for test in $CLASSFILES; do
(	
	echo ------ Running $test --------- 
	cd test;
	rm -f $test.java.out $test.snap-jvm.out
	rm -f $test.class
	javac $test.java
        java $test > $test.java.out
	../snap-jvm $test  > $test.snap-jvm.out
	diff $test.java.out $test.snap-jvm.out
	if [ $? -eq 0 ] 
        then
		echo ">>>>> Test " $t passed;
		tests_passed=`cat tests_passed`
		tests_passed=`expr $tests_passed + 1`
		echo $tests_passed > tests_passed
	else
		echo "***** Test " $t failed;
		tests_failed=`cat tests_failed`
		tests_failed=`expr $tests_failed + 1`
		echo $tests_failed > tests_failed
	fi 
)
done; 

echo
echo `cat test/tests_passed` tests passed
echo `cat test/tests_failed` tests failed
echo
