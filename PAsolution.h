#ifndef PASOLUTION_H
#define	PASOLUTION_H

#include <string>
#include <map>
#include "oaDesignDB.h"
#include "PAutils.h"

using namespace std;
using namespace oa;

#define NOROTATE 0
#define ROTATE90 1
#define ROTATE180 2
#define ROTATE270 3

class PAsolution
{
public:
	PAsolution(oaBlock* topblock);
	virtual ~PAsolution();
	void printSolution();
private:
	oaNativeNS ns;
	pinMove _pinMove;
	map<oaInst*, int> _rotation;
};

#endif
