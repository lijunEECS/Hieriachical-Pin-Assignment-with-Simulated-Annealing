#include "PAutils.h"
#include <iostream>
using namespace std;

bool operator < (const macroPin& _l, const macroPin& _r)
{
	if (_l.macroName < _r.macroName)
		return true;
	if (_l.macroName == _r.macroName)
		return _l.instTermName < _r.instTermName;

	return false;
}

bool operator == (const macroPin& _l, const macroPin& _r)
{
	return (_l.macroName == _r.macroName) && (_l.instTermName == _r.instTermName);
}

oaString getMacroName(oaInst* inst) {
	oaNativeNS ns;
	oaString instName, masterCellName, macroName;
	oaDesign* masterDesign = inst->getMaster();
	masterDesign->getCellName(ns, masterCellName);
	inst->getName(ns, instName);
	int masterCellNameLength = masterCellName.getLength();
	int instNameLength = instName.getLength();
	macroName = masterCellName;
	assert(instName.getLength() > 0);
	macroName[masterCellName.getLength() - instName.getLength() - 1] = NULL;
	return macroName;
}

int getHPWL(oaNet* net) {
	int termNum = (net->getTerms()).getCount();
	int instTermNum = (net->getInstTerms()).getCount();
	int maxX = 0;
	int maxY = 0;
	int minX = INT_MAX;
	int minY = INT_MAX;
	oaPoint tempPoint;
	if (instTermNum == 0 && termNum == 0) {
		return 0;
	}
	if (instTermNum > 0) {
		oaIter<oaInstTerm> instTermIter(net->getInstTerms());
		while (oaInstTerm * instTerm = instTermIter.getNext()) {
			instTerm->getInst()->getOrigin(tempPoint);
			if (tempPoint.x() > maxX) { maxX = tempPoint.x(); }
			if (tempPoint.x() <  minX) { minX = tempPoint.x(); }
			if (tempPoint.y() > maxY) { maxY = tempPoint.y(); }
			if (tempPoint.y() < minY) { minY = tempPoint.y(); }
		}
	}
	if (termNum > 0) {
		oaIter<oaTerm> termIter(net->getTerms());
		while (oaTerm * term = termIter.getNext()) {
			oaIter<oaPin> pinIter(term->getPins());
			while (oaPin *pin = pinIter.getNext()) {
				oaIter<oaPinFig> pinFigIter(pin->getFigs());
				while (oaPinFig * pinFig = pinFigIter.getNext()) {
					oaBox bbox;
					pinFig->getBBox(bbox);
					bbox.getCenter(tempPoint);
					if (tempPoint.x() > maxX) { maxX = tempPoint.x(); }
					if (tempPoint.x() <  minX) { minX = tempPoint.x(); }
					if (tempPoint.y() > maxY) { maxY = tempPoint.y(); }
					if (tempPoint.y() < minY) { minY = tempPoint.y(); }
				}
			}
		}
	}
	int tempHPWL = maxX - minX + maxY - minY;
	return tempHPWL;
}