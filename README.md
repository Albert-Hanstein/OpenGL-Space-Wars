# OpenGL-Space-Wars
A 'Space Wars' themed virtual world, created using OpenGL in C++

How to operate the program:
- UP 		- Accelerate front
- DOWN 		- Decelerate until stopping
- LEFT		- Turn left
- RIGHT		- Turn right
- Page Up		- Pitch view upwards
- Page Down 	- Pitch view downwards
- <ESC>, Q, q 	- Exit the program (all modes)
- P, p		- Move to predefined location where screenshot was taken
- T, t		- Start Tour mode
- T (shift + t)	- When in Tour mode, pause the tour
- E, e		- Exit tour mode and go back to Demonstration mode
- Mouse movements and clicks are not captured as input in this program.

List of files:
- ame_nebula 	- Folder containing texture for skybox
- include		- Folder containing header files such as stb_image.h
- textures	- Folder containing textures for spaceships
- model_loading	- The vs and fs files are the shaders for the spaceships
- Cone.make	- The make file on which 'make' works. This is the file which needs to be modified to include '-lassimp'.
- Aircraft_obj	- The .obj and .mtl files contain information on the spaceship object
- importObj.h	- My own header file handling instructions for importing the spaceship objects
- premake4.lua	- The .lua file on which 'premake4 gmake' works
- Screenshot.PNG	- A screenshot of the program
- shaderClass.h	- My own header file for setting up shaders
- skybox		- The .vert and .frag files are the shaders for the skybox
- tutorial_cone.cpp - The .cpp file for my code
- tutorial3	- The .vert and .frag files are the shaders for the planets

How to build the program:
- Assimp is used to import object, so if generating new makefile, please include -lassimp in the make file.
- This code was written starting from the cone demo, so it is built using 'premake4 gmake' and 'make'.

How program works:
- The program takes some time to load the textures upon running, so the program window may appear white at the beginning. That is perfectly normal, and to attenuate the effect of this delay, the prompt gives a foreword for the scene to keep the user entertained while waiting.
- The program contains two moving spaceships and several stationary planets textured by Perlin noise, in front of an an outer space background.

Credits:
Spaceship object downloaded from https://free3d.com/3d-model/e-45-aircraft-71823.html
