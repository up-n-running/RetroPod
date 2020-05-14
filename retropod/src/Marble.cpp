#ifdef __APPLE__
#include <GLUT/glut.h>
#include <stdlib.h>  // exit()
#include <unistd.h>  // close(), write()
#else
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <math.h>

#include "Hovercraft.h"

#define PI 3.1415926535897

//make the main global variables visible
extern float ballAngleX;
extern float ballAngleZ;

//--------------------CONSTRUCTORS / DESTRUCTORS------------------------
Hovercraft::Hovercraft(void) {
	displacement=new float[3];
	velocity=new float[3];
	for (int i=0; i<3; i++) {
		displacement[i]=0.0;
		velocity[i]=0.0;
	}
}


Hovercraft::Hovercraft(float* position) {
	displacement=new float[3];
	velocity=new float[3];
	for (int i=0; i<3; i++) {
		displacement[i]=position[i];
		velocity[i]=0.0;
	}
}


Hovercraft::~Hovercraft(void) {
}


//--------------------OTHER PUBLIC FUNCTIONS--------------------------------

void  Hovercraft::manuallySetDisplacement(float x, float y, float z) {
  displacement[0]=x;
  displacement[1]=y;
  displacement[2]=z;
}


float* Hovercraft::getDisplacement(void) {
	return displacement;
}

void Hovercraft::accelerate(float* acceleration) {
	for (int i=0; i<3; i++)
		velocity[i]+=acceleration[i];
}

void Hovercraft::moveFor(float time) {
	for (int i=0; i<3; i++)
		displacement[i]+=velocity[i]*time;
}

void Hovercraft::draw(void) {
  glLoadIdentity();
  glColor3f(1.0, 1.0, 0.0);
  glTranslatef(displacement[0], displacement[1], displacement[2]);
  //glRotatef(ballAngleX, 0.0f, 0.0f, 1.0f);
  //glRotatef(ballAngleZ, 1.0f, 0.0f, 0.0f);

  
  float jX, jY, jZ;
  jX=sin(ballAngleX);
  jY=-cos(ballAngleX)*sin(ballAngleZ);
  jZ=-cos(ballAngleX)*cos(ballAngleZ);

  GLfloat ballSpecular[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat ballShinyness[] = {50.0};
  GLfloat ballDiffuse[] = {1.0, 1.0, 0.0, 1.0};
  GLfloat ballAmbience[] = {1.0, 1.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT, GL_SPECULAR, ballSpecular);
  glMaterialfv(GL_FRONT, GL_SHININESS, ballShinyness);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, ballDiffuse);
  glMaterialfv(GL_FRONT, GL_AMBIENT, ballAmbience);

  glutSolidCube(1.0);
  glBegin(GL_LINE_LOOP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(50*jX, 50*jY, 50*jZ);
  glEnd();

}