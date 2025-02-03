@echo off

cmake -B build -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release
cmake --build build --target all -j16

pause
