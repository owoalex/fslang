#/bin/bash
g++ -o main.o main.cpp
./main.o -o exampleProgram/bin/main.o exampleProgram/src/main.fs
cat exampleProgram/src/main.fs.cpp
