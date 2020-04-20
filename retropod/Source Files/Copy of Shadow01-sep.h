#include "Node.h"

class Shadow {

public:

  static void drawShadow(float* shadowObjectX, float* shadowObjectY, int shadowObjectSize, Surface *mySurface);

private:

  static float* lineM;
  static float* lineC;
  static bool*  lineNormal;
  
  static void initShadowLines (float* linesX, float* linesY, int noOfLines);
  static void lineCrossesEdge (int lineNo, float thisNodeX, float thisNodeY, float nextNodeX, float nextNodeY,
                               bool *doesItCross, float *crossX, float *crossY );
  static bool isPointOnInsideOfLine (int lineNo, float l1X, float l1Y, float l2X, float l2Y, float pointX, float pointY);
  static void findEquationForLine (float x1, float y1, float x2, float y2, float* m, float* c, bool *inversed);
  static void initShadowLinkedList (int colNo, int rowNo, bool topLeft, Node **nThisPointer, Node **nLastPointer);
};