#ifndef ASSIGN_H
#define ASSIGN_H

#include <string>
#include <map>
#include "oaDesignDB.h"
#include "PAutils.h"
#include "ProjectInputRules.h"

using namespace std;
using namespace oa; 

class pinAssigner
{
public:
	pinAssigner(oaBlock* topblock, ProjectInputRules rules);
	virtual ~pinAssigner();
	void printPinAssignment();
private:
	PAsolution solution;
	PAsolution originalPA;
	oaNativeNS ns;
	int pinMoveStep;
	int minPinPitch;
	int maxPinPerturbation;
};

#endif