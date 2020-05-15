REM THIS IS WORK IN PROGRESS AS THE OLD GLUT PROJECT IS NOW DEAD - HAVE TO CONVERT TO THE NEW GLUT LIBRARY FREEGLUT

REM Install Cygwin from here: http://www.cygwin.com/    (only 2 optional modules you need to select are: g++ and mingw64-x86_64-freeglut)
REM Add C:\cygwin\bin to windows PATH environment variable
REM you should be able to run g++ from cmd now

g++ -o ./dist_mac/RetroPod ./src/surface.cpp ./src/shadow.cpp ./src/Node.cpp ./src/Model.cpp ./src/Hovercraft.cpp ./src/Checkpoint.cpp ./src/Main.cpp -lglu32 -lopengl32 -D GL_SILENCE_DEPRECATION -I. -L.
