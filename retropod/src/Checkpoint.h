#include "physicsmaths.h"

class Checkpoint {

private:

	struct Particle {
		float initX;
		float initY;
		float initZ;
		float initTime;
        //initial x, y and z resultant of velocity, plus time particle was emmited
	};

	int       maxParticles;
	float     gravity;
	float     whoomph;
	float	  currentTime;
	Particle* part;

public:

  Checkpoint(void);
	Checkpoint(int maxParticle, float whoomp);
	~Checkpoint(void);

  void updateBy(float time);
	void draw(Vector Position);

private:

	void resetParticle(int particleNo);

};