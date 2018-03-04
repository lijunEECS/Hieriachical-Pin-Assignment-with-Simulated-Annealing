#include "PAsolution.h"

#include <iostream>
using namespace std;

PAsolution::PAsolution(oaBlock* topblock)
{
	oaNativeNS _ns;
	ns = _ns;
	oaIter<oaInst> instIter(topblock->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		oaString macroName = getMacroName(inst);
		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		while (oaInstTerm* instTerm = instTermIter.getNext()) {
			if (!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			while (oaPin* temp_pin = pinIter.getNext())
			{
				macroPin temp = getMacroPin(temp_pin, inst);
				if (_pinMove.find(temp) != _pinMove.end()) {
					assert(_pinMove[temp] == 0);
				}
				else {
					_pinMove[temp] = 0;
				}
			}
		}
		if (_rotation.find(inst) != _rotation.end()) {
			assert(_rotation[inst] == NOROTATE);
		}
		else {
			_rotation[inst] = NOROTATE;
		}
	}
}

PAsolution::~PAsolution()
{
}

void PAsolution::printSolution()
{
	cout << "=============================================" << endl;
	cout << "Pin assignment solution:" << endl;
	for (pinMoveIter it = _pinMove.begin(); it != _pinMove.end(); it++) {
		cout << it->first.macroName << ", " << it->first.PinName << ", " << it->second << endl;
	}
	for (map<oaInst*, int>::iterator it = _rotation.begin(); it != _rotation.end(); it++) {
		oaString masterCellName;
		oaInst* inst = it->first;
		oaDesign* master = inst->getMaster();
		master->getCellName(ns, masterCellName);
		cout << masterCellName << ", " << it->second << endl;
	}
}
