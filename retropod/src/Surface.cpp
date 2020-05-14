#ifdef __APPLE__
#include <GLUT/glut.h>
#include <stdlib.h>  // exit()
#include <unistd.h>  // close(), write()
#else
#include <windows.h>
#include <GL/glut.h>
#endif


#include <stdio.h>
#include <math.h>

#include "Surface.h"

#define PI 3.1415926535897

/***********************\
*NOTE: Check the .h file for more comments about the class, it's member variables and notations used below.
\***********************/

//---------------------------------------CONSTRUCTORS / DESTRUCTORS---------------------------------------



/***************************\
*Simple default constructor, initialises all
*the member variables with sensible values to create a small, flat surface
*then generates the openGL display list to draw this surface.
*NOTE: there should be no display list number 1 already defined.
\***************************/
Surface::Surface(void) {

  //create small 2d array
	pointY = (float**)new float[20][20];
	noOfXPoints=20;	noOfZPoints=20;

  //set all heights to 0
	flatten();

  //setup display list
	displayListNo=(GLuint)1;
	createDisplayList(displayListNo);
}



/***************************\
*Simple constructor, usefull for debugging. generates a flat surface
*with (xPoints-1) columns and (zPoints-1) rows
*then generates the openGL display list do draw this surface.
*listNo defines the display list number to use, and it should not already exist
\***************************/
Surface::Surface(int listNo, int xPoints, int zPoints) {

	//sanity check paramaters
	if (listNo<1) 
		listNo=1;
	if (xPoints<2) 
		xPoints=2;
	if (zPoints<2) 
		zPoints=2;

	//create the 2-d array
	pointY = new float*[xPoints];
	for (int i=0; i<xPoints; i++) {
		pointY[i] = new float[zPoints];
	}
	noOfXPoints=xPoints;
	noOfZPoints=zPoints;

  //set all heights to 0
	flatten();

  //setup display list
	displayListNo=(GLuint)listNo;
	createDisplayList(displayListNo);

}



/***************************\
*This constructor reads all the values for the member variables from a file.
*The file pointer must be pointing to the first line of the surface defining
*part of the text file.
*listNo defines the display list number to use, and it should not already exist
\***************************/
Surface::Surface(const int listNo, FILE** filePointerPointer) {
	
  //setup temp string
	char tempString[256];

	//load number of cols, and no of rows respectively
	int xPoints=atoi(fgets(tempString, 50, *filePointerPointer));
	int zPoints=atoi(fgets(tempString, 50, *filePointerPointer));

	//create a 2-d array
	pointY = new float*[xPoints];
	for (int i=0; i<xPoints; i++) {
		pointY[i] = new float[zPoints];
	}

	//initialise size variables
	noOfXPoints=xPoints;
	noOfZPoints=zPoints;


	//load all point heights in order
	for (int x=0; x<noOfXPoints; x++) {
		for (int z=0; z<noOfZPoints; z++) {
			pointY[x][z]=atof(fgets(tempString, 50, *filePointerPointer));
		}
	}

  //setup display list
	displayListNo=(GLuint)listNo;
	createDisplayList(displayListNo);
}



/***********\
*Destructor
\***********/
Surface::~Surface(void) {
	//delete a 2d array
	for (int i=0; i<noOfXPoints; i++) {
		delete [] pointY[i];
	}
	delete [] pointY;

}

//-----------------------------------OTHER PUBLIC FUNCTIONS-------------------------------------------



/***********************\
*returns the width of the surface (distance across X axis) (inlined)
\***********************/
float Surface::getXLength() {
  return (float)(noOfXPoints-1);
}



/***********************\
*returns the length of the surface (distance across Z axis) (inlined)
\***********************/
float Surface::getZLength() {
  return (float)(noOfZPoints-1);
} 



/***********************\
*returns the number of columns (inlined)
\***********************/
int Surface::getNoOfCols() {
	return noOfXPoints-1;
}



/***********************\
*returns the number of rows (inlined)
\***********************/
int Surface::getNoOfRows() {
	return noOfZPoints-1;
}




/************************************\
*This function calculates the height (y value) of the surface at the co-ordinates x=xPos, z=zPos (NOTE: only non positive z values lie on the surface)
*If xPos and zPos are whole numbers then this is easy, otherwise we need to work out which polygon we are on, then find the height
*on the plane defined by the polygon at the appropriate x and z values. 
\************************************/  
float Surface::getHeightAt(float xPos, float zPos) {

  //these will store surface segment containing the co-ordinates x=xPos, z=zPos.
  int colNo;
  int rowNo;

  //this will store whether we are on the top left polygon or bottom right polygon.
  bool topLeft;

  //force the position we are looking at to be inside the boundaries of the surface
  if (xPos<0.0f) 
    xPos=0.0f;
  if (xPos>getXLength()) 
    xPos=getXLength(); 
  if (-zPos<0.0f)
    zPos=0.0f; 
  if (-zPos>getZLength()) 
    zPos=-getZLength(); 

  //fetch values into the three variables defining which polygon we are on.
  fetchPolygonNo(&colNo, &rowNo, &topLeft, xPos, zPos);

  //*****now find the height at the point*****.

  //start with the height of the bottom left corner of the segment
  float height=pointY[colNo][rowNo];

  //now adjust this height, depending on which polygon we are on, and where abouts we are within the polygon

  //in the code below, the 4 innermost if statements are to stop invalid array element accesses.
  //ie trying to find the height of a point which is not on the surface! Where one of these if
  //statements stops a calculation, i can guarantee that that calculation would not have changed the height anyway.
  if (topLeft) {

    if (rowNo!=getNoOfRows())  //to make sure we don't try to access values that are too high.
      height += ( -zPos -floor(-zPos) )   *   (pointY[colNo  ][rowNo+1]-pointY[colNo  ][rowNo  ]);
    if (colNo!=getNoOfCols() && rowNo!=getNoOfRows()) //to make sure we don't try to access values that are too high.
      height += (  xPos -floor( xPos) )   *   (pointY[colNo+1][rowNo+1]-pointY[colNo  ][rowNo+1]);

  }
  else {

    if (colNo!=getNoOfCols()) //to make sure we don't try to access values that are too high.
      height += (  xPos -floor( xPos) )   *   (pointY[colNo+1][rowNo  ]-pointY[colNo  ][rowNo  ]);
    if (colNo!=getNoOfCols() && rowNo!=getNoOfRows()) //to make sure we don't try to access values that are too high.
      height += ( -zPos -floor(-zPos) )   *   (pointY[colNo+1][rowNo+1]-pointY[colNo+1][rowNo  ]);

  }

  //now return the height
  return height;

}







/****************************************\
*This draws the surface display list at the current origin. If the display list has somehow
*not been defined, it prints a message to standard output instead.
\****************************************/
void Surface::draw(GLuint listNo) {

  if (glIsList(listNo) ) {
    glCallList(listNo);
  }
  else {
	  printf("%s (%i) %s\n", "The Display List",listNo,"For The Surface Has Not Been Defined.");
  }
}



/*****************************************\
*This Creates a display list that draws the surface whose origin matches the current openGL origin.
*'listNo' defines the display list number, this display list must not be already defined.
\*****************************************/
void Surface::createDisplayList(GLint listNo) {

  glNewList(listNo, GL_COMPILE);

    //setup the material properties of the surface.
    GLfloat surfSpecular[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat surfShinyness[] = {150.0f};
    GLfloat surfDiffuse[] = {0.1f, 0.8f, 0.1f, 1.0f};
    GLfloat surfAmbience[] = {0.1f, 0.8f, 0.1f, 1.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, surfSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, surfShinyness);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, surfDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, surfAmbience);

    //as we cycle through each surface segment, these will store the co-ordinates of the bottom
    //left corner of the current segment.
  	float refPointX;
  	float refPointZ;

    //every time we create a new polygon, this will store the normal vector for all three vertices of the polygon.
    Vector normalVector;

    //loop through each surface segment in turn.
    for (int rowNum=0; rowNum<=noOfZPoints-2; rowNum++) {
  		for (int colNum=0; colNum<=noOfXPoints-2; colNum++) {

        //find the co-ordinates of the bottom left corner of the current surface segment
   			refPointX=(float)colNum;
  			refPointZ=(float)(-rowNum);

        //find the normal vector for the TOP LEFT polygon
  			normalVector.x=pointY[colNum  ][rowNum+1]-pointY[colNum+1][rowNum+1];
        normalVector.y=1.0f;
  			normalVector.z=pointY[colNum  ][rowNum+1]-pointY[colNum  ][rowNum  ];

        //convert normal vector to the same vector, but of unit length
  			normalVector.Normalize();

        //now create the TOP LEFT polygon
  			glBegin(GL_POLYGON);
  			  glNormal3f(normalVector.x, normalVector.y, normalVector.z); 
  			  glVertex3f(refPointX     , pointY[colNum  ][rowNum  ], refPointZ     );
  			  glVertex3f(refPointX     , pointY[colNum  ][rowNum+1], refPointZ-1.0f);
  			  glVertex3f(refPointX+1.0f, pointY[colNum+1][rowNum+1], refPointZ-1.0f);
  			glEnd();

        //find the normal vector for the BOTTOM RIGHT polygon
  			normalVector.x=pointY[colNum  ][rowNum  ]-pointY[colNum+1][rowNum  ];
        normalVector.y=1.0f;
  			normalVector.z=pointY[colNum+1][rowNum+1]-pointY[colNum+1][rowNum  ];

        //convert normal vector to the same vector, but of unit length
  			normalVector.Normalize();

        //now create the BOTTOM RIGHT polygon
  			glBegin(GL_POLYGON);
  			  glNormal3f(normalVector.x, normalVector.y, normalVector.z); 
  			  glVertex3f(refPointX     , pointY[colNum  ][rowNum  ], refPointZ     );
  			  glVertex3f(refPointX+1.0f, pointY[colNum+1][rowNum  ], refPointZ     );
  			  glVertex3f(refPointX+1.0f, pointY[colNum+1][rowNum+1], refPointZ-1.0f);
  			glEnd();
  
  		}
  	}

  glEndList();

}



/***********************************\
*This function creates or overides the file 'fileName' in the current directory
*with a file defining the surface - suitable for loading in the file reading constructor above.
\***********************************/
void Surface::save(char* fileName) {

	//setup temp string
	char tempString[256];

	//setup file stuff
	FILE *filePointer=0;
	filePointer=fopen(fileName, "w");

  //check file thing worked
	if (!filePointer) {
		printf("Sorry, Could Not Create/Open the File %s in the Current Directory\n", fileName);
		exit (0);
	}

	//save number of cols, and no of rows respectively
	sprintf(tempString, "%i\n", noOfXPoints);
	fputs(tempString, filePointer);
	sprintf(tempString, "%i\n", noOfZPoints);
	fputs(tempString, filePointer);

	//save all point heights in order
	for (int x=0; x<noOfXPoints; x++) {
		for (int z=0; z<noOfZPoints; z++) {
			sprintf(tempString, "%f\n", pointY[x][z]);
			fputs(tempString, filePointer);
		}
	}

	//close file
	fclose(filePointer);
}



/***********************************************\
*given the x and z co-ordinates of a point in space, this function returns the rownum
*and colnum of the co-ordinate's enclosing square, the boolean is set to
*true if the point is in the top left half of the square, and false otherwise.
\***********************************************/
void Surface::fetchPolygonNo(int *col, int *row, bool *topLeft, float x, float z) {
  *col=(int)floor( x);
  *row=(int)floor(-z);

  *topLeft=(  ( -z-floor(-z) )  >  ( x-floor(x) )  );
}

//--------------------------------------PRIVATE FUNCTIONS-------------------------------------------



/**************************\
*This function flattens the surface by setting all heights to zero.
\**************************/
void Surface::flatten(void) {
	for (int x=0; x<noOfXPoints; x++) {
		for (int z=0; z<noOfZPoints; z++) {
			pointY[x][z]=(float)0.0;
		}
	}
}



/**********************************\
*This function is usefull for debugging.
*It adds a wavyness to the surface. Each wave is paralell to the X axis.
*The greater the value of 'wobbliness' the more waves there are (the shorter the wavelength).
*'height' sets the amplitude of the waves.
\**********************************/
void Surface::wobblify(float wobbliness, float height) {
	float rowHeight;
	for (int rowNum=0; rowNum<noOfZPoints; rowNum++) {
		rowHeight=cos(((float)rowNum)*wobbliness)*height;
		for (int colNum=0; colNum<noOfXPoints; colNum++) {
		  //printf("%f\n", pointZ[colNum, rowNum]);
			pointY[colNum][rowNum]+=rowHeight;
		}
	}
}



/*****************************\
*This function is usefull for debugging.
*It adds a half pipe type effect, whose contour lines are paralell to the X axis.
*'height' sets the height of the walls of the half pipe.
\*****************************/
void Surface::halfPipeify(float height) {
	float colHeight;
  float wobbliness=PI/(noOfXPoints);
	for (int colNum=0; colNum<noOfXPoints; colNum++) {
		colHeight=(1-sin(((float)colNum)*wobbliness))*height;
		for (int rowNum=0; rowNum<noOfZPoints; rowNum++) {
			pointY[colNum][rowNum]+=colHeight;
		}
	}
}
