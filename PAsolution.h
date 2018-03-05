#ifndef PASOLUTION_H
#define	PASOLUTION_H

#include <string>
#include <map>
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
	PAsolution(oaBlock* topblock, pinDict& dict, ProjectInputRules& rules, oaNativeNS _ns);
	PAsolution(PAsolution& _ps);
	virtual ~PAsolution();
	void printSolution();
	void pertubate(int perturbationRange);
	oaNativeNS ns;
	pinDict _pinDict;
	pinMove _pinPos;               //应该把合法位置编号，记录每一个pin的位置序号而不是移动的step数目！
	map<oaInst*, int> _rotation;
	map<oaInst*, int> _maxPos;
	map<oaInst*, int> _xPosNum;
	map<oaInst*, int> _yPosNum;
	map<oaInst*, int> _minX;
	map<oaInst*, int> _minY;
	map<oaString, int> _macroMaxPos;
	int pinMoveStep;
	int minPinPitch;
	void applySolution(oaBlock* topblock);
private:
};

#endif
