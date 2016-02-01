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
./simpsh --creat --wronly flagtest
testrun 0 "Test the creat flag for correctness. This also tests flagging in general."
echo "-------------------------------------------"

./simpsh --rdonly nosuchfile
testrun 1 "Report error if no file is available for reading only"
echo "-------------------------------------------"

./simpsh --wronly nosuchfile
testrun 1 "Report error if no file is available for writing only"
echo "-------------------------------------------"

./simpsh --rdonly $t1
testrun 0 "Test case where the file exists for reading only and opens correctly"
echo "-------------------------------------------"

./simpsh --wronly $t1
testrun 0 "Test case where the file exists for writing only and opens correctly"
echo "-------------------------------------------"

#Command call validation
./simpsh --command 1 2 3 echo $t1
testrun 1 "Report error if command attempts to use an out of range file descriptor"
echo "-------------------------------------------"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 1 2 3
testrun 1 "Report error if command uses the wrong number of options"
echo "-------------------------------------------"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
testrun 0 "Test case where we open a file for reading, a file for writing, and a file for error writing and execute the cat command."
echo "-------------------------------------------"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 1 0 2 cat $t1 $t2
testrun 0 "Report warning if trying to use rdonly file in stdout and trying to use wronly file in stdin, without interupting execution"
echo "-------------------------------------------"

./simpsh --rdonly $t1 --wronly $t2 --wronly $t3 --command 2 1 0 cat $t1 $t2
testrun 0 "Report warning if trying to use rdonly file in stderr, without interupting execution"
echo "-------------------------------------------"

#Continue execution even after a failed command
./simpsh --verbose --wronly $t1 --command 1 2 3 echo foo --command 0 0 0 echo foo
testrun 1 "Continue execution even after a failed command"
echo "-------------------------------------------"

#Close Testing
./simpsh --wronly $t1 --close breakthis
testrun 1 "Give an error if the argument is not a number."
echo "-------------------------------------------"

./simpsh --wronly $t1 --rdonly $t2 --close 0
testrun 0 "Properly close an opened file."
echo "-------------------------------------------"

./simpsh --wronly $t1 --close 1
testrun 1 "Fail to close nonexistent file desc."
echo "-------------------------------------------"

./simpsh --pipe --close 1 --close 0
testrun 0 "Close both ends of a pipe."
echo "-------------------------------------------"

#Abort Testing
#This works.
#./simpsh --abort
#if [ $? -eq 139 ]; then
#    echo "TEST SUCCEEDED:"
#    echo "Abort successfully SEGFAULTS"
#fi

#default/ignore/catch testing
./simpsh --ignore 11 --abort  --rdonly $t1 --wronly $t2 --wronly $t3 --command 0 1 2 cat $t1 $t2
testrun 0 "Ignore the segfault and then perform the command properly."
echo "-------------------------------------------"

./simpsh --catch 11 --abort 
echo $?
echo "This value should be 11."
echo "-------------------------------------------"

./simpsh --ignore 11 --catch 11 --default 11 --abort
echo "This should SEGFAULT without catching anything."
echo "-------------------------------------------"

#pause testing
./simpsh --pause --wronly $t1 --close 0 &
kill -SIGCONT $!
testrun 0 "Successful open and close after resuming."
echo "simpsh was successfully paused and resumed."
echo "-------------------------------------------"

#wait design problem testing

./simpsh --pipe --command 0 1 1 ls --wait 0
testrun 0 "Successful wait with an argument."
echo "-------------------------------------------"

./simpsh --pipe --command 0 1 1 ls --wait 1
testrun 1 "Realize that wait has an argument, and that it's invalid."
echo "-------------------------------------------"

./simpsh --rdonly testfiles/foo --creat --wronly err --pipe --pipe --pipe --pipe --pipe --pipe --command 0 3 1 cat /dev/urandom --command 2 5 1 tr -dc 'a-zA-Z0-9' --command 4 7 1 head -c 100M --command 6 1 1 grep 'James' --command 0 9 1 cat /dev/urandom --command 8 11 1 tr -dc 'a-zA-Z0-9' --command 10 13 1 head -c 10M --command 12 1 1 grep 'James' --wait 6
echo "This should take <12 seconds."
echo "Note that if we perform a normal wait the 100M command would take around a minute, thus confirming that this works."
echo "-------------------------------------------"


exit
