#!/bin/bash
# 
# File:   mac_compile.bash
# Author: john
#
# Created on 14 May 2020, 20:08:10
#

c++ -o ./dist_mac/RetroPod ./src/surface.cpp ./src/shadow.cpp ./src/Node.cpp ./src/Model.cpp ./src/Hovercraft.cpp ./src/Checkpoint.cpp ./src/Main.cpp -L/System/Library/Frameworks -framework GLUT -framework OpenGL -D GL_SILENCE_DEPRECATION -framework Carbon