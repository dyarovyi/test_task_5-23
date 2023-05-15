# CPU Stat app

This is a program to check the CPU usage percentage on Linux machines using the information from /proc/stat. 

## Important:
This software was written on MacOS. Since there is no such thing as '/proc/stat' file, the program runs using a mock file 'proc/stat'. The program also alternates data in this file to imitate the behaviour on Linux.
On Mac the program can be used only for demonstration. Stable work on Linux is not guaranteed.

## Installation

Run following command to compile a program file:
```
cmake CMakeLists.txt 
```

Run following command to run a program:
```
./cpu_usage_app
```

Run following command to compile a test file:
```
make test
```

Run following command to run tests:
```
./cpu_usage_app
```

Made by dyarovyi
