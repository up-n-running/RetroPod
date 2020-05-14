#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <windows.h>
#include <GL/glut.h>
#endif


#include <stdio.h>
#include <string.h>

//added these 2 to get it working on mac!
#include <stdlib.h> 
#include <unistd.h> 


#include "Model.h"


Model *myModel;
bool paused=false;
bool gameOver=false;
int winWidth=600;   //these store the current window height and width.
int winHeight=600;  //in case any external classes need to know.


void init ( void )
{

   //SETUP OPENGL PARAMATERS 
   glEnable     ( GL_DEPTH_TEST );
   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

   //SETUP OPENGL LIGHTING
   GLfloat lightPosition []={-35.0f, 75.0f, -100.0f, 0.0f};  //ininitely far away
   GLfloat lightAmbience []={0.05f, 0.05f, 0.05f, 1.0f};
   GLfloat lightDiffusion[]={0.95f, 0.95f, 0.95f, 1.0f};
   glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
   glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbience);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffusion);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_LEQUAL);
   
   //PRINT LEVEL SELECT TEXT
   printf("Please Select The Level Appropriate To Your Computer Speed...\n\n"); 
   printf("1: Small Level, Low  Checkpoint Detail; For Bog Standard Graphics Cards.\n"); 
   printf("2: Small Level, High Checkpoint Detail; For 16Mb-ish Graphics Cards.\n"); 
   printf("3: Large Level, Low  Checkpoint Detail; For 32Mb-ish Graphics Cards.\n"); 
   printf("4: Large Level, High Checkpoint Detail; For 64Mb-ish Graphics Cards.\n"); 
   printf("5: Large Level, Mega Checkpoint Detail; For Supercomputers.\n\n"); 

   //RETRIEVE USER INPUT AND GENERATE A FILENAME
   char userInput[256];
   char fileName[256];
   bool validEntry=false;

   while (!validEntry) {

      printf("Please Input 1, 2, 3, 4 or 5\n");
      gets(userInput);
      validEntry=true;

      //now we check that the user input is a single character and this character is one of 1, 2, 3, 4 or 5
      if (!userInput[1]) {

        switch (userInput[0]) {
          case '1':
            strcpy(fileName, "small_100.txt"); break;
          case '2':
            strcpy(fileName, "small_1200.txt"); break;
          case '3':
            strcpy(fileName, "massive_200.txt"); break;
          case '4':
            strcpy(fileName, "massive_3000.txt"); break;
          case '5':
            strcpy(fileName, "massive_9000.txt"); break;
          default:
            validEntry=false; break;
        }

      }
      else {

        validEntry=false;

      }

   }

   //CREATE GAME MODEL
   printf("Loading Level, Please Wait.....\n");
   myModel = new Model(fileName);

}



void display ( void )
{
   myModel->draw();
   glutSwapBuffers();
}



void reshape(int w, int h)
{
   winWidth=w;
   winHeight=h;
   glViewport     ( 0, 0, w, h );
}



void idle(void)
{
  if(!paused && !gameOver) {
		gameOver = myModel->update(0.07f);   //0.07 is a nice time step, much bigger and the hovercraft reacts badly to collisions.
		glutPostRedisplay();
	}
}



#pragma argsused   //used with borland compiler
void keyboard ( unsigned char key, int x, int y )
{
   switch ( key ) {
      case 27:  /*  Escape key  */
         exit ( 0 );
         break;
      case 'f':
         glutFullScreen ( );
         break;
      case 'w':
         glutReshapeWindow ( 600, 600 );
         break;
      case 'p':
         paused=!paused;
         break;
      default:
         break;
   }
   
}



/*  Main Loop
 *  Open window with initial window size, title bar,
 *  RGBA display mode, and handle input events.
 */
int main ( int argc, char** argv )
{

   glutInit ( &argc, argv );
   glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE );
   glutInitWindowSize  ( 600, 600 );
   glutCreateWindow    ( "Retro Pod" );
   init ( );
   glutReshapeFunc     ( reshape  );
   glutKeyboardFunc    ( keyboard );
   glutDisplayFunc     ( display  );
   glutIdleFunc        ( idle     );
   glutMainLoop        ( );
  return 0;

}
