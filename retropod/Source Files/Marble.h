class Hovercraft {

private:
	float *displacement;
	float *velocity;

public:
	Hovercraft(void);
	Hovercraft(float* position);
	~Hovercraft(void);

	void   manuallySetDisplacement(float x, float y, float z);
	float* getDisplacement(void);
	void   accelerate(float* acceleration);
	void   moveFor(float time);
    void   draw(void);

};