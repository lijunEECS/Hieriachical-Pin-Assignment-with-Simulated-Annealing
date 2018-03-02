#ifndef ASSIGN_H
#define ASSIGN_H

#include <string>
#include <map>
#include "oaDesignDB.h"
#include "PAutils.h"

using namespace std;
using namespace oa; 

class pinAssigner
{
public:
	pinAssigner(oaBlock* topblock);
	virtual ~pinAssigner();
	void printPinAssignment();
private:
	PAsolution solution;
	oaNativeNS ns;
};

#endif