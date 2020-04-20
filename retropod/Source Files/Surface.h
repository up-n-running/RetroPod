#include "physicsmaths.h"
#include <GL/glut.h>

/***************************************************************************\
*A surface is simply stored as a 2-dimensional array called pointY (actually, an array of arrays of floats).
*The 2-d array defines a grid of point heights which collectively define the surface.
*Imagine you are stood at the origin of our co-ordinate system, the negative z axis pointing forwards in front of you,
*the positive Y axis pointing vertically upwards. In this co-ordinate system, the positive X axis will be pointing to 
*your right. Now imagine a surface made up of a grid of 19 columns and 29 rows. The columns are paralell to the Z axis
*and the rows are paralell to your X axis. 
*pointY[0][0]   defines the height (y value) at the corner point x=0, z=0.
*pointY[0][30]  defines the height at the corner point x=0 z=-29.
*pointY[20][0]  defines the height at the corner point x=19 z=0.
*pointY[20][30] defines the height at the corner point x=19 z=-29.
*For instance the call 'getHeightAt(10.0, -15.0)' will give you the height at the centre of the surface;
*P.S. mapping the row number onto the NEGATIVE Z axis was a rubbish design decision, but there you go.
*
*Each square of the grid is called a segment (e.g. there are rows*cols number of segments), but since not all four
*corners of a segment are guaranteed to lie in the same plane in 3D space, we must draw each segment as 2 triangular polygons.
*As viewed from above, each polygon is a right angled triangle, and the line of bisection of each segment cuts
*through opposite corners making a line paralell to the line z=-x (as viewed from above).
*Within each segment, the 2 triangles are referred to as top left and bottom right.
*Top Left and Bottom Right referr to the respective positions when viewed from a camera positioned
*directly above the surface, the bottom of the camera pointing down the positive Z axis, and the left of the 
*camera pointing down the negative x axis.
*
*Often in the code, we refer to a surface segment using its column number and row number. Here we count the
*leftmost column as column 0, and similarly, we count the bottom most row as row number 0 (when viewed
*from above with the camera positioned as in the above paragraph).
\***************************************************************************/

class Surface { ///sort out what's public and what's not for other .h files + work out what's const

private:
	float **pointY;    //the 2-d array defining a grid of heights.
	int noOfXPoints;   //the number of columns in the grid plus one.
	int noOfZPoints;   //the number of rows in the grid plus one.

  GLuint displayListNo;  //the number of the display list for drawing the surface

public:

	Surface(void);
	Surface(const int listNo,const int xPoints,const int zPoints);
  Surface(const int listNo, FILE** filePointerPointer);
	~Surface(void);

  inline float getXLength(void);
  inline float getZLength(void);
  inline int   getNoOfCols(void);
  inline int   getNoOfRows(void);

  float getHeightAt(float xPos, float zPos);
  inline float getHeightAtIntUnsafe(int col, int row);

  void createDisplayList(GLint listNo);
  void draw(GLuint listNo);

  void save(char* fileName);

  void fetchPolygonNo(int *col, int *row, bool *topLeft, float x, float z);

private: 

  void flatten(void);
  void wobblify(float wobbliness, float height);
  void halfPipeify(float height);

};

///sort this out.
/******************************************\
*This function is an unintelligent version of 'getHeightAt'. it is simply used to
*access elements of the 2d pointY array. because speed is of the essence here, it's inlined
*and IT HAS NO SANITY CHECKING ON THE PARAMETERS and theferore could easily crash is abused.
\******************************************/
inline float Surface::getHeightAtIntUnsafe(int col, int row) {
  return pointY[col][row];
}