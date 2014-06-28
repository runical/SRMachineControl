#!/bin/sh

g++ ExecutingScript.cpp SRM.cpp Encoder.cpp -o Test.o

./Test.o > log.txt
