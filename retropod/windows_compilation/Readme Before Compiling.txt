REM Follow these instructions to install MinGW and freeglut:
REM https://www.transmissionzero.co.uk/computing/using-glut-with-mingw/
REM Then open the cmd command prompt, cd to the batch script and run it
REM Or if you're feeling confident just use my bulletpoint installation notes here:

REM -- Quick Instructions --

REM Install MinGW from here to default suggested directory: https://osdn.net/projects/mingw/downloads/68260/mingw-get-setup.exe/
REM The only optional component you need is: mingw-gcc-c++-bin
REM Add C:\MinGW\bin to windows $PATH Environment Variable

REM Get the latest freeglut here: http://freeglut.sourceforge.net/index.php#download
REM If necessary install 7-Zip to unzip the .tar.gz file: https://www.7-zip.org/
REM Create a freeglut folder in a location readble by all users, eg:
REM C:\Program Files\Common Files\MinGW\freeglut\
REM Copy the “lib\” and “include\” folders from the freeglut archive to that location
REM -------------------------------------- OR -------------------------------------
REM Just copy the MinGW directory to from the windows_compiliation directory in this repository to
REM C:\Program Files\Common Files

REM: Then open the cmd command prompt, cd to the batch script and run it