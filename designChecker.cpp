#include "designChecker.h"
#include <iostream>
#include <map>
using namespace std;

typedef set<myBox> pinBbox;
typedef map<oaDesign*, oaInst*> masterDic;

myBox::myBox(oaBox box) {
	bbox = box;
}

int myBox::getLeft()const
{
	return bbox.left();
}

int myBox::getRight()const
{
	return bbox.right();
}

int myBox::getTop()const
{
	return bbox.top();
}

int myBox::getBottom()const
{
	return bbox.bottom();
}

bool myBox::operator<(const myBox& right)const
{
	if (this->getLeft() != right.getLeft())
	{
		return this->getLeft() < right.getLeft();
	}
	else if (this->getRight() != right.getRight())
	{
		return this->getRight() < right.getRight();
	}
	else if (this->getTop() != right.getTop())
	{
		return this->getTop() < right.getTop();
	}
	else if (this->getBottom() != right.getBottom())
	{
		return this->getBottom() < right.getBottom();
	}
	return false;
}

myBox::~myBox() 
{
}

designChecker::designChecker() 
{
}

designChecker::~designChecker()
{
}

bool designChecker::checkHierarchicalAssignment(oaDesign* design)
{
	masterDic dic1;
	oaBlock* block = design->getTopBlock();
	oaIter<oaInst> instIter(block->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		oaDesign* instMaster = inst->getMaster();
		if (!dic1[instMaster]) {
			dic1[instMaster] = inst;
			continue;
		}
		else {
			oaInst* refInst = dic1[instMaster];

			if (refInst->getInstTerms().getCount() != inst->getInstTerms().getCount()) {
				return false;
			}

			pinBbox refPins;
			oaIter<oaInstTerm> instTermIter(refInst->getInstTerms());
			while (oaInstTerm* instTerm = instTermIter.getNext()) {
				oaTerm* tempTerm = instTerm->getTerm();
			    	oaIter<oaPin> pinIterator(tempTerm->getPins());
				oaIter<oaPinFig> pinFigIterator(pinIterator.getNext()->getFigs());
				oaPinFig* pinFig = pinFigIterator.getNext();
				oaBox tempBbox;
				pinFig->getBBox(tempBbox);
				myBox bbox(tempBbox);
				refPins.insert(bbox);
			}

			oaIter<oaInstTerm> currentInstTermIter(inst->getInstTerms());
			while (oaInstTerm* instTerm = currentInstTermIter.getNext()) {
				oaTerm* tempTerm = instTerm->getTerm();
			    	oaIter<oaPin> pinIterator(tempTerm->getPins());
				oaIter<oaPinFig> pinFigIterator(pinIterator.getNext()->getFigs());
				oaPinFig* pinFig = pinFigIterator.getNext();
				oaBox tempBbox;
				pinFig->getBBox(tempBbox);
				myBox bbox(tempBbox);
				if (refPins.find(bbox) == refPins.end()) {
					return false;
				}
			}
		}
	}
	return true;
}

