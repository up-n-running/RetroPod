//#define NULL 0;

class Node  
{

public:

  //variables
	float x;
	float y;
	bool  firstNode;
	bool  hasBeenVisited;
  bool  inside;
	Node *next;

  //functions
	Node(float xVal, float yVal);
	~Node();

  static int getNoOfNodes();


private:
  //variables
  static int NoOfNodes;

};

