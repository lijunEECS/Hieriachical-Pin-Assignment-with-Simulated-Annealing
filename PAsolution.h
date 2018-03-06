#ifndef PASOLUTION_H
#define	PASOLUTION_H

#include <string>
#include <map>
#include <math.h>
#include <climits>
#include "oaDesignDB.h"
#include "PAutils.h"
#include "OAHelper.h"
#include "ProjectInputRules.h"

using namespace std;
using namespace oa;

#define NOROTATE 0
#define ROTATE90 1
#define ROTATE180 2
#define ROTATE270 3
#define random(a,b) (rand()%(b-a+1)+a)

class PAsolution
{
public:
	PAsolution();
	PAsolution(oaBlock* topblock);
	PAsolution(PAsolution& _ps);
	virtual ~PAsolution();
	void printSolution();
	void pertubate(int perturbationRange);
	void legalizePinPos();               
	static void initializeStaticMember(oaBlock* topblock, pinDict& dict, ProjectInputRules& rules, oaNativeNS _ns);
	static oaNativeNS ns;
	static pinDict _pinDict;
	static map<oaInst*, int> _maxPos;
	static map<oaInst*, int> _xPosNum;
	static map<oaInst*, int> _yPosNum;
	static map<oaInst*, int> _minX;
	static map<oaInst*, int> _minY;
	static map<oaString, int> _macroMaxPos;
	static int pinMoveStep;
	static int minPinPitch;
	pinMove _pinPos;
	map<oaInst*, int> _rotation;
	void applySolution(oaBlock* topblock);
private:
};



#endif
