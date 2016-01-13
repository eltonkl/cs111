#!/bin/bash

#a simple function that will print out the appropriate prefix in the message after each test case has been executed
#the input argument is the value that the previous execution should have ended up with based on the test case
function test $arg1
{
    result=$?;
    if [ $result -neq $1 ]; then
        echo "TEST FAILED: ";
        echo;
    else
        echo "TEST SUCCEEDED: ";
        echo;
    fi
}

#create simpler associations to the test files for the purposes of testing. 
t1=/testfiles/foo
t2=/testfiles/bar
errors=/testfiles/errors

#TESTING
#The general format of the segment is to run a possible simpsh use and then examine the output to see if it appears to be the proper output

./simpsh --rdonly nosuchfile
test(1) "Report error if no file is available for reading only"

./simpsh --wronly nosuchfile
test(1) "Report error if no file is available for writing only"

./simpsh --rdonly $t1
test(0) "Test case where the file exists for reading only and opens correctly"

./simpsh --wronly $t1
test(0) "Test case where the file exists for writing only and opens correctly"

./simpsh --command 1 2 3 echo $t1
test(1) "Report error if command attempts to use an out of range file descriptor"

./simpsh --command 1 2 3
test(1) "Report error if command uses the wrong number of options"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
test(0) "Test case where we open a file for reading, a file for writing, and a file for error writing and execute the cat command."

./simpsh --rdonly $t1 --rdonly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
test(1) "Report error if command attempts to write to a read only file"

./simpsh --wronly $t1 --rdonly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
test(1) "Report error if command attempts to read from a write only file"



