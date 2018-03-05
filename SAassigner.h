#ifndef SAASSIGNER_H
#define	SAASSIGNER_H

#include "PAsolution.h"
#include <set>
#include "ProjectInputRules.h"
#include "OAHelper.h"
using namespace std;
using namespace oa;

class SAassigner
{
public:
	SAassigner(PAsolution& solution, ProjectInputRules& rules, pinDict& dict, oaNativeNS _ns);
	virtual ~SAassigner();

private:
	pinDict _pinDict;
	PAsolution _solution;
	int pinMoveStep;
	int minPinPitch;
	int maxPinPerturbation;
	oaNativeNS ns;
};

#endif
