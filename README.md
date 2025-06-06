# N-body Pendulum
https://github.com/user-attachments/assets/ada7c15c-5e7a-439c-9e82-7960a29c1e3d
# How to Build?
`git submodule init`  
`git submodule update`  
`cmake -B build`  
`cmake --build build --target all`  
# How to use?
In build folder, run `Pendulum` executable. After that, pendulum from `build/examples/input.txt` is loaded.  
Structure of input.txt file:  
```
DeltaTime multiplayer
Camera coordinates (x, y, z) 
Camera angle (2 values)
Number of Pendulums
Number of moving nodes for each pendulum
For each pendulum, for each moving node:
* Initial angle
* Initial angular velocity
* Node mass
* Length of node's line
```
`WASD` can be use to move camera  
arrows can be used to rotate camera  
`R` reloades pendulum from file  
`T` starts/stops simulation
# Literature
https://www.researchgate.net/publication/336868500_Equations_of_Motion_Formulation_of_a_Pendulum_Containing_N-point_Masses
