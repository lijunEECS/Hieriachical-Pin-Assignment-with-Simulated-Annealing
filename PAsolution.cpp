#include "PAsolution.h"

#include <iostream>
using namespace std;

PAsolution::PAsolution()
{
}

PAsolution::PAsolution(oaBlock* topblock, pinDict& dict, oaNativeNS _ns)
{
	ns = _ns;
	_pinDict = dict;
	oaIter<oaInst> instIter(topblock->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		oaString macroName = getMacroName(inst);
		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		//cout << '*' << endl;
		while (oaInstTerm* instTerm = instTermIter.getNext()) {
			if (!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			while (oaPin* temp_pin = pinIter.getNext())
			{
				//cout << '.' << endl;
				macroPin temp = getMacroPin(temp_pin, inst, _pinDict);
				_pinMove[temp] = 0;
				//cout << '+' << endl;
			}
		}
		int rotateIndex;
		oaOrient orient = inst->getOrient();
		switch (orient) {
		case oacR0:
			rotateIndex = NOROTATE;
			break;
		case oacR90:
			rotateIndex = ROTATE90;
			break;
		case oacR180:
			rotateIndex = ROTATE180;
			break;
		case oacR270:
			rotateIndex = ROTATE270;
			break;
		}

		_rotation[inst] = rotateIndex;
	}
}

PAsolution::PAsolution(PAsolution& _ps, bool smallPerturbation, int perturbationRange)
{
	_pinMove = _ps._pinMove;
	_rotation = _ps._rotation;
	if (smallPerturbation) {
		for (pinMoveIter it = _pinMove.begin(); it != _pinMove.end(); it++)
		{
			it->second += random(-perturbationRange, perturbationRange);
		}
		for (map<oaInst*, int>::iterator it = _rotation.begin(); it != _rotation.end(); it++) 
		{
			it->second += random(NOROTATE, ROTATE270);
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
		cout << it->first.macroName << ", " << it->first.pinLabel << ", " << it->second << endl;
	}
	for (map<oaInst*, int>::iterator it = _rotation.begin(); it != _rotation.end(); it++) {
		oaString masterCellName;
		oaInst* inst = it->first;
		oaDesign* master = inst->getMaster();
		master->getCellName(ns, masterCellName);
		cout << masterCellName << ", " << it->second << endl;
	}
}