#include "SAassigner.h"
#include <iostream>
using namespace std;

SAassigner::SAassigner(PAsolution& solution, ProjectInputRules& rules, pinDict& dict, oaNativeNS _ns)
{
	_pinDict = dict;
	_solution = solution;
	pinMoveStep = (int)(rules.getPinMoveStep() * DBU_PER_MACRON);
	minPinPitch = (int)(rules.getMinPinPitch() * DBU_PER_MACRON);
	maxPinPerturbation = (int)(rules.getMaxPinPerturbation() * DBU_PER_MACRON);
	ns = _ns;
}

SAassigner::~SAassigner()
{
}

void SAassigner::applySolution(oaBlock* topBlock)
{
	oaIter<oaInst> instIter(topBlock->getInsts());
	while (oaInst* inst = instIter.getNext())
	{
		int rotateIndex = _solution._rotation[inst];
		switch (rotateIndex) {
		case ROTATE90:
			rotate90(inst);
			break;
		case ROTATE180:
			rotate180(inst);
			break;
		case ROTATE270:
			rotate270(inst);
			break;
		}
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

		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		while (oaInstTerm* instTerm = instTermIter.getNext())
		{
			if(!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			while (oaPin* pin = pinIter.getNext())
			{
				macroPin _macroPin = getMacroPin(pin, inst, _pinDict);
				int stepNum = _solution._pinMove[_macroPin];
				if (stepNum == 0) continue;
				int moveDBU = stepNum * pinMoveStep;
				oaIter<oaPinFig> pinFigIter(pin->getFigs());
				oaPinFig* pinFig = pinFigIter.getNext();
				oaBox pinBBox;
				pinFig->getBBox(pinBBox);
				int side;
				if (pinBBox.bottom() == 0)
				{
					if (pinBBox.left() == 0)
						side = ON_LEFTBOTTOM;
					else if (pinBBox.right() == instWidth)
						side = ON_RIGHTBOTTOM;
					else
						side = ON_BOTTOM;
				}
				else if (pinBBox.top() == instHeight)
				{
					if (pinBBox.left() == 0)
						side = ON_LEFTTOP;
					else if (pinBBox.right() == instWidth)
						side = ON_RIGHTTOP;
					else
						side = ON_TOP;
				}
				else if (pinBBox.left() == 0)
				{
					side = ON_LEFT;
				}
				else if (pinBBox.right() == instWidth)
				{
					side = ON_RIGHT;
				}
				else {
					bool pinOnBoundary = false;
					cout << "=========================================" << endl;
					cout << "This pin is not on the boundary." << endl;
					cout << pinBBox.left() << ", " << pinBBox.right() << ", " << pinBBox.top() << ", " << pinBBox.bottom() << endl;
					cout << instHeight << ", " << instWidth << endl;
					cout << "=========================================" << endl;
					assert(pinOnBoundary);
				}
				movePin(pinFig, pinBBox, side, instHeight, instWidth, moveDBU);
			}
		}


	}
}

