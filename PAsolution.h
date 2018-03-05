#ifndef PASOLUTION_H
#define	PASOLUTION_H

#include <string>
#include <map>
#include "oaDesignDB.h"
#include "PAutils.h"

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
	PAsolution(oaBlock* topblock, pinDict& dict, oaNativeNS _ns);
	PAsolution(PAsolution& _ps, bool smallPerturbation, int perturbationRange);
	virtual ~PAsolution();
	void printSolution();
	oaNativeNS ns;
	pinDict _pinDict;
	pinMove _pinMove;
	map<oaInst*, int> _rotation;
private:
};

#endif
