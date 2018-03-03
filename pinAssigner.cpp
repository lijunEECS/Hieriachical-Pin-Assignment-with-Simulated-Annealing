#include "pinAssigner.h"
#include "OAHelper.h"
#include <iostream>
using namespace std;

pinAssigner::pinAssigner(oaBlock* topblock, ProjectInputRules rules)
{
	pinMoveStep = (int)(rules.getPinMoveStep() * 2000);
	minPinPitch = (int)(rules.getMinPinPitch() * 2000);
	maxPinPerturbation = (int)(rules.getMaxPinPerturbation() * 2000);
	oaNativeNS _ns;
	ns = _ns;
	oaIter<oaInst> instIter(topblock->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		oaString macroName = getMacroName(inst);
		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		while (oaInstTerm* instTerm = instTermIter.getNext()) {
			if (!isExternalPin(instTerm)) continue;
			macroPin newPin = getMacroPin(instTerm);
			oaTerm* assocTerm = instTerm->getTerm();
			oaPoint relativePos = OAHelper::GetTermPosition(assocTerm);
			if (solution.find(newPin) != solution.end()) {
				assert(solution[newPin] == relativePos);
			}
			else {
				solution[newPin] = relativePos;
				originalPA[newPin] = relativePos;
			}
		}
	}
}

pinAssigner::~pinAssigner() 
{
}

void pinAssigner::printPinAssignment()
{
	cout << "=============================================" << endl;
	cout << "Original pin assignment:" << endl;
	for (solutionIter it = originalPA.begin(); it != originalPA.end(); it++) {
		cout << it->first.macroName << ", " << it->first.instTermName << ", " << it->second.x() << ", " << it->second.y() << endl;
	}

	cout << "=============================================" << endl;
	cout << "Improved pin assignment:" << endl;
	for (solutionIter it = solution.begin(); it != solution.end(); it++) {
		cout << it->first.macroName << ", " << it->first.instTermName << ", " << it->second.x() << ", " << it->second.y() << endl;
	}
}