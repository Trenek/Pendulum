@echo off

cmake -B build -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Debug
cmake --build build --target all -j1

pause
