#include "physicsmaths.h"

class Hovercraft {

private:
	//variables describing the physical state of the craft
	Vector displacement;
	Vector velocity;
    //orientation
	Quaternion orientation;

public:
	Vector localYVector;
	Vector localXVector;
	Vector localZVector;

private:
	Vector angularVelocity;
    
	//physical properties
	float mass;
	Matrix3x3 inertiaMatrix;
	Matrix3x3 inertiaMatrixInverse;
	//dimensions
	float width;
	float length;
	float height;

	//jet variables
	int   jetRows;
	int   jetCols;

	//fans
	float fanLocalXDistFromMiddle;
	float fanLocalYDistFromMiddle; //note z doesn't matter in terms of model, neg would mean fans pull, pos would mean they push - who cares!
	float fanForwardForce;
	float fanBackwardForce;

	int   currentCheckpoint;
	float currentDamage;

public:
	Hovercraft(void);
	Hovercraft(Vector position);
	~Hovercraft(void);

	void   manuallyResetPosition(Vector d);
	Vector getDisplacement(void);   ///MAYBE DO SOME (PROPPER) INLINING
	Vector getUnitJetVector(void);
	Vector getJetPosition(int colNo, int rowNo);  
  void   getShadowCoordinates(Vector *p1, Vector *p2, Vector *p3, Vector* p4);
	int    getJetRows(void);
	int    getJetCols(void);
  float  jetDistance2JetForce (float distance);
  Vector getJetLocalDisplacementFromCentre(int jetCol, int jetRow);
  Vector getPendulumMoments(float pendulumLength, float pendulumWeight);
  Vector getResultantFanForce();
  Vector getResultantFanMoment();
	float  getFanLocalXDistFromMiddle(void);
	float  getFanLocalYDistFromMiddle(void);
	float  getLeftFanForceMagnitude(void);
	float  getRightFanForceMagnitude(void);
	Vector getMinusLocalUnitZVector(void);
	void   fetchDimensions(float* dims);   ///this, and a lot of other of these functions are bolox.
	Vector getIdealCameraPosition();
	void   accelerate(Vector acceleration, float time);
	void   moveFor(float time);
	void   applyTorque(Vector moment, float time);
	void   rotateFor(float time);
	void   collisionSlowDown(void);
  void   applyDamage(float damage);


  inline int getCurrentCheckpoint(void) {  ///why is this a hovercraft function
		return currentCheckpoint;
	}
	inline void setCurrentCheckpoint(int ccp) {
		currentCheckpoint=ccp;
	}
	inline float getCurrentDamage(void) {
		return currentDamage;
	}

    void   draw(void);

private:
	bool   IsKeyDown(short KeyCode);
	void   glJohnSolidCuboid(float wid, float len, float hei);

};