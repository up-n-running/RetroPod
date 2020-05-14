#include "Node.h"
#include "Surface.h"

  ///THIS FILE IS ALL BOLOX, 
class Shadow {

public:

  Shadow();
  Shadow(float* shadowObjectX, float* shadowObjectY, int shadowObjectSize);
  ~Shadow();
  void drawShadow(float* shadowObjectX, float* shadowObjectY, int shadowObjectSize, Surface *mySurface);

private:

  float* lineM;
  float* lineC;
  bool*  lineNormal;
  
  void initShadowLines (float* linesX, float* linesY, int noOfLines);
  void lineCrossesEdge (int lineNo, float thisNodeX, float thisNodeY, float nextNodeX, float nextNodeY,
                               bool *doesItCross, float *crossX, float *crossY );
  bool isPointOnInsideOfLine (int lineNo, float l1X, float l1Y, float l2X, float l2Y, float pointX, float pointY);
  void findEquationForLine (float x1, float y1, float x2, float y2, float* m, float* c, bool *inversed);
  void initShadowLinkedList (int colNo, int rowNo, bool topLeft, Node **nThisPointer, Node **nLastPointer);
  void finishShadowLinkedList (Node *nThis, Node* nLast, Surface *mySurface, bool drawIt);
  
  

};