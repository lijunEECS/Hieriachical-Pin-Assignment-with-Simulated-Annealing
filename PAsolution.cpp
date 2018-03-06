#include "PAsolution.h"

#include <iostream>
using namespace std;

void PAsolution::initializeStaticMember(oaBlock* topblock, pinDict& dict, ProjectInputRules& rules, oaNativeNS _ns)
{
	ns = _ns;
	_pinDict = dict;
	pinMoveStep = (int)(rules.getPinMoveStep() * DBU_PER_MACRON);
	minPinPitch = (int)(rules.getMinPinPitch() * DBU_PER_MACRON);
	oaIter<oaInst> instIter(topblock->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		int maxCenterX, minCenterX, maxCenterY, minCenterY;
		bool findHorizon = false;
		bool findVertical = false;

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
		while (oaInstTerm* instTerm = instTermIter.getNext()) {
			if (findHorizon && findVertical) break;
			if (!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			oaIter<oaPinFig> pinFigIter(pinIter.getNext()->getFigs());
			oaPinFig* pinFig = pinFigIter.getNext();
			oaBox pinBBox;
			pinFig->getBBox(pinBBox);
			oaPoint pinCenter;
			pinBBox.getCenter(pinCenter);
			if (pinBBox.bottom() == 0 || pinBBox.top() == instHeight)
			{
				findHorizon = true;
				minCenterX = pinCenter.x() % pinMoveStep;
				maxCenterX = instWidth - ((instWidth - pinCenter.x()) % pinMoveStep);
			}
			else if (pinBBox.right() == instWidth || pinBBox.left() == 0)
			{
				findVertical = true;
				minCenterY = pinCenter.y() % pinMoveStep;
				maxCenterY = instHeight - ((instHeight - pinCenter.y()) % pinMoveStep);
			}
		}
		int temp_maxPos = (((maxCenterX - minCenterX) + (maxCenterY - minCenterY)) / pinMoveStep + 2) * 2;
		oaString macroName = getMacroName(inst);
		_macroMaxPos[macroName] = temp_maxPos;
		_maxPos[inst] = temp_maxPos;
		_xPosNum[inst] = (maxCenterX - minCenterX) / pinMoveStep + 1;
		_yPosNum[inst] = (maxCenterY - minCenterY) / pinMoveStep + 1;
		_minX[inst] = minCenterX;
		_minY[inst] = minCenterY;
	}
}

PAsolution::PAsolution()
{
}

PAsolution::PAsolution(oaBlock* topblock)
{
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
				int side = onWhichEdge(inst, temp_pin);
				oaIter<oaPinFig> pinFigIter(temp_pin->getFigs());
				oaPinFig* pinFig = pinFigIter.getNext();
				oaBox pinBBox;
				pinFig->getBBox(pinBBox);
				oaPoint pinCenter;
				pinBBox.getCenter(pinCenter);
				int pinPos;
				switch (side){
					case ON_BOTTOM:
						pinPos = pinCenter.x() / pinMoveStep;
						break;
					case ON_RIGHT:
						pinPos = _xPosNum[inst] + (pinCenter.y() / pinMoveStep);
						break;
					case ON_TOP:
						pinPos = 2 * _xPosNum[inst] + _yPosNum[inst] - pinCenter.x() / pinMoveStep - 1;
						break;
					case ON_LEFT:
						pinPos = _maxPos[inst] - pinCenter.y() / pinMoveStep - 1;
						break;
				}
				
				macroPin temp = getMacroPin(temp_pin, inst, _pinDict);
				_pinPos[temp] = pinPos;
			}
		}
		_rotation[inst] = NOROTATE;
	}
}

PAsolution::PAsolution(PAsolution& _ps)
{
	_pinPos = _ps._pinPos;
	_rotation = _ps._rotation;
}

PAsolution::PAsolution(PAsolution& _ps1, PAsolution& _ps2)
{
	_pinPos = _ps1._pinPos;
	_rotation = _ps2._rotation;
	for(map<oaInst*, int>::iterator it = _rotation.begin();it!= _rotation.end();it++){
		switch(it->second){
			case ROTATE90:
				it->second=ROTATE270;
				break;
			case ROTATE270:
				it->second=ROTATE90;
				break;
		}
	}
}

void PAsolution::pertubate(int perturbationRange)
{
	for (pinMoveIter it = _pinPos.begin(); it != _pinPos.end(); it++)
	{
		it->second += random(-perturbationRange, perturbationRange);
		/*if (_macroMaxPos[it->first.macroName] <= 0)
		{
			cout << "****************************" << endl;
			cout << it->first.macroName << ", " << _macroMaxPos[it->first.macroName] << endl;
			cout << "****************************" << endl;
			assert(_macroMaxPos[it->first.macroName] > 0);
		}
		it->second %= _macroMaxPos[it->first.macroName];*/
		if (it->second < 0)
		{
			it->second += _macroMaxPos[it->first.macroName];
		}
		if (it->second >= _macroMaxPos[it->first.macroName])
		{
			it->second %= _macroMaxPos[it->first.macroName];
		}
	}
	for (map<oaInst*, int>::iterator it = _rotation.begin(); it != _rotation.end(); it++) 
	{
		it->second += random(NOROTATE, ROTATE270);
		if (it->second > ROTATE270)
		{
			it->second %= (ROTATE270 + 1);
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
	for (pinMoveIter it = _pinPos.begin(); it != _pinPos.end(); it++) {
		cout << it->first.macroName << ", " << it->first.pinLabel << ", " << it->second << endl;
	}
	for (map<oaInst*, int>::iterator it = _rotation.begin(); it != _rotation.end(); it++) {
		oaString masterCellName;
		oaInst* inst = it->first;
		oaDesign* master = inst->getMaster();
		master->getCellName(ns, masterCellName);
		cout << masterCellName << ", " << it->second << endl;
	}

	cout << "......" << endl;
	for (map<oaString, int>::iterator it = _macroMaxPos.begin(); it != _macroMaxPos.end(); it++)
	{
		cout << it->first << ", " << it->second << endl;
	}
	cout << "=============================================" << endl;
}

void PAsolution::applySolution(oaBlock* topblock)
{
	oaIter<oaInst> instIter(topblock->getInsts());
	while (oaInst* inst = instIter.getNext())
	{
		int rotateIndex = _rotation[inst];
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

		if (orient == oacR90 || orient == oacR270)
		{
			int temp = instHeight;
			instHeight = instWidth;
			instWidth = temp;
		}

		int xNum = _xPosNum[inst];
		int yNum = _yPosNum[inst];
		int minX = _minX[inst];
		int minY = _minY[inst];
		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		while (oaInstTerm* instTerm = instTermIter.getNext())
		{
			if (!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			while (oaPin* pin = pinIter.getNext())
			{
				macroPin _macroPin = getMacroPin(pin, inst, _pinDict);
				int pos = _pinPos[_macroPin];
				oaBox pinBBox;
				getPinBBox(pin, pinBBox);
				int pinHeight = pinBBox.top() - pinBBox.bottom();
				int pinWidth = pinBBox.right() - pinBBox.left();
				oaPoint originalPinCenter, newPinCenter;
				pinBBox.getCenter(originalPinCenter);
				if (pos < 0 || pos >= 2 * (xNum + yNum)) {
					cout << pos << endl;
					assert(pos >= 0 && pos < 2 * (xNum + yNum));
				}
				if (pos < xNum)
				{
					int x = minX + pos*pinMoveStep;
					newPinCenter.set(x, pinHeight / 2);
				}
				else if (pos < xNum + yNum)
				{
					int y = (pos - xNum)*pinMoveStep + minY;
					newPinCenter.set(instWidth - pinWidth / 2, y);
				}
				else if (pos < 2 * xNum + yNum)
				{
					int x = (2 * xNum + yNum - pos - 1)*pinMoveStep + minX;
					newPinCenter.set(x, instHeight - pinHeight / 2);
				}
				else
				{
					int y = (2 * xNum + 2 * yNum - pos - 1)*pinMoveStep + minY;
					newPinCenter.set(pinWidth / 2, y);
				}
				oaPoint offset(newPinCenter.x() - originalPinCenter.x(), newPinCenter.y() - originalPinCenter.y());
				oaTransform trans(offset);
				oaIter<oaPinFig> pinFigIter(pin->getFigs());
				oaPinFig* pinFig = pinFigIter.getNext();
				pinFig->move(trans);
			}
		}


	}
}

void PAsolution::legalizePinPos()
{
	int movePitch = (int)ceil((float)minPinPitch / pinMoveStep);
	for (pinMoveIter it = _pinPos.begin(); it != _pinPos.end(); it++)
	{
		oaString currentMacroName = it->first.macroName;
		int currentPinPos = it->second;
		int rightNeighborLabel = -1;
		int rightNeighborPinPos = INT_MAX;
		int minPinPos = INT_MAX;
		int minPosPinLabel = -1;
		bool findMacro = false;
		int pinPitch;
		for (pinMoveIter itt = _pinPos.begin(); itt != _pinPos.end(); itt++)
		{
			if (itt->first.macroName != currentMacroName && findMacro) break;
			if (itt->first.macroName == currentMacroName)
			{
				findMacro = true;
			}
			if (itt->second > currentPinPos)
			{
				if (itt->second < rightNeighborPinPos)
				{
					rightNeighborPinPos = itt->second;
					rightNeighborLabel = itt->first.pinLabel;
				}
			}
			if (itt->second < minPinPos)
			{
				minPinPos = itt->second;
				minPosPinLabel = itt->first.pinLabel;
			}
		}
		if (rightNeighborPinPos == INT_MAX)
		{
			pinPitch = minPinPos;
		}
		else
		{
			pinPitch = rightNeighborPinPos - currentPinPos;
		}
		if (pinPitch < movePitch) {
			it->second -= (movePitch - pinPitch);
			if (it->second < 0) {
				it->second += _macroMaxPos[it->first.macroName];
			}
		}
	}
}