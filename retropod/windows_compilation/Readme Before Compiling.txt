Follow these instructions to install MinGW and freeglut:
https://www.transmissionzero.co.uk/computing/using-glut-with-mingw/
Then open the cmd command prompt, cd to the batch script and run it

Or if you're feeling confident just use my bulletpoint installation notes here:


-- Quick Instructions --

Install MinGW from here to default suggested directory: https://osdn.net/projects/mingw/downloads/68260/mingw-get-setup.exe/
The only optional component you need is: mingw-gcc-c++-bin
Add C:\MinGW\bin to windows $PATH Environment Variable

Just copy the MinGW directory and its contents from the windows_compiliation directory in this repository to
C:\Program Files\Common Files

Then open the cmd command prompt, cd to the batch script and run it



--- Additional Step If you want to ensure you're using the latest version of freeglut instead ---
Get the latest freeglut here: http://freeglut.sourceforge.net/index.php#download
If necessary install 7-Zip to unzip the .tar.gz file: https://www.7-zip.org/
Create a freeglut folder in a location readble by all users, eg:
C:\Program Files\Common Files\MinGW\freeglut\
Copy the “lib\” and “include\” folders from the freeglut archive to that location

