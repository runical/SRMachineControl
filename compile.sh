#!/bin/sh

g++ LibraryTest.cpp SRM.cpp Encoder.cpp -o Test.o

chmod +x Test.o

./Test.o
