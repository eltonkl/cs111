#!/bin/bash

#a simple function that will print out the appropriate prefix in the message after each test case has been executed
#the input argument is the value that the previous execution should have ended up with based on the test case
function testrun
{
    result=$?;
    if [ $result != $1 ]; then
        echo "TEST FAILED: ";
        echo $2;
    else
        echo "TEST SUCCEEDED: ";
        echo $2;
    fi
}

#create simpler associations to the test files for the purposes of testing. 
t1=testfiles/foo
t2=testfiles/bar
t3=testfiles/errors

#TESTING
#The general format of the segment is to run a possible simpsh use and then examine the output to see if it appears to be the proper output

#File opening
./simpsh --rdonly nosuchfile
testrun 1 "Report error if no file is available for reading only"

./simpsh --wronly nosuchfile
testrun 1 "Report error if no file is available for writing only"

./simpsh --rdonly $t1
testrun 0 "Test case where the file exists for reading only and opens correctly"

./simpsh --wronly $t1
testrun 0 "Test case where the file exists for writing only and opens correctly"

#Command call validation
./simpsh --command 1 2 3 echo $t1
testrun 1 "Report error if command attempts to use an out of range file descriptor"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 1 2 3
testrun 1 "Report error if command uses the wrong number of options"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
testrun 0 "Test case where we open a file for reading, a file for writing, and a file for error writing and execute the cat command."

#Inappropriate action catches
./simpsh --rdonly $t1 --rdonly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
testrun 1 "Report error if command attempts to write to a read only file"

./simpsh --wronly $t1 --rdonly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
testrun 1 "Report error if command attempts to read from a write only file"

#Command warnings NOTE: not sure exactly what the exit status of these commands should be. Please advise.
./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 1 0 2 cat $t1 $t2
testrun 0 "Report warning if trying to use rdonly file in stdout and trying to use wronly file in stdin, without interuppting execution"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 2 1 0 cat $t1 $t2
testrun 0 "Report warning if trying to use rdonly file in stderr, without interupting execution"

#Continue execution even after a failed command
./simpsh --verbose --wronly $t1 --command 1 2 3 echo foo --command 0 0 0 echo foo
testrun 0 "Continue execution even after a failed command"
