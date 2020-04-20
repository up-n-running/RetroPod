
#include "Node.h"

//initialise our static Node count
int Node::NoOfNodes=0;



//**********Construction/Destruction*********


Node::Node(float xVal, float yVal)
{
	x=xVal;
	y=yVal;
  firstNode=false;
	hasBeenVisited=false;
  inside=false;
	next=0;

  NoOfNodes++;

};

Node::~Node()
{
  NoOfNodes--;
}


//************Static Member Function***************
int Node::getNoOfNodes() {
  return NoOfNodes;
}
