#ifndef PAUTILS_H
#define PAUTILS_H

#include <string>
#include <time.h>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "oaDesignDB.h"
using namespace std;
using namespace oa;

struct macroPin
{
	macroPin(oaString& _macroName, oaString& _PinName) :macroName(_macroName), PinName(_PinName)
	{}
	oaString macroName;
	oaString PinName;
};

typedef map<macroPin, int> pinMove;
typedef map<macroPin, int>::iterator pinMoveIter;

bool operator < (const macroPin& _l, const macroPin& _r);
bool operator == (const macroPin& _l, const macroPin& _r);

oaString getMacroName(oaInst* inst);

macroPin getMacroPin(oaPin* pin, oaInst* inst);

int getHPWL(oaNet* net);

oaBox GetAbsoluteInstTermBBox(oaInst* inst, oaPin* pin);

bool isExternalPin(oaInstTerm* instTerm);

bool adjacent(oaBox& box1, oaBox& box2);

void rotate180(oaInst* inst);

void rotate90(oaInst* inst);

void rotate270(oaInst* inst);

void printDataForMatlab(oaBlock* topBlock, const char* filename);

#endif