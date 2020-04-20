#include "Surface.h"
#include "Hovercraft.h"
#include "Checkpoint.h"

class Model {   ///remember to tart up .h files as well

private:

	int jetCutOffDist;
	bool craftAboveGround;
	float modelTime;

  //stuff defined in level file
	Vector   startPosn;
	float    timeLimit;
	float    maxDamage;
	int      noOfCheckpoints;
	Vector*  checkpointPosn;
	int      checkPointDetail;

public:

 Vector cameraPosition;

	Surface *mySurface;
	Hovercraft *myHovercraft;
	Checkpoint *myCheckpoint;


	Model(void);
	Model(int surfXPoints, int surfZPoints, Vector craftPosition);
	Model(char* fileName);
	~Model(void);

  bool update (float time);
	void draw (void);

private:

	float jetDistance(int rowNo, int colNo, bool topLeft, Vector jetVector, Vector posVector);
	void  printStatus( void );
  void  printGameOverMessage( void );
	void  stringPercent(char* str, int percent);
	void  showText( void *font, char *str );
  void  updateCameraPosition( void );
  bool  isGameOver( void );

};