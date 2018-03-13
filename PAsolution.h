#ifndef PASOLUTION_H
#define	PASOLUTION_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <math.h>
#include <climits>
#include <stdlib.h>
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
	static void printStaticData();
	void pertubate(int perturbationRange);
	void randomRotate();
	bool legalizePinPos();               
	static void initializeStaticMember(oaBlock* topblock, pinDict& dict, ProjectInputRules& rules, oaNativeNS _ns);

	// All PAsolution objects should share global name space.
	static oaNativeNS ns;

	// All PAsolution objects should use the same instPin-to-label map.
	static pinDict _pinDict;
	
	// Within the same design, the shape of pin is expected to be fixed.
	// pinWidth and pinHeight record the fixed shape of pinFig.
	static int pinWidth;
	static int pinHeight;

	// If satisfying all constraints(a,b,c,d, pmin, pmax = 1) and leaving
	// out pin copy. The metric will only depend on max wirelength, sum of
	// all wirelength and average pin perturbation. We normalize it as
	// X = alpha * maxWireLength + beta * totalWireLength + gamma * avgPinPerturbation
	// where alpha = totalWireLength / maxWireLength (without optimization)
	//       beta  = 2.0
	//       gamma = 2.0 * totalWireLength / avgHalfMacroPerimeter (without optimizaiton)
	// The real metric is estimated as 7 - X +3 * e, where e is related to runtime.
	static float alpha;
	static float beta;
	static float gamma;

    // _maxPos records how many legal pin positions each inst can have.
	static std::map<oaInst*, int> _maxPos;

	// _xPosNum records how many legal pin positions on a horizontal edge 
	// each inst can have.
	static std::map<oaInst*, int> _xPosNum;

	// y_PosNum records how many legal pin positions on a vertical edge
	// each inst can have.
	static std::map<oaInst*, int> _yPosNum;

	// _minX records the x-axis offset from inst boundary of the most left legal pin
	static std::map<oaInst*, int> _minX;

	// _minY records the y-axis offset from inst boundary of the most bottom legal pin
	static std::map<oaInst*, int> _minY;

	// _macroMaxPos records the number of legal pin positions. It's a map from macroName
	// to the number of legal pin positions.
	static std::map<oaString, int> _macroMaxPos;

	// We labeled each legal pin position so that a pin assignment solution can be
	// stored as a map from macroPin to int.
	// _originalPinPos records the original pin assignment solution.
	// It is used to calculate the pin perturbation of current solution.
	static std::map<macroPin, int> _originalPinPos;

	// _instHeight and _instWidth record the height and width of each inst.
    static std::map<oaInst*, int> _instHeight;
    static std::map<oaInst*, int> _instWidth;

    // _relativePos map the index of a legal pin positon along with the 
    // pin owner macro to a relative pin center point.
    // The owner macro name and position index are stored in macroPin obeject.
    // Refer to comments of macroPin for more detail.
    static std::map<macroPin, oaPoint> _relativePos;

    // The following three parameters are rule related.
    // pinMoveStep indicates how many DBUs a single legal pin move has.
	static int pinMoveStep;

	// minPinPitch records the pin pitch in DBUs.
	static int minPinPitch;

	// maxPerturbation records max pin perturbation in DBUs.
    static int maxPerturbation;

    // _pinPos records the relative position of each pin. All legal position 
    // are labeled so that we can map macroPin to an int to indicate its position.
	std::map<macroPin, int> _pinPos;

	// _rotation records the rotation transformation that this solution will apply
	// to each inst.
	std::map<oaInst*, int> _rotation;
	void applySolution(oaBlock* topblock);
	bool checkPerturbation();
	float evaluate(oaBlock* block);
private:
};



#endif
