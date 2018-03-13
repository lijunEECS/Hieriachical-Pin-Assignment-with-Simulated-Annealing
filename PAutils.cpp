#include "PAutils.h"
#include <iostream>
using namespace std;
using namespace oa;
oaNativeNS _ns;


/* instPin is a pair of oaInst* and oaPin*. It is used to
   find the pin label.
*/
instPin::instPin(oaInst* _inst, oaPin* _pin)
{
	inst = _inst;
	pin = _pin;
	_inst->getMaster()->getCellName(_ns, instName);
	_pin->getName(pinName);
}

/* The struct macroPin is used to index pin location.
*  For hierarchical pin assignment, pin location should
*  only depend on the macro name and pin label.
*  Overload < so that macroPin can be stored by balance binary tree.
*  Notice that in practice, the pin label field can be either used 
*  to store a pin label or a index of legal position.
*/
bool operator < (const macroPin& _l, const macroPin& _r)
{
	if (_l.macroName < _r.macroName)
		return true;
	if (_l.macroName == _r.macroName)
		return _l.pinLabel < _r.pinLabel;

	return false;
}

bool operator == (const macroPin& _l, const macroPin& _r)
{
	return (_l.macroName == _r.macroName) && (_l.pinLabel == _r.pinLabel);
}

bool operator < (const instPin& _l, const instPin& _r)
{
	if (_l.instName != _r.instName)
		return _l.instName < _r.instName;
	if(_l.pinName != _r.pinName)
		return _l.pinName < _r.pinName;
	return false;
}

bool operator == (const instPin& _l, const instPin& _r)
{
	return (_l.instName == _r.instName && _l.pinName == _r.pinName);
}

/* Since we save a unique copy for each inst before executing
pin assignment, each inst will have unique master cell name.
This function parse master cell name to get the original master
cell name (macro name).
*/
oaString getMacroName(oaInst* inst) {
	oaString instName, masterCellName, macroName;
	oaDesign* masterDesign = inst->getMaster();
	masterDesign->getCellName(_ns, masterCellName);
	inst->getName(_ns, instName);
	int masterCellNameLength = masterCellName.getLength();
	int instNameLength = instName.getLength();
	macroName = masterCellName;
	assert(instName.getLength() > 0);
	macroName[masterCellName.getLength() - instName.getLength() - 1] = NULL;
	return macroName;
}

/* This function fetch a pin and an inst, then cast a corresponding
   macroPin object to return. 
*/
macroPin getMacroPin(oaPin* pin, oaInst* inst, pinDict& dict)
{
	oaString macroName = getMacroName(inst);
	instPin _instPin(inst, pin);
	assert(dict.find(_instPin) != dict.end());
	//if (dict.find(_instPin) == dict.end())
	//{
		//printPinDict(dict);
		// oaBox bbox;
		// oaIter<oaPinFig> pinFigIter(pin->getFigs());
		// oaPinFig* pinFig = pinFigIter.getNext();
		// pinFig->getBBox(bbox);
		// oaString masterCellName, pinName;
		// inst->getMaster()->getCellName(_ns, masterCellName);
		// pin->getName(pinName);
		// cout << "=========================================" << endl;
		// cout<< masterCellName<<", "<<pinName<<", "<< bbox.left() << ", " << bbox.right() << ", " << bbox.top() << ", " << bbox.bottom() << endl;
		// cout << "=========================================" << endl;
	// 	assert(dict.find(_instPin) != dict.end());
	// }
	int pinLabel = dict[_instPin];
	return macroPin(macroName, pinLabel);
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
	center.x() += center.y();
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
	int maxX = INT_MIN;
	int maxY = INT_MIN;
	int minX = INT_MAX;
	int minY = INT_MAX;
	oaPoint tempPoint;
	if (instTermNum == 0 && termNum == 0) {
		return 0;
	}
	if (instTermNum > 0) {
		oaIter<oaInstTerm> instTermIter(net->getInstTerms());
		while (oaInstTerm * instTerm = instTermIter.getNext()) {
			oaTerm* term = instTerm->getTerm();
			oaIter<oaPin> pinIter(term->getPins());
			oaPin *pin = pinIter.getNext();
			oaIter<oaPinFig> pinFigIter(pin->getFigs());
			oaPinFig * pinFig = pinFigIter.getNext();
			oaBox bbox;
			pinFig->getBBox(bbox);
			bbox.getCenter(tempPoint);
			if (tempPoint.x() > maxX) { maxX = tempPoint.x(); }
			if (tempPoint.x() < minX) { minX = tempPoint.x(); }
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
					if (tempPoint.x() < minX) { minX = tempPoint.x(); }
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
			//if (!isExternalPin(instTerm)) continue;
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


/* a pin dictionary is a map from instPin (which contains
   basically an oaInst* and a pin*) to the pin's label.
   Use this data structure to find the pin label fleetly.
*/
void buildPinDict(oaBlock* block, pinDict& dict)
{
	assert(dict.empty());
	set<oaString> labeledMacro;
	oaIter<oaInst> instIter(block->getInsts());
	while (oaInst* inst = instIter.getNext())
	{
		int tempLabel = 0;
		oaString macroName = getMacroName(inst);
		
		if (labeledMacro.find(macroName) == labeledMacro.end()) {
			labeledMacro.insert(macroName);
			oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
			while (oaInstTerm* instTerm = instTermIter.getNext())
			{
				if (!isExternalPin(instTerm)) continue;
				oaTerm* assocTerm = instTerm->getTerm();
				oaIter<oaPin> pinIter(assocTerm->getPins());
				while (oaPin* pin = pinIter.getNext())
				{
					//oaString pinName;
					//instTerm->getTermName(_ns, pinName);
					//cout << macroName << ", " << pinName << ", " << tempLabel << endl;
					pair<instPin, int> tempP;
					tempP = make_pair(instPin(inst,pin), tempLabel);
					pair<pinDictIter, bool> probe = dict.insert(tempP);
					// if (!probe.second)
					// {
					// 	oaString masterCellName;
					// 	inst->getMaster()->getCellName(_ns, masterCellName);
					// 	cout << "==============================================" << endl;
					// 	cout << "pinDict building error." << endl;
					// 	cout << masterCellName << endl;
					// 	cout << tempLabel << ", " << dict[instPin(inst, pin)] << endl;
					// 	cout << "==============================================" << endl;
					// }
					assert(probe.second);
					tempLabel++;
				}
			}
		}
		else {
			oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
			while (oaInstTerm* instTerm = instTermIter.getNext())
			{
				if (!isExternalPin(instTerm)) continue;
				oaTerm* assocTerm = instTerm->getTerm();
				oaIter<oaPin> pinIter(assocTerm->getPins());
				while (oaPin* pin = pinIter.getNext())
				{
					for (pinDictIter it = dict.begin(); it != dict.end(); it++)
					{
						oaString macroName2 = getMacroName(it->first.inst);
						if (macroName != macroName2) continue;

						oaBox pinBBox1, pinBBox2;
						oaIter<oaPinFig> pinFigIter1(it->first.pin->getFigs());
						oaPinFig* pinFig1 = pinFigIter1.getNext();
						pinFig1->getBBox(pinBBox1);
						oaIter<oaPinFig> pinFigIter2(pin->getFigs());
						oaPinFig* pinFig2 = pinFigIter2.getNext();
						pinFig2->getBBox(pinBBox2);
						
						if (pinBBox1 == pinBBox2) {
							tempLabel = it->second;
							/*oaString pinName;
							instTerm->getTermName(_ns, pinName);
							cout << macroName << "__, " << pinName << ", " << tempLabel << endl;*/
							pair<instPin, int> tempP;
							tempP = make_pair(instPin(inst,pin), tempLabel);
							pair<pinDictIter, bool> probe = dict.insert(tempP);
							// if (!probe.second)
							// {
							// 	oaString masterCellName, masterCellName2;
							// 	it->first.inst->getMaster()->getCellName(_ns, masterCellName2);
							// 	inst->getMaster()->getCellName(_ns, masterCellName);
							// 	cout << "==============================================" << endl;
							// 	cout << "pinDict building error." << endl;
							// 	cout << masterCellName <<", "<<masterCellName2<< endl;
							// 	cout << pinBBox1.left() << ", " << pinBBox1.right() << ", " << pinBBox1.top() << ", " << pinBBox1.bottom() << endl;
							// 	cout << pinBBox2.left() << ", " << pinBBox2.right() << ", " << pinBBox2.top() << ", " << pinBBox2.bottom() << endl;
							// 	cout << tempLabel << ", " << dict[instPin(inst,pin)] << endl;
							// 	cout << "==============================================" << endl;
							// }
							assert(probe.second);
							break;
						}
					}
				}
			}
		}
	}
}

/* This function print out the content of the pin dictionary
   to a txt file. The file name is specified by filename parameter.
*/
void printPinDict(pinDict& dict, const char* filename)
{
	fstream out(filename, ios::out);
	cout << "=============================================" << endl;
	cout << "Pin dictionary:" << endl;
	for (pinDictIter it = dict.begin(); it != dict.end(); it++)
	{
		oaBox bbox;
		oaIter<oaPinFig> pinFigIter(it->first.pin->getFigs());
		oaPinFig* pinFig = pinFigIter.getNext();
		pinFig->getBBox(bbox);
		oaString termName;
		it->first.pin->getTerm()->getName(_ns, termName);
		cout << it->first.instName << ", " <<termName<<", "<<it->first.pinName<<", "<< "pin" << it->second << ", " << bbox.left() << ", " << bbox.right() << ", " << bbox.top() << ", " << bbox.bottom() << endl;
		out<<bbox.left() << ", " << bbox.right() << ", " << bbox.top() << ", " << bbox.bottom() << endl;
	}
	cout << "=============================================" << endl;
	out.close();
}


/* Given a pin and its owner inst, this function return an integer,
   indicating on which edge this pin is.
*/
int onWhichEdge(oaInst* inst, oaPin* pin)
{
	oaBox instBBox;
	inst->getBBox(instBBox);
	int instHeight = instBBox.top() - instBBox.bottom();
	int instWidth = instBBox.right() - instBBox.left();

	oaOrient orient = inst->getOrient();
	oaOrient inverseOrient = orient.getRelativeOrient(oacR0);

	if (orient == oacR90 || orient == oacR270)
	{
		int temp = instHeight;
		instHeight = instWidth;
		instWidth = temp;
	}

	oaIter<oaPinFig> pinFigIter(pin->getFigs());
	oaPinFig* pinFig = pinFigIter.getNext();
	oaBox pinBBox;
	pinFig->getBBox(pinBBox);
	if (pinBBox.bottom() == 0)
		return ON_BOTTOM;
	if (pinBBox.right() == instWidth)
		return ON_RIGHT;
	if (pinBBox.top() == instHeight)
		return ON_TOP;
	if (pinBBox.left() == 0)
		return ON_LEFT;
	bool onBoundary = false;
	assert(onBoundary);
	return -1;
}

/* Function write the bounding box of the given pin into bbox.
*/
void getPinBBox(oaPin* pin, oaBox& bbox)
{
	oaIter<oaPinFig> pinFigIter(pin->getFigs());
	oaPinFig* pinFig = pinFigIter.getNext();
	pinFig->getBBox(bbox);
}
