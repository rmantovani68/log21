#!/bin/bash

if [ -f ".PID" ]
then
	echo "PID presente. Non faccio nulla"
	exit 1
else
	echo "1" > .PID
fi

ls -lrt *

for i in {1..30}
do
	sleep 1
	echo $i sec
done

rm .PID

exit 0
