#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "Checkpoint.h"

//--------------------CONSTRUCTORS / DESTRUCTORS------------------------

Checkpoint::Checkpoint() {
	maxParticles=30;
	whoomph=15.0f;
	currentTime=0.0f;
	gravity=9.0f;
	part=new Particle[maxParticles];
	for (int i=0; i<maxParticles; i++) {
		resetParticle(i);
	}
}


Checkpoint::Checkpoint(int maxParticle, float whoomp) {
	maxParticles=maxParticle;
	whoomph=whoomp;
	currentTime=0.0f;
	gravity=9.0f;
	part=new Particle[maxParticles];
	for (int i=0; i<maxParticles; i++) {
		resetParticle(i);
	}
}


Checkpoint::~Checkpoint() {
    delete [] part;
}


//--------------------OTHER PUBLIC FUNCTIONS--------------------------------

void Checkpoint::updateBy(float time) {
	//update time
	currentTime+=time;
}

void Checkpoint::draw(Vector position) {

	//goto the correct point in space
	glLoadIdentity();
	glTranslatef(position.x, position.y, position.z);

	//setup opengl material properties
	GLfloat particleSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat particleShinyness[] = {25.0f};
	GLfloat particleDiffuse[] = {1.0f, 0.0f, 0.3f, 1.0f};
	GLfloat particleAmbience[] = {1.0f, 0.0f, 0.3f, 1.0f};
	glMaterialfv(GL_FRONT, GL_SPECULAR, particleSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, particleShinyness);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, particleDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, particleAmbience);
	
	//declare variables to store the current particles position and time since creation
	Vector posn; 
	float  t;

	//cycle through each particle and draw/reset each one
	for (int i=0; i<maxParticles; i++) {
		t=currentTime-part[i].initTime;
		posn.y=t*part[i].initY - 0.5*t*t*gravity;

		//if it is not still in the air, then reset it, otherwise draw it.
		if (posn.y<-0.5f) {
			resetParticle(i);
		}
		else {
			//calculate x and z posns then draw it
			posn.x=t*part[i].initX;
			posn.z=t*part[i].initZ;
			glPushMatrix();
			    glTranslatef(posn.x, posn.y, posn.z);
				glutSolidCube(0.1);
			glPopMatrix();
		}
	}
}


//-----------------PRIVATE FUNCTIONS--------------------------

void Checkpoint::resetParticle (int particleNo) {
  //srand(time(NULL));rand();
  //printf("%f\n", (float)rand()/(float)RAND_MAX);
  part[particleNo].initX=whoomph*0.3f*(((float)rand()/(float)RAND_MAX)-0.5f);  //rand no between -0.15 and 0.15
  part[particleNo].initZ=whoomph*0.3f*(((float)rand()/(float)RAND_MAX)-0.5f);  //rand no between -0.15 and 0.15
  part[particleNo].initY=whoomph*(((float)rand()/(float)RAND_MAX)+1.0f);  //rand no between 1 and 2
  part[particleNo].initTime=currentTime;
}
