#ifndef PASOLUTION_H
#define	PASOLUTION_H

#include <string>
#include <map>
#include <set>
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
	PAsolution(PAsolution& _ps1, PAsolution& _ps2);
	virtual ~PAsolution();
	void printSolution();
	void pertubate(int perturbationRange);
	void legalizePinPos();               
	static void initializeStaticMember(oaBlock* topblock, pinDict& dict, ProjectInputRules& rules, oaNativeNS _ns);
	static oaNativeNS ns;
	static pinDict _pinDict;
	
	static int pinWidth;
	static int pinHeight;
	static float alpha;
	static float beta;
	static float gamma;

	static std::map<oaInst*, int> _maxPos;
	static std::map<oaInst*, int> _xPosNum;
	static std::map<oaInst*, int> _yPosNum;
	static std::map<oaInst*, int> _minX;
	static std::map<oaInst*, int> _minY;
	static std::map<oaString, int> _macroMaxPos;
	static std::map<macroPin, int> _originalPinPos;

    static std::map<oaInst*, int> _instHeight;
    static std::map<oaInst*, int> _instWidth;

    static std::map<macroPin, oaPoint> _relativePos;

	static int pinMoveStep;
	static int minPinPitch;
    static int maxPerturbation;

	std::map<macroPin, int> _pinPos;
	std::map<oaInst*, int> _rotation;
	void applySolution(oaBlock* topblock);
	float evaluate(oaBlock* block);
private:
};



#endif
