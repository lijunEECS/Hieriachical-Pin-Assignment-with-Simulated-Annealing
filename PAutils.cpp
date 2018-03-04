#include "PAutils.h"
#include <iostream>
using namespace std;


/* The struct macroPin is used to index pin location
*  For hierarchical pin assignment, pin location should
*  only depend on the macro name and pin name.
*  Overload < so that macroPin can be used as key for STL map
*/
bool operator < (const macroPin& _l, const macroPin& _r)
{
	if (_l.macroName < _r.macroName)
		return true;
	if (_l.macroName == _r.macroName)
		return _l.PinName < _r.PinName;

	return false;
}

bool operator == (const macroPin& _l, const macroPin& _r)
{
	return (_l.macroName == _r.macroName) && (_l.PinName == _r.PinName);
}

/* Since we save a unique copy for each inst before executing
pin assignment, each inst will have unique master cell name.
This function parse master cell name to get the original master
cell name (macro name).
*/
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


/* This function fetch an instTerm then use its owner inst's 
macro name and the instTerm name to create a macroPin struct.
*/
macroPin getMacroPin(oaPin* pin, oaInst* inst)
{
	oaString macroName = getMacroName(inst);
	oaString PinName;
	pin->getName(PinName);
	return macroPin(macroName, PinName);
}

/* This function rotate the inst 180 degree counterclockwise.
*/
void rotate180(oaInst* inst)
{
	oaPoint instOrigin;
	oaPoint center;
	oaBox bbox;
	inst->getBBox(bbox);
	bbox.getCenter(center);
	center.x() = 2 * center.x();
	center.y() = 2 * center.y();
	oaTransform trans(center, oacR180);
	inst->move(trans);
}

/* This function rotate the inst 90 degree counterclockwise.
*/
void rotate90(oaInst* inst)
{
	oaPoint instOrigin;
	oaPoint center;
	oaBox bbox;
	inst->getBBox(bbox);
	bbox.getCenter(center);
	int temp = center.x();
	center.x() +=  center.y();
	center.y() -= temp;
	oaTransform trans(center, oacR90);
	inst->move(trans);
}

/* This function rotate the inst 270 degree counterclockwise.
*/
void rotate270(oaInst* inst)
{
	oaPoint instOrigin;
	oaPoint center;
	oaBox bbox;
	inst->getBBox(bbox);
	bbox.getCenter(center);
	int temp = center.x();
	center.x() -= center.y();
	center.y() += temp;
	oaTransform trans(center, oacR270);
	inst->move(trans);
}

/* This function compute the HPWL for a given net.
*/
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

/* The bounding box of a pinFig is relative to its owner inst.
This function fetch a pin and its owner inst and return the bounding
box w.r.t. absolute cordinates.
*/
oaBox GetAbsolutePinBBox(oaInst* inst, oaPin* pin)
{
	oaIter<oaPinFig> pinFigIter(pin->getFigs());
	oaPinFig* pinFig = pinFigIter.getNext();
	oaBox bbox;
	pinFig->getBBox(bbox);
	oaPoint instOrigin;
	inst->getOrigin(instOrigin);
	oaOrient orient = inst->getOrient();

	//Rotate relative position by orient
	oaTransform trans = oaTransform(instOrigin, orient);
	bbox.transform(trans);

	return bbox;
}

/* This function determine if two oaBoxes are adjacent.
It is used to determine if a pin is external pin.
*/
bool adjacent(oaBox& box1, oaBox& box2) 
{
	if (box1.left() == box2.left())
		return true;
	if (box1.left() == box2.right())
		return true;
	if (box1.right() == box2.left())
		return true;
	if (box1.right() == box2.right())
		return true;
	if (box1.top() == box2.top())
		return true;
	if (box1.top() == box2.bottom())
		return true;
	if (box1.bottom() == box2.top())
		return true;
	if (box1.bottom() == box2.bottom())
		return true;
	return false;
}

/* The macros may have internal pins (VDD and VSS).
This function determine if a instTerm is external.
It is used to exclude internal pins for assignment.
*/
bool isExternalPin(oaInstTerm* instTerm)
{
	oaInst* inst = instTerm->getInst();
	oaBox instBBox;
	inst->getBBox(instBBox);
	oaTerm* assocTerm = instTerm->getTerm();
	oaIter<oaPin> pinIter(assocTerm->getPins());
	oaPin* pin = pinIter.getNext();
	oaBox pinBBox = GetAbsolutePinBBox(inst, pin);;
	return adjacent(instBBox, pinBBox);
}

/* This function print the bounding boxes of insts and
pins to a file. Then we can use matlab to analyse and visualize
the data for debug purposes.
*/
void printDataForMatlab(oaBlock* topBlock, const char* filename)
{
	int left = INT_MAX;
	int right = INT_MIN;
	int top = INT_MIN;
	int bottom = INT_MAX;

	fstream out(filename, ios::out);
	oaIter<oaInst> instIter(topBlock->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		oaBox BBox;
		inst->getBBox(BBox);

		if (BBox.left() < left) { left = BBox.left(); }
		if (BBox.right() > right) { right = BBox.right(); }
		if (BBox.top() > top) { top = BBox.top(); }
		if (BBox.bottom() < bottom) { bottom = BBox.bottom(); }
		
		out << BBox.left() << ',' << BBox.right() << ',' << BBox.top() << ',' << BBox.bottom() << ',' << 1 << endl;
		
		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		while (oaInstTerm* instTerm = instTermIter.getNext()) {
			if (!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaTermType termType = assocTerm->getTermType();
			oaString termTypeName = termType.getName();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			while (oaPin* pin = pinIter.getNext()) {
				BBox = GetAbsolutePinBBox(inst, pin);
				out << BBox.left() << ',' << BBox.right() << ',' << BBox.top() << ',' << BBox.bottom() << ',' << 2 << endl;
			}

		}
	}
	out << left << ',' << right << ',' << top << ',' << bottom << ',' << 0 << endl;
	out.close();
}