#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Model.h"

//this function draws the shadow of the hovercraft on the surface. it is located in the singleton shadow.cpp
extern int drawShadow(float* shadowObjectX, float* shadowObjectY, int shadowObjectSize, Surface *mySurface);

//--------------------CONSTRUCTORS / DESTRUCTORS------------------------

Model::Model(void) {

  //create constituent objects
  mySurface = new Surface();
	myHovercraft  = new Hovercraft ();
	myCheckpoint = new Checkpoint();

  //setup level specific variables.
	startPosn.x=1.0f;
	startPosn.y=1.0f;
	startPosn.z=-1.0f;
	timeLimit=1000.0f;
	maxDamage=25.0f;
	noOfCheckpoints=1;
	checkpointPosn=new Vector[1];
	for (int cp=0; cp<noOfCheckpoints; cp++) {
		checkpointPosn[cp].x=1.0f;
		checkpointPosn[cp].z=-1.0f;
		checkpointPosn[cp].y=mySurface->getHeightAt(checkpointPosn[cp].x, checkpointPosn[cp].z);
	}
  checkPointDetail=100;

  //setup camera position
  cameraPosition=myHovercraft->getIdealCameraPosition();

  //setup admin variables
  craftAboveGround=true;
	jetCutOffDist=5;
  modelTime=0.0f;
}


Model::Model(int surfXPoints, int surfZPoints, Vector craftPosition) {

  //create constituent objects
  mySurface = new Surface(1, surfXPoints, surfZPoints);
  myHovercraft  = new Hovercraft(craftPosition);
  myCheckpoint = new Checkpoint(checkPointDetail, 8.0f);

  //setup level specific variables
	startPosn.x=1.0f;
	startPosn.y=1.0f;
	startPosn.z=-1.0f;
	timeLimit=1000.0f;
	maxDamage=25.0f;
	noOfCheckpoints=1;
	checkpointPosn=new Vector[1];
	for (int cp=0; cp<noOfCheckpoints; cp++) {
		checkpointPosn[cp].x=1.0f;
		checkpointPosn[cp].z=-1.0f;
		checkpointPosn[cp].y=mySurface->getHeightAt(checkpointPosn[cp].x, checkpointPosn[cp].z);
	}
  checkPointDetail=100;

  //setup camera position
  cameraPosition=myHovercraft->getIdealCameraPosition();

  //setup admin variables
  craftAboveGround=true;
  jetCutOffDist=5;
  modelTime=0.0f;
}


Model::Model(char* fileName) {

	//setup temp string
	char tempString[256];

	//setup file stuff
	FILE **filePointerPointer= new FILE*;
	*filePointerPointer=fopen(fileName, "r");

  //check file thing worked
	if (!(*filePointerPointer)) {
		printf("Sorry, Could Not Find the File %s in the Current Directory\n", fileName);
		exit (0);
	}

  //create constituent Surface Object (this reads the first part of the file we've just opened)
  mySurface = new Surface(1, filePointerPointer);

  //NOW WE LOAD LEVEL SPECIFIC VARIABLES FROM THE REMAINING PARTS OF THE FILE:

	//load blank line
	fgets(tempString, 50, *filePointerPointer);

	//load start position, time limit and max damage info for this level
	startPosn.x=atof(fgets(tempString, 50, *filePointerPointer));
	startPosn.y=atof(fgets(tempString, 50, *filePointerPointer));
	startPosn.z=atof(fgets(tempString, 50, *filePointerPointer));
	timeLimit=atof(fgets(tempString, 50, *filePointerPointer));
	maxDamage=atof(fgets(tempString, 50, *filePointerPointer));

	//get no of checkpoints, and all checkPoint co-ordintes for this level
	noOfCheckpoints=atoi(fgets(tempString, 50, *filePointerPointer));
	checkpointPosn=new Vector[noOfCheckpoints];
	for (int cp=0; cp<noOfCheckpoints; cp++) {
		checkpointPosn[cp].x=atof(fgets(tempString, 50, *filePointerPointer));
		checkpointPosn[cp].z=atof(fgets(tempString, 50, *filePointerPointer));
		checkpointPosn[cp].y=mySurface->getHeightAt(checkpointPosn[cp].x, checkpointPosn[cp].z);
	}

  //get level of checkpoint detail (the higher the detail, the lower the speed)
	checkPointDetail=atoi(fgets(tempString, 50, *filePointerPointer));

	//close file
	fclose(*filePointerPointer);
  delete filePointerPointer;

  //create constituent Hovercraft and Checkpoint Objects
  myHovercraft  = new Hovercraft(startPosn);
  myCheckpoint = new Checkpoint(checkPointDetail, 8.0f);

  //setup camera position
  cameraPosition=myHovercraft->getIdealCameraPosition();

  //setup admin variables
  craftAboveGround=true;
  jetCutOffDist=5;
  modelTime=0.0f;
}



Model::~Model() {
	delete mySurface;
	delete myHovercraft;
  delete myCheckpoint;
	delete [] checkpointPosn;

  ///REMEMBER TO ADD: checkpoint and arrays
}



//--------------------OTHER PUBLIC FUNCTIONS--------------------------------



bool Model::update (float time) {

  //as we calculate each new force acting on the hovercraft, we update the following Vectors
  //which store the forces culumative effect on the hovercraft
	//jetForce stores the of all the forces acting on the craft.
  //jetMoment stores the of all the forces acting on the craft.
	Vector jetForce(0.0f, 0.0f, 0.0f);
	Vector jetMoment(0.0f, 0.0f, 0.0f);

	//variable to see whether any jets are underground
	bool allJetsOverGround=true;

	//find the boundaries of the craft's surrounding surface segments.
  //segments within these boundaries will be checked to see whether
  //any of the craft's jets are pushing against them
	int minCol, maxCol, minRow, maxRow;
	Vector craftPos = myHovercraft->getDisplacement();
	minCol=(int)(floor(craftPos.x))-jetCutOffDist;
	maxCol=minCol+2*jetCutOffDist;
	if (minCol<0) {
		minCol=0;
	}
	if (maxCol>mySurface->getNoOfCols()-1) {
		maxCol=(mySurface->getNoOfCols())-1;
	}
	minRow=(int)(floor(-craftPos.z))-jetCutOffDist;
	maxRow=minRow+2*jetCutOffDist;
	if (minRow<0) {
		minRow=0;
	}
	if (maxRow>mySurface->getNoOfRows()-1) {
		maxRow=(mySurface->getNoOfRows())-1;
	}

	//get the unit length vector describing the direction of the jets
	Vector jetUnitVec = myHovercraft->getUnitJetVector();

	//we model the upward thrust of the hovercraft as a uniform grid (of size jetRows x jetCols)
  //of air jets protuding perpendicularly from the underside of the hovercraft.
	int jetRows=myHovercraft->getJetRows();
	int jetCols=myHovercraft->getJetCols();

	//loop through all the jet forces acting on the craft
	for (int jetRow=0; jetRow<jetRows; jetRow++) {
		for (int jetCol=0; jetCol<jetCols; jetCol++) {

			//****FOR EACH JET FORCE****

			//get the position is space of the point source of the jet.
			Vector jetPos = myHovercraft->getJetPosition(jetCol, jetRow);  ///we can move these 3 variable declarations outside the for loops.

			//after the next two for loops, jetDist will store the distance between the source of the jet and the 
      //point at which the jet first crosses with one of the polygons making up the ground.
			float jetDist=1000.0f;

      //this is used inside the next two for loops to store the distance between
      //the current jet's source and the point where it hits the current polygon in question.
			float tempDist;

			//loop through all the surface segments in the locality of the hovercraft
			for (int row=minRow; row<=maxRow; row++) {
				for (int col=minCol; col<=maxCol; col++) {

          //****FOR EACH SURFACE SEGMENT****

          //each segment is made up of two polygons, so we need to do two sets of calculations...
          //NOTE: the two if statements below, mean that after all the loops round the surface
          //segments, the value of jetDist will be set to that of the nearest polygon below (or
          //JUST above) the hovercraft (in case the hovercraft goes momentarily below ground)

					//find the distance between jet and top left polygon
					tempDist=jetDistance(row,col,true, jetUnitVec, jetPos);
					if (tempDist<jetDist && tempDist>-0.5f) {
						jetDist=tempDist;
					}

          //find the distance between jet and bottom right polygon
					tempDist=jetDistance(row,col,false, jetUnitVec, jetPos);
					if (tempDist<jetDist && tempDist>-0.5f) {
						jetDist=tempDist;
					}

				}
			}

      //check to see if this jet is still above ground
      if ( jetDist <= 0.0f ) {
        allJetsOverGround=false;
      }

			//update variables describing the jet forces on the hovercraft
			//by adding the affects of this jet
			float forceMagnitude = myHovercraft->jetDistance2JetForce(jetDist) ;

      //now add the TRANSLATIONAL effect of this jet to the current total
			//           direction   *   magnitude  
      jetForce+= (-jetUnitVec) * forceMagnitude; 

      //now add the ROTATIONAL effect of this jet to the current total.
      //                     perpendicular direction                       *   magnitude
      jetMoment += myHovercraft->getJetLocalDisplacementFromCentre(jetCol, jetRow) * forceMagnitude;

		}
	}

  //*****DONE LOOPING THROUGH ALL JETS*****

  //using the above system, the more jets we have, the more upward thrust the craft has. we don't
  //want this, so we want to normalise each jet to affect the craft by an ammount of (1/number of jets), which
  //is the relative stength of each jet. This way the hovercraft will hover, however many jets!
	jetForce  /= (jetRows*jetCols);
	jetMoment /= (jetRows*jetCols);

  //the only way to keep the hovercraft from turning over (once this has happened, very strange things start to happen
  //to the modelled physics) is to keep it the right way up. Imagine that as part of every model update, we quickly attach
  //a pendulum, prodruding perpendicularly downwards from the centre of the underside of the hovercraft. the pendulum remains
  //attached for the duration of the time step we are updating the system by. During this time step, we allow the craft to pivot
  //about it's centre. After every iteration of the model update function, the hovercraft will have been 'swung' towards its upright
  //position. It is worth noting that the function that updates the hovercraft's orientation, also applies some rorational
  //friction to the angular velocity vector. this means the swinging of the pendulum is damped (as if the pendulum was swinging in
  //a bucket of treacle). Thsi is how we keep the hovercraft upright.
  //This pendulum is modelled as being of length 1, and with a weight of 0.35
  jetMoment += myHovercraft->getPendulumMoments(1.0f, 0.35f);

	//now add on affects of the 2 fans
  jetForce  += myHovercraft->getResultantFanForce();
	jetMoment += myHovercraft->getResultantFanMoment();

	//although the MASS of the hovercraft is set, we can choose whatever gravitational constant to 
  //suit the gameplay we want, so the WEIGHT of the hovercraft is simply a constant chosen by me.
  //here we hodel the weight to be 0.9.
	jetForce.y-= 0.9f;

  //we are now ready to apply the rotational and translational forces. But before we do, we must first check whether the
  //result of the last time this happened was to make the hovercraft collide with the ground.
	//if we are partially below ground, and we were not the last time this function was called, then implement a collision.
	if (!allJetsOverGround) {
		if (craftAboveGround) {
			myHovercraft->collisionSlowDown();
		}
		craftAboveGround=false;
	}
	else {
		craftAboveGround=true;
	}

	//accelerate hovercraft
	myHovercraft->accelerate(jetForce, time);

  //rotate hovercraft
	myHovercraft->applyTorque(jetMoment, time);

  //update the hovercraft's position and orientation accordingly.
  myHovercraft->moveFor(time);
	myHovercraft->rotateFor(time);

  //see if we have fallen off (i.e. we are at height -10 or below).
	if ( (myHovercraft->getDisplacement()).y < -10.0f ) {
		//we've fallen off - reset hovercraft's position and velocity - and increment damage quota
		myHovercraft->manuallyResetPosition(startPosn);
    myHovercraft->applyDamage(5.0f);
	}

	//check to see if we are near enough to the base of the current checkpoint
	Vector fosh = myHovercraft->getDisplacement();
	if (      (  myHovercraft->getDisplacement()  -  checkpointPosn[myHovercraft->getCurrentCheckpoint()]  ).Magnitude()  <  2.0f) {
		//we have hit the checkpoint, so remake checkpoint an the next place and update myHovercraft.currentCheckpoint
		delete myCheckpoint;
		myCheckpoint=new Checkpoint(checkPointDetail, 8.0f);
		myHovercraft->setCurrentCheckpoint(myHovercraft->getCurrentCheckpoint()+1);
    ///if checkpoint number is too high then do something about it.
    ///something like.........
	  ///if (checkNo>=noOfCheckpoints)
		  ///checkNo=noOfCheckpoints-1;


	}

  //move camera towards its ideal position
  updateCameraPosition();

  //update the 'running total' model time variable.
  modelTime+=time;

  //update the checkpoint's time so that we get the fountainy effect
  myCheckpoint->updateBy(time*0.55f); //this 'fudge factor' of 0.55 is to make acelleration due to gravity appear the same for the checkpoints as for the hovercraft.

  //return a booleam to say whether the game has finished or not.
  return isGameOver();
}



/***********************************************\
* This function not only draws the whole scene, but sets up the
* projection paramaters (lense) and camera position relative to the current origin
* The function makes the assumption that the following openGL constants are enabled:
*  GL_DEPTH_TEST
*  GL_LIGHTING
*  GL_LEQUAL
\***********************************************/
void Model::draw (void) {

  //SETUP CAMERA PROJECTION FOR DRAWING MODEL
  glMatrixMode   ( GL_PROJECTION );
  glLoadIdentity ( );
  glFrustum (-0.1, 0.1, -0.1, 0.1, 0.1, 15000.0);
  //set camera position
  Vector object = myHovercraft->getDisplacement();
  gluLookAt (cameraPosition.x, cameraPosition.y, cameraPosition.z, object.x, object.y, object.z, 0.0, 1.0, 0.0);
  

  //SETUP ACTUAL MODEL
  glMatrixMode   ( GL_MODELVIEW );
  glLoadIdentity ( );
  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPushMatrix ( );

    //DRAW ACTUAL MODEL

    //draw surface, hovercraft and checkpoint
  	mySurface->draw(1);
  	myHovercraft->draw();
  	myCheckpoint->draw(checkpointPosn[myHovercraft->getCurrentCheckpoint()]);  ///why does checkpoint not store it's position
 
    //draw shadow
    float shadowObjectX[4];
    float shadowObjectY[4];
    Vector p1, p2, p3, p4;
    myHovercraft->getShadowCoordinates(&p1, &p2, &p3, &p4);
    shadowObjectX[0]=p1.x ; shadowObjectY[0]=-p1.z ;
    shadowObjectX[1]=p2.x ; shadowObjectY[1]=-p2.z ;
    shadowObjectX[2]=p3.x ; shadowObjectY[2]=-p3.z ;
    shadowObjectX[3]=p4.x ; shadowObjectY[3]=-p4.z ;
    drawShadow(shadowObjectX, shadowObjectY, 4, mySurface);

  glPopMatrix  ( );


  //SET SETTINGS FOR TEXT PRINTING MODE
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST ); 
  glColor3f( 1.0, 1.0, 1.0);

  //SETUP CAMERA PROJECTION FOR DRAWING TEXT
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

    glLoadIdentity();
    glOrtho( 0, 10, 0, 10, -1.0, 1.0 );
  
    //NOW PRINT THE APPROPRIATE TEXT ON THE SCREEN
    if (isGameOver()) {
      printGameOverMessage();
    }
    printStatus();

  glPopMatrix();

  //SET SETTINGS BACK TO MODEL DRAWING MODE
  glEnable( GL_LIGHTING );
  glEnable( GL_DEPTH_TEST ); 


}

//-----------------PRIVATE FUNCTIONS--------------------------



/***********************************************\
*Imagine a 3D line whose direction is described by 'jetVector', that passes through the point 'posVector'.
*Given a particular surface polygon (described by its segment's column and row number, and whether 
*it is the top left or bottom right polygon), this function return the following:
*IF the line does not intersect with the polygon THEN it returns 1000.0
*IF the line does intersect with the polygon THEN it returns the distance between posVector and the point of intersection.
*
*NOTE: For the above returned distance to be correct, the jetVector must be of unit length.
*NOTE2: if the line intersects, but jetVector is pointing away from the polygon, then the distance returned will be negative.
*
*
*HERE'S HOW IT WORKS:
*(in the folowing, a capital denotes a vector, and for instance; R.x denotes the x element of vector R):
*
*
*The triangular polygon in question defines a 3d plane like such:
*
*Take one arbitrary corner (say c1), whose co-ordinates are the vector C1, lets call the other corners c2 and c3, and their 
*respective co-ordinate vectors C2 and C3. We want to find the direction vectors of the 2 edges of the triangle coming out
*of the corner c1. we call these vectors R and Q such that  R = C2 - C1  and  Q = C3 - C1 .
*
*The vector equation of the plane therefore looks like this:
*    ( r_coefficient * R ) + ( q_coefficient * Q ) + C1    where r_coefficient (rCoeff) and q_coefficient (qCoeff) are arbitrary scalars.
*NOTE: any pair of values for rCoeff and qCoeff produce a point on the plane using this equation, and conversely, any point on the plane has a corresponding
*value for the pair rCoeff and qCoeff.
*
*
*By the same logic, the Vector equation of the 3d line is
*    (j_coefficient * J) + K     where J is the function paramater jetVector and K is the function paramater posVector and j_coefficient (jCoeff) is an arbitrary scalar.
*
*
*
*The point at which the line intersects with the triangle's plane is found by solving the following equation.
*    rCoeff*R + qCoeff*Q + C = jCoeff*J + K     where rCoeff, qCoeff and jCoeff are the 3 unknowns.
*
*This equates to the following set of simultaneous equations in matrix/vector format:
*    [ R.x   Q.x  -J.x]   [rCoeff]   [K.x-C.x]
*    [ R.y   Q.y  -J.y] * [qCoeff] = [K.y-C.y]
*    [ R.z   Q.z  -J.z]   [jCoeff]   [K.z-C.z]
*
*We solve these three eqnuations for 3 unknowns using the Gauss-Jordan method of Elimination.
*
*The distance between 'posVector' and the point of intersection is jCoeff multiplied my the length of the 'jetVector'
*
*The point of intersection is within the original triangle iff the following three inequalities hold:
*   rCoeff >= 0
*   qCoeff >= 0
*   rCoeff+qCoeff <= 1
*
*Simple!
*
*
*SOME NOTES ABOUT THE ELIMINATION:
*
*Here is how the above matrix equation looks in terms of the variables defined in the function body.
*
*   For the TOP LEFT triangles:
*    [  1   0  -jetVector.x]   [rCoeff]   [dX]
*    [ hX  hZ  -jetVector.y] * [qCoeff] = [dY]
*    [  0   1  -jetVector.z]   [jCoeff]   [dZ]
*
*   For the BOTTOM RIGHT triangles:
*    [ -1   0  -jetVector.x]   [rCoeff]   [dX]
*    [ hX  hZ  -jetVector.y] * [qCoeff] = [dY]
*    [  0  -1  -jetVector.z]   [jCoeff]   [dZ]
*
*As stated before if we look at the surface from above (i.e. in the X/Z plane), all the polygons are right angled
*triangles. I mention above that we can choose any corner of the polygon to be 'c1' it we choose the 'right angled' corner
*then the R and Q vectors end up having lots of 1's and 0's in, which saves us a lot of multiplications!
*
*dX is simply K.x - C.x,  and likewise with dY and dZ.
*
\***********************************************/
float Model::jetDistance(int rowNo, int colNo, bool topLeft, Vector jetVector, Vector posVector) {
  //variables defining r and q vectors
  float hX, hZ;
  //variables defining multiplicative constants for the elementary row operations,
  //gauss jordan actaully requires 6 elementary row ops, but due to the many 1's and 0's in the
  //original matrix, anly the 3'rd and 4'th require any thinking about.
  float m3, m4;
  //variables to store the coefficient variables
  float rCoeff, qCoeff, jCoeff;
  //variables to store the d values
  float dX, dY, dZ;
  //a variable to store matrix elements after the elem row ops.
  float M23;

  //setup hX, hZ and dX, dY and dZ
  hX=mySurface->getHeightAtIntUnsafe(colNo+1, rowNo+1) - mySurface->getHeightAtIntUnsafe(colNo  , rowNo+1);  ///DOESNT IT MATTER WHETHER WE ARE TOP RIGHT OR BOTTOM LEFT?
  hZ=mySurface->getHeightAtIntUnsafe(colNo  , rowNo  ) - mySurface->getHeightAtIntUnsafe(colNo  , rowNo+1);

  if (!topLeft) {
	  hX=-hX;
	  hZ=-hZ;
    dX=posVector.x-(float)(colNo+1);
    dY=posVector.y-mySurface->getHeightAtIntUnsafe(colNo+1, rowNo);
    dZ=posVector.z+(float)rowNo;
  }
  else {
    dX=posVector.x-(float)colNo;
    dY=posVector.y-mySurface->getHeightAtIntUnsafe(colNo, rowNo+1);
    dZ=posVector.z+(float)(rowNo+1);
  }

  //some hard sums (elimination)
  if (topLeft)
    M23=  jetVector.x*hX + jetVector.z*hZ - jetVector.y;
  else 
    M23= -jetVector.x*hX - jetVector.z*hZ - jetVector.y;

  m3=-jetVector.x/M23;
  m4=-jetVector.z/M23;

  //some hard sums (substitution to solve simultaneous equations) 
  if (topLeft)
    dY=dY-hX*dX-hZ*dZ;
  else
	  dY=dY+dX*hX+dZ*hZ;

  dX=dX-m3*dY;
  dZ=dZ-m4*dY;

  rCoeff=topLeft ? dX : -dX;
  qCoeff=topLeft ? dZ : -dZ;
  jCoeff=dY/M23;

  //now check whether the line actuall crosses through the triangle.
  if (rCoeff>=0 && qCoeff>=0 && (rCoeff+qCoeff)<=1) {
    return jCoeff;
  }

  //if not return default
  return 1000.0f;
}



/**********************************************\
*This function simply accepts a pointer to a string and a integer between 0 and 100
*the string being pointed to is set to a nicely formatted, 4 character long string
*representing the number as a percentage figure.
\**********************************************/
void Model::stringPercent (char* str, int percentage) {
	if (percentage>100)
		percentage=100;
	if (percentage<0)
		percentage=0;
	char temp[5];
	sprintf(temp, "%i%%", percentage);
	strcpy(str, "    ");
	char* strPtr=str;
	char* tempPtr=temp;

	if (percentage<100)
		strPtr++;
	if (percentage<10)
		strPtr++;
	while (*tempPtr) {
		*strPtr++=*tempPtr++;
	}
}



/*****************************************\
*This prints the appropriate 'game status' message in the bottom left the screen.
\*****************************************/
void Model::printStatus() {

	  //****Format the percentage figures so they look nice****

    //format the current model time as a percentage of the total time allowed for the level.
  	char tp[5], t[256];
  	strcpy(t, "          Time: ");
  	stringPercent(tp,  ((1.0f-(modelTime/timeLimit))*100)  );
  	strcat(t, tp);

    //format the current damage remaining as a percentage of the total damage allowed for the hovercraft.
  	char ep[5], e[256];
  	strcpy(e, "       Energy: ");
  	stringPercent(ep,  ((1.0f-(myHovercraft->getCurrentDamage())/(maxDamage))*100)  );
  	strcat(e, ep);

    //format the current no of checkpoints remaining as a percentage of the total no of checkpoints
  	char cp[5], c[256];
  	strcpy(c, "Checkpoints: ");
  	stringPercent(cp,  ((1.0f-((float)myHovercraft->getCurrentCheckpoint())/((float)noOfCheckpoints))*100)   );
  	strcat(c, cp);


	  //****print the text at co-ords relative to bottom left of the screen****
    glRasterPos2i( 1, 1 );
    showText( GLUT_BITMAP_HELVETICA_18, t );
    glRasterPos2f( 1, 1.5 );
    showText( GLUT_BITMAP_HELVETICA_18, e );
    glRasterPos2i( 1, 2 );
    showText( GLUT_BITMAP_HELVETICA_18, c );

}



/*****************************************\
*This prints the appropriate 'game over' message in the centre of the screen.
\*****************************************/
void Model::printGameOverMessage() {

  //generate the game over message, depending on how the game finished...
  char message[256];
  if  ( modelTime >= timeLimit ) {
    strcpy(message, "Game Over : You Ran Out Of Time.");
  }
  if  ( myHovercraft->getCurrentDamage() >= maxDamage ) {
    strcpy(message, "Game Over : Your Craft Is Too Damaged.");
  }
  if  ( myHovercraft->getCurrentCheckpoint() >= noOfCheckpoints ) {
    strcpy(message, "Well Done, You Made It!!!");
  }

  //print the text at co-ords relative to bottom left of the screen
  glRasterPos2i( 3, 6 );
  showText( GLUT_BITMAP_HELVETICA_18, message );
  glRasterPos2i( 3, 4 );
  showText( GLUT_BITMAP_HELVETICA_18, "Now Press Esc To Quit Demo." );

}


/*************************************\
*This prints the string 'str', at the current opengl raster position
*using the font 'font'.
\*************************************/
void Model::showText( void *font, char *str ) {
    int i, l = strlen(str);
    for( i=0; i<l; i++ ) {
        glutBitmapCharacter(font,*str++);
    }
}



/*****************************************\
*This Function asks the hovercraft for a position directly behind itself (and up a bit).
*Then recalculates, the y (height) position, taking the height of the surface into account.
*Then moves the camera position 1/5 of the way towards this ideal position.
*Doing this gives you the impression of the camera constantly 'swinging round' towards it's ideal position.
\*****************************************/
void Model::updateCameraPosition() {

  Vector idealPosition = myHovercraft->getIdealCameraPosition();

  //for the x and z co-ords, swing the camera 1/5th of the way towards it's ideal position from where it is currently
  cameraPosition.x += 0.2 * (idealPosition.x-cameraPosition.x) ;
  cameraPosition.z += 0.2 * (idealPosition.z-cameraPosition.z) ;

  //note: here we completely override the y co-ordinate returned from the getIdealCameraPosition function!
  idealPosition.y = (mySurface->getHeightAt(idealPosition.x, idealPosition.z) + mySurface->getHeightAt(cameraPosition.x, cameraPosition.z) +8.0f) / 2.0f ;

  //now, for the y co-ord, swing the camera 1/5th of the way towards it's ideal position from where it is currently
  cameraPosition.y += 0.2 * (idealPosition.y-cameraPosition.y) ;

}



/*******************************************\
*This function simply looks at the model's, hovercraft's and surface's
*member variables to see if the game is over.
\*******************************************/
bool Model::isGameOver() {

  return (
           ( modelTime >= timeLimit )
      ||   ( myHovercraft->getCurrentDamage() >= maxDamage )
      ||   ( myHovercraft->getCurrentCheckpoint() >= noOfCheckpoints )
  );

}