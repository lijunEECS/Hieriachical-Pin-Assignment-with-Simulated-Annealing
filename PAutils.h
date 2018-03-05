#ifndef PAUTILS_H
#define PAUTILS_H

#include <string>
#include <time.h>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "oaDesignDB.h"
using namespace std;
using namespace oa;

#define ON_BOTTOM 0
#define ON_RIGHT 1
#define ON_TOP 2
#define ON_LEFT 3
#define ON_LEFTBOTTOM 4
#define ON_RIGHTBOTTOM 5
#define ON_RIGHTTOP 6
#define ON_LEFTTOP 7

struct macroPin
{
	macroPin(oaString& _macroName, int _pinLabel) :macroName(_macroName), pinLabel(_pinLabel)
	{}
	oaString macroName;
	int pinLabel;
};

struct instPin
{
	instPin(oaInst* _inst, oaPin* _pin);
	instPin()
	{}
	oaInst* inst;
	oaPin* pin;
	oaString instName;
	oaString pinName;
};

typedef map<macroPin, int> pinMove;
typedef map<instPin, int> pinDict; 
typedef map<instPin, int>::iterator pinDictIter;
typedef map<macroPin, int>::iterator pinMoveIter;

#define DBU_PER_MACRON 2000

bool operator < (const macroPin& _l, const macroPin& _r);
bool operator < (const instPin& _l, const instPin& _r);
bool operator == (const macroPin& _l, const macroPin& _r);
bool operator == (const instPin& _l, const instPin& _r);

oaString getMacroName(oaInst* inst);

macroPin getMacroPin(oaPin* pin, oaInst* inst, pinDict& dict);

int getHPWL(oaNet* net);

oaBox GetAbsolutePinBBox(oaInst* inst, oaPin* pin);

bool isExternalPin(oaInstTerm* instTerm);

bool adjacent(oaBox& box1, oaBox& box2);

void rotate180(oaInst* inst);

void rotate90(oaInst* inst);

void rotate270(oaInst* inst);

void buildPinDict(oaBlock* block, pinDict& dict);

void printDataForMatlab(oaBlock* topBlock, const char* filename);

int onWhichEdge(oaInst* inst, oaPin* pin);

void printPinDict(pinDict& dict);

void getPinBBox(oaPin* pin, oaBox& bbox);

#endif