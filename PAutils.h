#ifndef PAUTILS_H
#define PAUTILS_H

#include <string>
#include "oaDesignDB.h"
using namespace std;
using namespace oa;

struct macroPin
{
	macroPin(oaString& _macroName, oaString& _instTermName) :macroName(_macroName), instTermName(_instTermName)
	{}
	oaString macroName;
	oaString instTermName;
};

typedef map<macroPin, oaPoint> PAsolution;
typedef map<macroPin, oaPoint>::iterator solutionIter;

bool operator < (const macroPin& _l, const macroPin& _r);
bool operator == (const macroPin& _l, const macroPin& _r);

oaString getMacroName(oaInst* inst);

int getHPWL(oaNet* net);


#endif