#!/bin/bash

for (( var = 0 ; var < 17 ; var++ )) 
do
	gpio write $var 0
done
