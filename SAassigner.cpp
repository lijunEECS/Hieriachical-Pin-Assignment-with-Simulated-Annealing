#include "SAassigner.h"
#include <iostream>
using namespace std;

SAassigner::SAassigner(PAsolution& solution, ProjectInputRules& rules, pinDict& dict, oaNativeNS _ns)
{
	_pinDict = dict;
	_solution = solution;
	pinMoveStep = (int)(rules.getPinMoveStep() * DBU_PER_MACRON);
	minPinPitch = (int)(rules.getMinPinPitch() * DBU_PER_MACRON);
	maxPinPerturbation = (int)(rules.getMaxPinPerturbation() * DBU_PER_MACRON);
	ns = _ns;
}

SAassigner::~SAassigner()
{
}



