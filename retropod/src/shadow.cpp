#ifdef __APPLE__
#include <stdlib.h>  // exit()
#include <unistd.h>  // close(), write()
#else
#endif

#include <stdio.h>
#include <math.h>

#include "Shadow.h"


/*******************************************************************\
*Shadow.cpp is not a class, but a singleton. 
*Only the function 'drawShadow' is intended for external use.
*All of the other functions are simply functions called by the 'drawShadow'
*function.
*A class Node is also defined here, it is only to be used by functions inside this file.
*
*You pass in to the function - a pointer to a surface and two arrays defining the shape and position
*of the shadow object to draw. The function draws the shadow of the object as if the light is shining
*vertically downwards from a point infinitely far away, and vertically above the surface. Note, 
*using this light source means that the height of the object we are shadowing does not affect
*the shadow itself. The object that's casting the shadow is called the shadow object.
*
*Although the shadow drawing problem looks simple it is actually a very complex aglorithm. Before
*describing what the algorithm does it is first necessary to explain a few conventions used 
*in this file only:
*
*EQUATION FOR A LINE:
*Usually, you express the equation defining a line as two constants 'M' and 'C' in the equation Y=MX+C.
*In the real world however this is bad because as the line tends to vertical, 'M' and 'C' tend to 
*+ or - infinity. I therefore define a line using three constants; 'M', 'C' and a boolean called 'lineNormal'.
*If a line is nearer horizontal than vertical, then 'lineNormal' is set to true, and 'M' and 'C' represent
*their respective values in the equation Y=MX+C.
*If, however a line is nearer vertical than horizontal, then 'lineNormal' is set to false, and
*'M' and 'C' represent their respective values in the equation X=MY+C.  (note how x and y have been swapped round).
*
*THE CO-ORDINATE SYSTEM:
*In the surface co-ordinates, the surface starts at the origin and lies alongside the positive x axis 
*and negative z axis. The Y axis represents vertically upwards, and therefore the height.
*As mentioned above, the height of the shadow object is unimportant, but it's position and size 
*along the X and negative Z axis are important. For the sake of simplicity all co-ordinates
*representing the shadow and shadow object's corners are represented as co-ordinates in terms of
*X and Y, not X and Z. Essentially, the shadow co-ordinate x=8, y=3 corresponds to the surface 
*co-ordinate x=8, z=-3. Likewise the equation of a line Y=MX+C actually equates to -Z=MX+C in
*surface co-ordinates. 
*
*
*HERES WHAT THE 'drawShadow' FUNCTION DOES.
*The comments in the file surface.h describe a view of the surface from above. Imagine this view using an
*orthogonal projection. This would give you an projection of the surface in just the x and (negative) z axis
*Now imagine looking at both the surface and the shadow object using this projection. The shadow object
*would cover up a section of the surface. the 'drawShadow' function re-draws that part of the surface in grey,
*but raised from the surface by a tiny ammount. effectively forming a grey blanket over the top of the
*appropriate part of the surface.
*'drawShadow' has three paramaters that define the shadow object; two arrays of equal length 
*(shadowObjectX and shadowObjectY), and an integer variable describing the length of the two arrays.
*Using the orthogonal, downwards projection mentioned above, the shadow object appears as a 2d
*shape obscuring the surface. The co-ordinates of this 2d shape should be defined the 2 arrays using
*shadow co-ordinates, and not surface co-ordinates (see section above). I.E:
* X=shadowObjectX[0], Y=shadowObjectY[0] is the 1st corner of the shape clockwise
* X=shadowObjectX[1], Y=shadowObjectY[1] is the 2nd corner of the shape clockwise
* X=shadowObjectX[2], Y=shadowObjectY[2] is the 3rd corner of the shape clockwise
* etc.
*
*
*HERE'S HOW THE 'drawShadow' WORKS.
*
*
\*******************************************************************/














static float* lineM;
static float* lineC;
static bool*  lineNormal;
  
int drawShadow(float* shadowObjectX, float* shadowObjectY, int shadowObjectSize, Surface *mySurface);
static void initShadowLines (float* linesX, float* linesY, int noOfLines);
static void finishShadowLines ();
static void lineCrossesEdge (int lineNo, float thisNodeX, float thisNodeY, float nextNodeX, float nextNodeY,
                               bool *doesItCross, float *crossX, float *crossY );
static bool isPointOnInsideOfLine (int lineNo, float l1X, float l1Y, float l2X, float l2Y, float pointX, float pointY);
static void findEquationForLine (float x1, float y1, float x2, float y2, float* m, float* c, bool *inversed);
static void initShadowLinkedList (int colNo, int rowNo, bool topLeft, Node **nThisPointer, Node **nLastPointer);
static void finishShadowLinkedList (Node *nThis, Node* nLast, Surface *mySurface, bool drawIt);



/*********************************************************\
* This function simply calculates the Y=MX+C for all lines that
* make up The Shadow Object. Is stores the
* Coefficients in the 2 global arrays, which it initialises at the same time.
* ///IT ONLY WORKS IF THE LINE IS NOT VERTICAL.
\*********************************************************/
static void initShadowLines(float* linesX, float* linesY, int noOfLines) {

    //create array of equations for the shadow object lines
    lineM = new float[noOfLines];
    lineC = new float[noOfLines];
    lineNormal = new bool[noOfLines];
    //next we populate these 2 arrays......

    //loop through every line of the shadow object and create an equation (Y='m'*X+'c') for each line.
    for (int lineNo=0; lineNo < noOfLines ; lineNo++) {

      //find the co-efficients of the current line in question, and set the global array elements to these coefficient values
      findEquationForLine(linesX[lineNo], linesY[lineNo], linesX[(lineNo+1)%noOfLines], linesY[(lineNo+1)%noOfLines], &lineM[lineNo], &lineC[lineNo], &lineNormal[lineNo]);

    }

}

static void finishShadowLines () {   //do i need a [] ?
    delete [] lineM;
    delete [] lineC;
    delete [] lineNormal;
}



//////////////////////////////////////////////////////////////////////////////



static void initShadowLinkedList (int colNo, int rowNo, bool topLeft, Node **nThisPointer, Node **nLastPointer) {
  //setup the circular linked list of size 3
  Node *n1, *n2, *n3;
       
  if (topLeft) {
    n1=new Node( (float)colNo    , (float)(rowNo+1) );
    n2=new Node( (float)(colNo+1), (float)(rowNo+1) );
    n3=new Node( (float)colNo    , (float)rowNo     );
  }
  else {
    n1=new Node( (float)(colNo+1), (float)rowNo     );
    n2=new Node( (float)colNo    , (float)rowNo     );
    n3=new Node( (float)(colNo+1), (float)(rowNo+1) );
  }
       
   n1->next=n2;
   n2->next=n3;
   n3->next=n1;
        
   *nThisPointer=n1;
   *nLastPointer=n3;
}

static void finishShadowLinkedList (Node *nThis, Node* nLast, Surface *mySurface, bool drawIt) {
  //*************DESTROY LINKED LIST********************
  
  Node *nTemp;

  nLast->next=NULL;



  ///glDisable( GL_LIGHTING );
  ///glLoadIdentity();
  ///glColor3f( 0.2f, 0.4f, 0.2f );

  if (drawIt) 
    glBegin(GL_POLYGON);

  while (nThis != NULL) {

    if (drawIt) 
      glVertex3f (nThis->x, mySurface->getHeightAt(nThis->x, -nThis->y)+0.05f, -nThis->y ); 

    nTemp=nThis->next;
    delete nThis;
    nThis=nTemp;
  }

  if (drawIt) 
    glEnd();


}


/********************************************\
* The Main Function
* The shadow shape should be convex for it to work properly, if not alter
* The main function a to allow the 'noOfLineCrosses' variable to be any of 0, 2, 4, 6, etc,
* and the isPointInsideOfLine function to work for convex shapes (assuming it doesn't already!!!)
\********************************************/

int drawShadow(float* shadowObjectX, float* shadowObjectY, int shadowObjectSize, Surface *mySurface) {

  //SETUP PRETEND PARAMATERS 
  //                         1      2      3      4   
//  float shadowObjectX[4]={  1.8f,  4.8f,  4.8f,  1.8f};  //note the hovercraft's Z values have already been negated and converted to Y values - i know this is crap.
//  float shadowObjectY[4]={  3.2f,  3.2f,  0.2f,  0.2f};
//  int shadowObjectSize=4;

  //CALL INITIALISATION FUNCTION TO CREATE THE ARRAYS OF COEFFICIENTS OF THE EQUATIONS DESCRIBING THE SHADOW OBJECT'S LINES
  initShadowLines(shadowObjectX, shadowObjectY, shadowObjectSize);

  //setup opengl stuff
  glDisable( GL_LIGHTING );
  glLoadIdentity();
  glColor3f( 0.2f, 0.4f, 0.2f );

  //declare some pointers to nodes of the linked list
  Node *nThis, *nLast, *nNew;

  //variables to define the range of surface segments we are going to analyse.
  int minCol=mySurface->getNoOfCols()-1; 
  int maxCol=0;
  int minRow=mySurface->getNoOfRows()-1;
  int maxRow=0;

  //temp veriables used below
  int tmpColNo, tmpRowNo;
  bool tmpUnused;

  //find the range of surface segments under the shadow object
  for (int i=0 ; i<shadowObjectSize ; i++) {

    //find row and column number of the point in question
    mySurface->fetchPolygonNo(&tmpColNo, &tmpRowNo, &tmpUnused,  shadowObjectX[i], -shadowObjectY[i]);

    //update surface segment range paramaters accordingly
    if (minCol>tmpColNo)
      minCol=tmpColNo;
    if (maxCol<tmpColNo)
      maxCol=tmpColNo;
    if (minRow>tmpRowNo)
      minRow=tmpRowNo;
    if (maxRow<tmpRowNo)
      maxRow=tmpRowNo;

  }

  //finally ensure our boundaries define an area fully within the surface.
	if (minCol<0)
		minCol=0;
	if (maxCol>mySurface->getNoOfCols()-1)
		maxCol=(mySurface->getNoOfCols())-1;
	if (minRow<0)
		minRow=0;
	if (maxRow>mySurface->getNoOfRows()-1)
		maxRow=(mySurface->getNoOfRows())-1;

  //now loop through all the surface segments in the range
  for (int row=minRow ; row <= maxRow ; row++){
    for (int col=minCol ; col <= maxCol ; col++) {
      for (int topLeft=0 ; topLeft < 2 ; topLeft++ ) {

        //***************INITIALISE LINKED LIST********************

        initShadowLinkedList(col, row, (bool)topLeft, &nThis, &nLast);
      
        //***************LOOP*****************
      
        //this variable is set to true once the algoritmn has realised that the shadow polygon is completely outside
        //the shadow shape. once it is set to true, the algorithm performs no more operations on the list. 
        bool listDiscarded = false;
      
        //these variables are used as we traverse the linked list to keep track of where the current shadow line crosses the shadow polygon.
        bool doesItCross=false;
        float crossX, crossY; crossX=crossY=0.0f;
        bool  areWeInside=false;
        bool firstTimeInWhile=true;
      
        for (int lineNo=0; (lineNo<shadowObjectSize && !listDiscarded) ; lineNo++) {
      
      		//printf("********Shadow line number: %i\n", lineNo); 
       
          //reset variables used in keeping track of the while loop status
          firstTimeInWhile=true;
          int noOfLineCrosses=0; //this records the number of times that the currelt line crosses the shadow polygon
      
          //check to see if we are starting on the inside or outside of the current line.
          areWeInside = isPointOnInsideOfLine (lineNo, shadowObjectX[lineNo], shadowObjectY[lineNo], shadowObjectX[(lineNo+1)%shadowObjectSize], shadowObjectY[(lineNo+1)%shadowObjectSize], nThis->x, nThis->y);
      
          //mark the first node, so we know when to stop
          nThis->firstNode=true;
      
          //LOOP ROUND THE SHADOW POLYGON ADDING NODES AT CROSSOVER POINTS, AND MARKING EACH NODE AS INSIDE OR OUTSIDE.
          while (!nThis->firstNode || firstTimeInWhile) { 
      
            //first we mark the current node with its inside/outside status.
            nThis->inside=areWeInside;
      
            //check if the current shadow line crosses the line between this node and the next node
            lineCrossesEdge (lineNo, nThis->x, nThis->y, nThis->next->x, nThis->next->y, &doesItCross, &crossX, &crossY );
      
            //debug about line crossing.
            int debug=doesItCross;
            //printf("this: x=%f, y=%f, next: x=%f, y=%f, X-over=%i: x=%f, y=%f\n",nThis->x, nThis->y, nThis->next->x, nThis->next->y, debug, crossX, crossY);
      
            //if there was a crossover, we move on, add an extra node to the linked list, and flip the 'are we inside/outside' flag.
            if (doesItCross) {
              //declare the fact that we've found a crossover
              noOfLineCrosses++;
              //create new node
              nNew=new Node(crossX, crossY);
              nNew->inside=true;
      			  //printf("  ******created new node at (%f, %f)\n", crossX, crossY); 
              //patch it into the linked list
      			  nNew->next=nThis->next;
      			  nThis->next=nNew;
              //move to next node in the original linked list
      			  nLast=nNew;
      			  nThis=nNew->next;
              //update the 'are we inside/outside' flag
              areWeInside=!areWeInside;
            }
            else {
      
              //simply move on to the next node.
      			  nLast=nThis;
      			  nThis=nThis->next;
      
            }
      
            //prepare for next loop round while
            firstTimeInWhile=false;
      
          } 
          //END OF THE FIRST LOOP ROUND THE SHADOW POLYGON.
      
          if (noOfLineCrosses!=0 && noOfLineCrosses!=2)
            printf("CRIMENY!!!, THERE WERE %i CROSSOVERS\n", noOfLineCrosses);
          //printf("START OF SECOND LOOP ROUND POLYGON\n");
      

          //press any key!!
          //char* hello=new char[5];
          //gets(hello);



          //NOW LOOP ROUND THE SHADOW POLYGON IRADICATING ANY OUTSIDE NODES. AS WE TRAVERSE THE LIST, WE
          //MARK EACH NODE AS VISITED, SO WE KNOW WHEN WE'VE DONE A FULL CIRCLE.
          while (!nThis->hasBeenVisited && !listDiscarded) { 
      
            //first we mark the current node so we know it's been visited
            nThis->hasBeenVisited=true;
      
            //debug about wherter it's marked as inside or outside
            int debug=nThis->inside;
            //printf("this: x=%f, y=%f, next: x=%f, y=%f, inside?=%i\n",nThis->x, nThis->y, nThis->next->x, nThis->next->y, debug);
      
            //if the current node is outside, delete it!
            if (!nThis->inside) {
      
              //delete this node and move to next one
              nLast->next=nThis->next;
              delete nThis;
              nThis=nLast->next;
      
              //if deleting that node has left us with a list of size two, the shadow must now be just a line,
              //and therefore have zero area, so we discard this list and the algoritmn will now undergo no more checks.
              if (Node::getNoOfNodes()<=2) {
                listDiscarded=true;
              }
      
              //debugging..
              //printf("  ******deleted this node, and moved onto next one. There are now %i nodes\n", Node::getNoOfNodes()); 
            }
            else {
      
              //simply move on to the next node.
      			  nLast=nThis;
      			  nThis=nThis->next;
       
            }
      
          }
          //END OF THE SECOND LOOP ROUND THE SHADOW POLYGON.
      
          //debug
          //printf("START OF THIRD LOOP ROUND POLYGON\n");


           //press any key!!
          //gets(hello);



          //NOW LOOP ROUND THE SHADOW POLYGON ONE LAST TIME 
          //WE MUST MARK EVERYTHING AS UNVISITED AGAIN, AND DELETE ANY RECORD OF WHICH WAS THE FIRST NODE, READY FOR NEXT TIME. 
          while (nThis->hasBeenVisited) { 
      
            //mark current node as unvisited
            nThis->hasBeenVisited=false;
            nThis->firstNode=false;
      
            //debug
            int debug=nThis->inside;
            //printf("this: x=%f, y=%f, next: x=%f, y=%f, inside?=%i\n",nThis->x, nThis->y, nThis->next->x, nThis->next->y, debug);
      
            //move to next node
            nThis=nThis->next;
      
          }
          //END OF THE THIRD LOOP ROUND THE SHADOW POLYGON.
      
          //press any key!!
          //gets(hello);
          //delete [] hello;

        }
        //END OF LOOP TRAVERSALS FOR ALL SHADOW LINES

        //***************DeStRoY LINKED LIST********************
        finishShadowLinkedList(nThis, nLast, mySurface, !listDiscarded);

      }
    }
  }

  //CALL UNINITIALISATION FUNCTION TO DELETE THE ARRAYS OF COEFFICIENTS OF THE EQUATIONS DESCRIBING THE SHADOW OBJECT'S LINES
  finishShadowLines();

  //unsetup opengl stuff
  glEnable( GL_LIGHTING );

  return 1;
}


/*********************************************************\
* This function finds the crossover point between the following
* two lines: the shadow object's line numbered 'lineNo' and the current side
* of our shadow polygon (defined by the 2 input co-ordinates). If this 
* crossover point is between the 2 paramater co-ordinates, then 'doesItCross' will
* be set to true and crossX and crossY will be set to these crossover 
* co-ordinates. Otherwise, doesItCross is set to zero.
* ///THIS MAY BE BOLOX: AT THE MOMENT IT ONLY WORKS FOR NON VERTICAL LINES, BOTH IN THE CALCULATION OF NODEM AND NODEC, AND IN THE 'DO THEY CROSS' ALGORITHM, AND IN THE 'IS THE CROSSOVER POINT IN THE RANGE' BIT
\*********************************************************/
static void lineCrossesEdge (int lineNo, float thisNodeX, float thisNodeY, float nextNodeX, float nextNodeY,
                             bool *doesItCross, float *crossX, float *crossY ) {

    //delcare variables to store m and c in equation of line (y=mx+c normal OR x=my+c inverse)
    //and also a boolean to store the equation type. The line in question is the current side of the shadow polygon
    float nodeM, nodeC;
    bool nodeNormal;

    //populate the above variables with the line definition
    findEquationForLine (thisNodeX, thisNodeY, nextNodeX, nextNodeY, &nodeM, &nodeC, &nodeNormal);

    //set output paramaters to default values
	  *doesItCross=false;
    *crossX=0.0f;
    *crossY=0.0f;

    //create a float which may to swap the values of crossX and crossY below.
    float temp;

    //check that lines are not paralell - if they are then they must not cross, and we need do nothing!!
    if ( (lineM[lineNo]!=nodeM) || (lineNormal[lineNo]!=nodeNormal) ) {  

      //if non-paralell, find crossover point between the two lines

      //check to see if both equations are represented in the same format.
      if ( nodeNormal == lineNormal[lineNo] ) {
    
        //below is the general solution to the crossover point between either 2 normal equations or 2 inverse equations
        *crossX = (nodeC-lineC[lineNo]) / (lineM[lineNo]-nodeM);
        *crossY = lineM[lineNo] * (*crossX) + lineC[lineNo];

        //if both equations were inverse, we need to swap crossX and crossY over
        //we do NOT do this this by swapping the pointers over, we must swap the actual values themselves.
        //this is because the pointers in question are local to this function only!
        if (!lineNormal[lineNo]) {
          temp = *crossX;
          *crossX=*crossY;
          *crossY=temp;
        }

      }
      else {

        //below is the general solution to the crossover point between 1 normal equation and 1 inverse equation.
        *crossX = (nodeM*lineC[lineNo] + nodeC) / (1 - lineM[lineNo]*nodeM);
        *crossY = lineM[lineNo] * (*crossX) + lineC[lineNo];

        //if the line eqn is inverse, and the node eqn normal, we need to swap crossX and crossY over
        //we do NOT do this this by swapping the pointers over, we must swap the actual values themselves.
        //this is because the pointers in question are local to this function only!
        if (!lineNormal[lineNo]) {
          temp = *crossX;
          *crossX=*crossY;
          *crossY=temp;
        }

      }

		  //check whether crossover is in the range. NOTE: if the line crosses through 'this' node exactly it counts as crossing the edge, if it crosses 'next' node exactly it does not count. This is so that the line does not count as crossing the shadow polygon twice for only one real cross
		  if ( (      nodeNormal  && ( (*crossX>=thisNodeX && *crossX<nextNodeX) || (*crossX>nextNodeX && *crossX<=thisNodeX) )   )
        || (    (!nodeNormal) && ( (*crossY>=thisNodeY && *crossY<nextNodeY) || (*crossY>nextNodeY && *crossY<=thisNodeY) )   )    ) {

			  //if so, set crossover flag to true
			  *doesItCross=true;

      }

	  }

}



/*************************************\
* This function accepts the current shadow line number, and 3 sets of co=ordinates:
* (l1X, l1Y) defines the first point on the line (the more anticlockwise of the two points on the shadow shape)
* (l2X, l2Y) defines the other point on the line (the more     clockwise of the two points on the shadow shape)
* (pointX, pointY) defines the point for which we are testing the insidedness.
\*************************************/
static bool isPointOnInsideOfLine (int lineNo, float l1X, float l1Y, float l2X, float l2Y, float pointX, float pointY) {
	//Is the start node on the inside or the outside of the current line	

	bool inside=false;

	if (lineNormal[lineNo]) {     //line is more horizontal than vertical.

    //now imagine a vertical line cutting through the point (pointX, pointY). We calculate
    //the Y-value where this vertical line intersects with shadow line numbered: 'lineNo'.
    //if this intersect y value > pointY then the point is INSIDE iff x1<x2, and
    //if this intersect y value < pointY then the point is INSIDE iff x1>x2.

    //     |      intersect Y value           | > |pointY|  <==> |l1X|<|l2X|
    if ( ( (lineM[lineNo]*pointX+lineC[lineNo]) > (pointY) ) == ( l1X < l2X )  ) {
			inside=true;
		}

	}
	else {     //line is more vertical than horizontal.

    //now imagine a horizontal line cutting through the point (pointX, pointY). We calculate
    //the X-value where this horizontal line intersects with shadow line numbered: 'lineNo'.
    //if this intersect x value > pointx then the point is INSIDE iff y1>y2, and
    //if this intersect x value < pointx then the point is INSIDE iff y1<y2.

    //     |      intersect X value           | > |pointX|  <==> |l1Y|>|l2Y|
    if ( ( (lineM[lineNo]*pointY+lineC[lineNo]) > (pointX) ) == ( l1Y > l2Y )  ) {
			inside=true;
		}
	}

  return inside;

}



/******************************************\  THIS DESCRIPTION NEEDS CHANGING
* This function accepts four input paramaters, and has 2 
* output paramaters. The 4 input paramaters represent 2
* co-ordinates which define a line. The output parameters
* are M and C respectively in the equation y=Mx+C that
* defines the line in question.
\******************************************/
static void findEquationForLine (float x1, float y1, float x2, float y2, float* m, float* c, bool *normalEquation) {

    //we first need to check if it's best to represent this line as a Y=MX+C Eqn, or a X+MY+C Eqn
    //which are the normal and inverse equations respectively.
    //NOTE: we use normal if line is more horizontal than vertical, and inverse otherwise.
    if ( fabs(x1-x2) > fabs(y1-y2) ) {

      //the line is more horizontal than vertical, so we use the normal form of the equation

      //find 'm' for the line defined by the input co-ordinates (y=mx+c).
	    *m = (y2-y1)/(x2-x1);

  	  //find 'c' for the line defined by the input co-ordinates (y=mx+c).
	    *c = y1-*m*x1;

      //set the equation type flag.
      *normalEquation=true;

    }
    else {

      //the line is more horizontal than vertical, so we use the normal form of the equation

      //first we must check that the two co-ordinate points are not the same
      //NOTE not ncessary for use in hovercraft game!!
      //if ((x2-x1)==0.0f) {
        //*************THROW AN EXCEPTION HERE************   
      //}

      //find 'm' for the line defined by the input co-ordinates (x=my+c).
	    *m = (x2-x1)/(y2-y1);

  	  //find 'c' for the line defined by the input co-ordinates (x=my+c).
	    *c = x1-*m*y1;

      //set the equation type flag.
      *normalEquation=false;

    }

}