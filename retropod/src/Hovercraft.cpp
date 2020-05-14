#ifdef __APPLE__
#include <GLUT/glut.h>
#include <stdlib.h>  // exit()
#include <unistd.h>  // close(), write()
#include <Carbon/Carbon.h>
#else
#include <windows.h>
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <math.h>

#include "Hovercraft.h"

// #define PI 3.1415926535897 //defined in carbon

#define unitXVec Vector(1.0f, 0.0f, 0.0f)
#define unitYVec Vector(0.0f, 1.0f, 0.0f)
#define unitZVec Vector(0.0f, 0.0f, 1.0f)

//virtual keys
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28

//--------------------CONSTRUCTORS / DESTRUCTORS------------------------
Hovercraft::Hovercraft(void) {
	displacement=Vector(0.0f, 0.0f, 0.0f);
	velocity=Vector(0.0f, 0.0f, 0.0f);
	jetRows=5;
	jetCols=4;
	length=width=1.0f;
	height=0.2f;
	mass=0.1f;   ///we should model this as having a weight of 1.
	orientation=MakeQFromEulerAngles(0.0, 0.0, 0.0);
	localXVector=QVRotate(orientation, unitXVec);
	localYVector=QVRotate(orientation, unitYVec);
	localZVector=QVRotate(orientation, unitZVec);
	angularVelocity=Vector(0.0, 0.0, 0.0);

	inertiaMatrix=Matrix3x3(mass*(height*height+length*length)/12.0f, 0.0f, 0.0f, 0.0f,
							mass*(width *width +length*length)/12.0f, 0.0f, 0.0f, 0.0f,
							mass*(height*height+width *width )/12.0f);

	inertiaMatrixInverse=inertiaMatrix.Inverse();

	fanLocalXDistFromMiddle=0.4f;
	fanLocalYDistFromMiddle=0.01f;
	fanForwardForce=0.2f;
	fanBackwardForce=0.15f;

	currentCheckpoint=0;
	currentDamage=0.0f;
}


Hovercraft::Hovercraft(Vector position) {
	displacement=position;  //is this just a pointer operation
	velocity=Vector(0.0f, 0.0f, 0.0f);
	jetRows=5;
	jetCols=4;
	length=height=width=1.1f;
	height=0.2f;
	mass=0.1f;
	orientation=MakeQFromEulerAngles(0.0, 0.0, 0.0);
	localXVector=QVRotate(orientation, unitXVec);
	localYVector=QVRotate(orientation, unitYVec);
	localZVector=QVRotate(orientation, unitZVec);

	angularVelocity=Vector(0.0, 0.0, 0.0);

	inertiaMatrix=Matrix3x3((height*height+length*length)/120.0f, 0.0f, 0.0f, 0.0f,
							(width *width +length*length)/120.0f, 0.0f, 0.0f, 0.0f,
							(height*height+width *width )/120.0f);

	inertiaMatrixInverse=inertiaMatrix.Inverse();

	//printf("%f    %f    %f\n", inertiaMatrixInverse.e11, inertiaMatrixInverse.e21, inertiaMatrixInverse.e31);
	//printf("%f    %f    %f\n", inertiaMatrixInverse.e12, inertiaMatrixInverse.e22, inertiaMatrixInverse.e32);
	//printf("%f    %f    %f\n", inertiaMatrixInverse.e13, inertiaMatrixInverse.e23, inertiaMatrixInverse.e33);

	fanLocalXDistFromMiddle=0.4f;
	fanLocalYDistFromMiddle=0.01f;
	fanForwardForce=0.3f;    
	fanBackwardForce=0.2f;    ///change me back maybe

	currentCheckpoint=0;
	currentDamage=0.0f;

}


Hovercraft::~Hovercraft(void) {

	//put stuff in me (not in the biblical sense)
}


//--------------------OTHER PUBLIC FUNCTIONS--------------------------------

void  Hovercraft::manuallyResetPosition(Vector d) {
  displacement=d;
  velocity=Vector(0.0f, 0.0f, 0.0f);
}


Vector Hovercraft::getDisplacement() {
	return displacement;
}


Vector Hovercraft::getUnitJetVector(){
  return (-localYVector);

}


Vector Hovercraft::getJetPosition(int colNo, int rowNo) {
  ///MUST VECTORISE THIS
  Vector localPosition;
  Vector jetPosn;
  localPosition=((  (float)colNo/((float)jetCols-1) - 0.5f  ) * width )*localXVector
	          + (-height/2.0f                                         )*localYVector
			  + ((  (float)rowNo/((float)jetRows-1) - 0.5f  ) * length)*localZVector;

  jetPosn.x=displacement.x + localPosition.x;
  jetPosn.y=displacement.y + localPosition.y;
  jetPosn.z=displacement.z + localPosition.z;  ///MERGE THESE THREE LINES

  return jetPosn;
}

void Hovercraft::getShadowCoordinates(Vector *p1, Vector *p2, Vector *p3, Vector* p4) { 
 ///MUST DO SOMETHING ABOUT THESE ARGUMENTS
  p1->x=p2->x=p3->x=p4->x=displacement.x;
  p1->y=p2->y=p3->y=p4->y=displacement.y;  ///THESE THREE LINES CAN BE MERGED INTO 1
  p1->z=p2->z=p3->z=p4->z=displacement.z;

  //as if looking from above, so we ignore y co-ords
  *p1 += -0.5f * width * localXVector - 0.5f * length * localZVector;
  *p2 += +0.5f * width * localXVector - 0.5f * length * localZVector;
  *p3 += +0.5f * width * localXVector + 0.5f * length * localZVector;
  *p4 += -0.5f * width * localXVector + 0.5f * length * localZVector;

}

int Hovercraft::getJetRows() {
	return jetRows;
}

int Hovercraft::getJetCols() {
	return jetCols;
}


float Hovercraft::jetDistance2JetForce (float distance) {

  float force= 1.0f / (distance+0.25f) ;   //(distance+0.3f) :- lots of collisions but quite stable, (distance+0.0f) :- a bit like hovercraft pinball, but not that many actual collisions!!

  //for the rest of the function, we apply a bound to the force so it doesn't do anything stupid

	if (force<0.0f || force>10.0f) { 

    //here the jet distance is really small (jet is really close to ground), or even
    //negative (underground), so we must set the force to the upper bound.
    force=10.0f;

  }
  else {
	  if (force<0.4f) {

      //here the jet is miles away from the ground, so we apply a 'token gesture' force
      force=0.4f;

    }
  }

  return force;

}



Vector Hovercraft::getJetLocalDisplacementFromCentre(int jetCol, int jetRow) {

  Vector localDist(0.0f, 0.0f, 0.0f);
			 
  localDist.x= - ((  (float)jetRow/((float)jetRows-1) - 0.5f  ) * width );
	localDist.z=   ((  (float)jetCol/((float)jetCols-1) - 0.5f  ) * length);

  return localDist;
}



Vector Hovercraft::getPendulumMoments(float pendulumLength, float pendulumWeight) {
	Vector moment( (localZVector.y*pendulumLength*pendulumWeight) , 0.0f , (-localXVector.y*pendulumLength*pendulumWeight) );
  return moment;
}


Vector Hovercraft::getResultantFanForce() {

  Vector fanForce = -localZVector * (getRightFanForceMagnitude() + getLeftFanForceMagnitude());
  return fanForce;
}


Vector Hovercraft::getResultantFanMoment() {
  
  float right = getRightFanForceMagnitude();
  float left  = getLeftFanForceMagnitude();

  Vector fanMoment( fanLocalYDistFromMiddle*(left+right) , fanLocalXDistFromMiddle*(right-left) , 0.0f );

  return fanMoment;
}


float Hovercraft::getFanLocalXDistFromMiddle() {
	return fanLocalXDistFromMiddle;
}

float Hovercraft::getFanLocalYDistFromMiddle() {
	return fanLocalYDistFromMiddle;
}

float Hovercraft::getLeftFanForceMagnitude() {
        //VK_UP is Windows and kVK_UpArrow is mac so this is a bit of a hack but there's no platform 
        //independent implementation of 'is a key pressed' which is madness!
	if ( IsKeyDown(VK_UP) || IsKeyDown(VK_RIGHT) || IsKeyDown(kVK_UpArrow) || IsKeyDown(kVK_RightArrow) ){
		return fanForwardForce;
	}
	if ( IsKeyDown(VK_DOWN) || IsKeyDown(kVK_DownArrow) ){
		return -fanBackwardForce;
	}
	return 0.0f;
}

float Hovercraft::getRightFanForceMagnitude() {
        //VK_UP is Windows and kVK_UpArrow is mac so this is a bit of a hack but there's no platform 
        //independent implementation of 'is a key pressed' which is madness!
	if ( IsKeyDown(VK_UP) || IsKeyDown(VK_LEFT) || IsKeyDown(kVK_UpArrow) || IsKeyDown(kVK_LeftArrow)){
		return fanForwardForce;
	}
	if ( IsKeyDown(VK_DOWN) || IsKeyDown(kVK_DownArrow) ){
		return -fanBackwardForce;
	}
	return 0.0f;
}

Vector Hovercraft::getMinusLocalUnitZVector(){
	return -localZVector;
}

void Hovercraft::fetchDimensions(float *dims) {
	dims[0]=width;
	dims[1]=height;
	dims[2]=length;
}


Vector Hovercraft::getIdealCameraPosition() {
  Vector position;
	position.x = displacement.x+5.0f*localZVector.x;
	position.y = displacement.y+3.0;
	position.z = displacement.z+5.0f*localZVector.z;
  return position;
}



void Hovercraft::accelerate(Vector acceleration, float time) {
		velocity+=acceleration*time;
}

void Hovercraft::moveFor(float time) {   ///get rid of the 0.97
	float frictionSlowDown=0.99*pow(10, -0.07f*time);   //the -0.06614 constant is log base 10 of (0.8587), and represents the fact that we want a speed of 1 to be a speed of 0.8587 after 1 unit of time 
//	printf("%f     \n", frictionSlowDown);
		velocity*=frictionSlowDown;
		displacement+=velocity;
}



void Hovercraft::applyTorque(Vector moment, float time) {

	//make moment into a vector
	///Vector moments(moment[0], moment[1], moment[2]);

	//add angular acceleration onto angular velocity
	angularVelocity += inertiaMatrixInverse * 
						(moment - 
						(angularVelocity^
						(inertiaMatrix * angularVelocity)))
						* time;	

//	printf("xRotVel=%f yRotVel=%f zRotVel=%f\n", angularVelocity.x, angularVelocity.y, angularVelocity.z);
}

void Hovercraft::rotateFor(float time) {

	//calculate friction slow down for this time period
	float frictionSlowDown=pow(10, -1.2098f*time);   //the -0.7745098 constant is described above
//	printf("%f     \n", frictionSlowDown);

	//increment orientation by angular velocity, and apply friction to it also. 
	angularVelocity*=frictionSlowDown;  //vector * scalar
	//bound the angular velocity to avoid spaccing
	float max=5.0f;
	if (angularVelocity.x<-max) angularVelocity.x=-max;
	if (angularVelocity.x>max) angularVelocity.x=max;
	if (angularVelocity.y<-max) angularVelocity.y=-max;
	if (angularVelocity.y>max) angularVelocity.y=max;
	if (angularVelocity.z<-max) angularVelocity.z=-max;
	if (angularVelocity.z>max) angularVelocity.z=max;

//	printf("ANG VEL : %f, %f, %f\n", angularVelocity.x, angularVelocity.y, angularVelocity.z);
	orientation +=	(orientation * (angularVelocity/1.0)) * (0.5f * time);

	// now normalize the orientation quaternion:
	float mag = orientation.Magnitude();
	if (mag != 0)
		orientation /= mag;

	//now recalculate local axis vectors
	localXVector=QVRotate(orientation, unitXVec);
	localYVector=QVRotate(orientation, unitYVec);
	localZVector=QVRotate(orientation, unitZVec);
}


void Hovercraft::collisionSlowDown(void){
	currentDamage+=velocity.Magnitude();
  velocity=Vector(0.0f, 0.0f, 0.0f);
}

void Hovercraft::applyDamage(float damage) {
  currentDamage+=damage;
}


void Hovercraft::draw(void) {
  glLoadIdentity();
  glColor3f(1.0, 1.0, 0.0);
  //glTranslatef(displacement[0], displacement[1], displacement[2]);
  //glRotatef(ballAngleX, 0.0f, 0.0f, 1.0f);
  //glRotatef(ballAngleZ, 1.0f, 0.0f, 0.0f);
  
///  float jX, jY, jZ;
///  jX=sin(ballAngleX);
///  jY=-cos(ballAngleX)*sin(ballAngleZ);
///  jZ=-cos(ballAngleX)*cos(ballAngleZ);

  GLfloat ballSpecular[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat ballShinyness[] = {50.0};
  GLfloat ballDiffuse[] = {1.0, 1.0, 0.0, 1.0};
  GLfloat ballAmbience[] = {1.0, 1.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT, GL_SPECULAR, ballSpecular);
  glMaterialfv(GL_FRONT, GL_SHININESS, ballShinyness);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, ballDiffuse);
  glMaterialfv(GL_FRONT, GL_AMBIENT, ballAmbience);


/*
  // **************************TESTING JET VECTORS*****************
	//fetch unit jet vector
	float jetUnitVec[3];
	getUnitJetVector(jetUnitVec);

	//loop through all the jet forces acting on the craft
	for (int jetRow=0; jetRow<jetRows; jetRow++) {
		for (int jetCol=0; jetCol<jetCols; jetCol++) {

			// ****FOR EACH JET FORCE****
			//fetch jet position
			float jetPos[3];
			getJetPosition(jetPos, jetCol, jetRow);

			glBegin(GL_LINE_LOOP);
				glVertex3f(jetPos[0], jetPos[1], jetPos[2]);
				glVertex3f(jetPos[0]+3*jetUnitVec[0], jetPos[1]+3*jetUnitVec[1], jetPos[2]+3*jetUnitVec[2]);
			glEnd();
		}
	}
  // *****************END TESTING***************************
*/
  glTranslatef(displacement.x, displacement.y, displacement.z);

  //put this line back up to the top*****************************

  /* Used for testing only
  //find orientation of rod and draw it
  Vector down(0.0, -1.0, 0.0);
  Vector fish=QVRotate(orientation, down);
  glBegin(GL_LINE_LOOP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(10*fish.x, 10*fish.y, 10*fish.z);
  glEnd();
  */
    
  //do rotation
  Vector axis=QGetAxis(orientation);
  glRotatef(RadiansToDegrees(QGetAngle(orientation)), axis.x, axis.y, axis.z);
  //printf("Z=%f Y=%f X=%f Angle=%f\n", axis.z, axis.y, axis.x, RadiansToDegrees(QGetAngle(orientation)));

  //draw it                \MAIN/
  glJohnSolidCuboid(width, length, height);
///glBegin(GL_LINE_LOOP);
///glVertex3f(0.0f, 0.0f, 0.0f);
///glVertex3f(50*jX, 50*jY, 50*jZ);
///glEnd();

  //fans.....
  float fansize=0.2f;

  GLfloat fanSpecular[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat fanShinyness[] = {50.0};
  GLfloat fanDiffuse[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat fanAmbience[] = {1.0, 0.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT, GL_SPECULAR, fanSpecular);
  glMaterialfv(GL_FRONT, GL_SHININESS, fanShinyness);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, fanDiffuse);
  glMaterialfv(GL_FRONT, GL_AMBIENT, fanAmbience);

  glTranslatef(-fanLocalXDistFromMiddle+0.1f, fanLocalYDistFromMiddle+fansize/2, length/2-fansize/2);
  glutSolidCube(fansize);

  glTranslatef(fanLocalXDistFromMiddle*2-0.2, 0.0f, 0.0f);
  glutSolidCube(fansize);

}

//**************************************PRIVATE FUNCTIONS***********************************
/* UNCOMMENT TO COMPILE ON WINDOWS
 * bool Hovercraft::IsKeyDown(short KeyCode) {
	SHORT	retval;
	retval = GetAsyncKeyState(KeyCode);
	if (HIBYTE(retval))
		return true;
	return false;
}
 */


/* UNCOMMENT TO COMPILE ON MAC */    
bool Hovercraft::IsKeyDown( short KeyCode ) 
{ 
    unsigned short inKeyCode = KeyCode;
    unsigned char keyMap[16]; 
    GetKeys((BigEndianUInt32*) &keyMap);
    
    //const char* t = reinterpret_cast<const char *>( keyMap[ inKeyCode >> 3] );
    
    //printf(t);
    return (bool)((0 != ((keyMap[ inKeyCode >> 3] >> (inKeyCode & 7)) & 1))); 
}    
   /* 
uint16_t vKey = kVK_LeftArrow;
uint8_t index = (vKey - 1) / 32;
uint8_t shift = ((vKey - 1) % 32);
KeyMap keyStates;
GetKeys((BigEndianUInt32*)keyStates);
if (keyStates[index] & (BigEndianUInt32)(1 << shift))
{
    // left arrow key is down
}

}
*/




void Hovercraft::glJohnSolidCuboid(float wid, float len, float hei) {

	float x, y, z;  //stores distance of the faces away from the origin along the x, y and z axis.
	x=wid/2;
	y=hei/2;
	z=len/2;

    //NOW DRAW ALL FACES
	//each face's vertices begin from top left and go clockwise (when looking from above directly at the face)
	glBegin(GL_POLYGON);  //face nearest camera.
		glNormal3f( 0.0f, 0.0f, 1.0f); 
		glVertex3f(-x, y, z);
		glVertex3f( x, y, z);
		glVertex3f( x,-y, z);
		glVertex3f(-x,-y, z);
	glEnd();
	glBegin(GL_POLYGON);  //face furthest away from camera.
		glNormal3f( 0.0f, 0.0f,-1.0f); 
		glVertex3f( x, y,-z);
		glVertex3f(-x, y,-z);
		glVertex3f(-x,-y,-z);
		glVertex3f( x,-y,-z);
	glEnd();
	glBegin(GL_POLYGON);  //left hand face
		glNormal3f(-1.0f, 0.0f, 0.0f); 
		glVertex3f(-x, y,-z);
		glVertex3f(-x, y, z);
		glVertex3f(-x,-y, z);
		glVertex3f(-x,-y,-z);
	glEnd();
	glBegin(GL_POLYGON);  //right hand face
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( x, y, z);
		glVertex3f( x, y,-z);
		glVertex3f( x,-y,-z);
		glVertex3f( x,-y, z);
	glEnd();
	glBegin(GL_POLYGON);  //top face
		glNormal3f( 0.0f, 1.0f, 0.0f); 
		glVertex3f(-x, y,-z);
		glVertex3f( x, y,-z);
		glVertex3f( x, y, z);
		glVertex3f(-x, y, z);
	glEnd();
	glBegin(GL_POLYGON);  //bottom face
		glNormal3f( 0.0f,-1.0f, 0.0f); 
		glVertex3f(-x,-y, z);
		glVertex3f( x,-y, z);
		glVertex3f( x,-y,-z);
		glVertex3f(-x,-y,-z);
	glEnd();

}