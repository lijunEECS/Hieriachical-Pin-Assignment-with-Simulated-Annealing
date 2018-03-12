#include "PAsolution.h"

#include <iostream>
using namespace std;

void PAsolution::initializeStaticMember(oaBlock* topblock, pinDict& dict, ProjectInputRules& rules, oaNativeNS _ns)
{
	ns = _ns;
	_pinDict = dict;
	pinMoveStep = (int)(rules.getPinMoveStep() * DBU_PER_MACRON);
	minPinPitch = (int)(rules.getMinPinPitch() * DBU_PER_MACRON);
	maxPerturbation = (int)(rules.getMaxPinPerturbation() * DBU_PER_MACRON);
	//assert(maxPerturbation<0);
	//maxPerturbation -= minPinPitch;
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

		_instHeight[inst] = instHeight;
		_instWidth[inst] = instWidth;

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
			pinHeight = pinBBox.top() - pinBBox.bottom();
			pinWidth = pinBBox.right() - pinBBox.left();
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
		assert((maxCenterX - minCenterX)%pinMoveStep == 0);
		assert((maxCenterY - minCenterY)%pinMoveStep == 0);
		int temp_maxPos = (((maxCenterX - minCenterX)/ pinMoveStep + (maxCenterY - minCenterY)/ pinMoveStep)  + 2) * 2;
		oaString macroName = getMacroName(inst);
		_macroMaxPos[macroName] = temp_maxPos;
		_maxPos[inst] = temp_maxPos;
		_xPosNum[inst] = (maxCenterX - minCenterX) / pinMoveStep + 1;
		_yPosNum[inst] = (maxCenterY - minCenterY) / pinMoveStep + 1;
		_minX[inst] = minCenterX;
		_minY[inst] = minCenterY;
	}

	set<oaString> processedMacro;
	instIter.reset();
	while(oaInst* inst = instIter.getNext()){
		oaString macroName = getMacroName(inst);
		if(processedMacro.find(macroName) == processedMacro.end())
		{
			int xNum = _xPosNum[inst];
			int yNum = _yPosNum[inst];
			int minX = _minX[inst];
			int minY = _minY[inst];
			int instHeight = _instHeight[inst];
			int instWidth = _instWidth[inst];
			processedMacro.insert(macroName);
			oaPoint newPinCenter;
			for (int pos = 0; pos < _macroMaxPos[macroName]; pos++)
			{
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
				macroPin pinPos(macroName, pos);
				_relativePos[pinPos] = newPinCenter;
			}
		}
	}

	oaIter<oaNet> netIter(topblock->getNets());
	int maxWirelengthWithoutPA = 0;
	int totalWirelengthWithoutPA = 0;
	while (oaNet* net = netIter.getNext()){
		int netLength = getHPWL(net);
		if(netLength > maxWirelengthWithoutPA)
		{
			maxWirelengthWithoutPA = netLength;
		}
		totalWirelengthWithoutPA += netLength;
	}

	float avgHalfInstPerimeter = 0;
	instIter.reset();
	int instNum = 0;
	while(oaInst* inst = instIter.getNext())
	{
		instNum++;
		oaBox instBBox;
		inst->getBBox(instBBox);
		avgHalfInstPerimeter += (instBBox.right() - instBBox.left() + instBBox.top() - instBBox.bottom());
	}
	avgHalfInstPerimeter /= instNum;

    alpha = totalWirelengthWithoutPA / maxWirelengthWithoutPA;
    beta = 2.0;
    gamma = 2.0 * totalWirelengthWithoutPA / avgHalfInstPerimeter;
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
		it->second = 4 - it->second;
	}
}

void PAsolution::pertubate(int perturbationRange)
{
	int pinNum = _pinPos.size();
	int selectedPin = random(0, pinNum-1);
	int i = 0;
	for (pinMoveIter it = _pinPos.begin(); it != _pinPos.end(); it++)
	{
		//if(i != selectedPin) continue;
		int seed = random(1, 100);
		if(seed > 1) continue;
		if(maxPerturbation>0)
		{
			int newPos = it->second + random(-perturbationRange, perturbationRange);
			if (newPos < 0)
			{
				newPos += _macroMaxPos[it->first.macroName];
			}
			if (newPos >= _macroMaxPos[it->first.macroName])
			{
				newPos %= _macroMaxPos[it->first.macroName];
			}
			oaString macroName = it->first.macroName;
			macroPin originalPinPos(macroName, _originalPinPos[it->first]);
			macroPin newPinPos(macroName, newPos);
			assert(_relativePos.find(originalPinPos) != _relativePos.end());
			assert(_relativePos.find(newPinPos) != _relativePos.end());
			oaPoint originalPinCenter = _relativePos[originalPinPos];
			oaPoint newPinCenter = _relativePos[newPinPos];
			int pertubation = abs(newPinCenter.x() - originalPinCenter.x()) + abs(newPinCenter.y() - originalPinCenter.y());
			if(pertubation <= maxPerturbation)
			{
				it->second = newPos;
			}
		}
		else
		{
			it->second += random(-perturbationRange, perturbationRange);
			if (it->second < 0)
			{
				it->second += _macroMaxPos[it->first.macroName];
			}
			if (it->second >= _macroMaxPos[it->first.macroName])
			{
				it->second %= _macroMaxPos[it->first.macroName];
			}
		}
		/*if (_macroMaxPos[it->first.macroName] <= 0)
		{
			cout << "****************************" << endl;
			cout << it->first.macroName << ", " << _macroMaxPos[it->first.macroName] << endl;
			cout << "****************************" << endl;
			assert(_macroMaxPos[it->first.macroName] > 0);
		}
		it->second %= _macroMaxPos[it->first.macroName];*/
		
	}
	for (map<oaInst*, int>::iterator it = _rotation.begin(); it != _rotation.end(); it++) 
	{
		int seed = random(0,100);
		if(seed>10) continue;
		it->second += random(NOROTATE, ROTATE270);
		if(_instWidth[it->first]!=_instHeight[it->first]){
			it->second *= 2;
		}
		if (it->second > ROTATE270)
		{
			it->second %= (ROTATE270 + 1);
		}
	}
}

PAsolution::PAsolution()
{
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

		oaIter<oaInstTerm> instTermIter(inst->getInstTerms());
		oaString macroName = getMacroName(inst);
		while (oaInstTerm* instTerm = instTermIter.getNext())
		{
			if (!isExternalPin(instTerm)) continue;
			oaTerm* assocTerm = instTerm->getTerm();
			oaIter<oaPin> pinIter(assocTerm->getPins());
			while (oaPin* pin = pinIter.getNext())
			{
				macroPin _macroPin = getMacroPin(pin, inst, _pinDict);
				int pos = _pinPos[_macroPin];
				macroPin pinPos(macroName, pos);
				oaPoint newPinCenter = _relativePos[pinPos];
				oaPoint originalPinCenter;
				oaBox pinBBox;
				getPinBBox(pin, pinBBox);
				pinBBox.getCenter(originalPinCenter);
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
	oaString currentMacroName;
	std::multimap<macroPin, pinMoveIter> orderPin;
	for(pinMoveIter it = _pinPos.begin(); ; it++)
	{
		if(it->first.macroName != currentMacroName || it == _pinPos.end())
		{
			if(!orderPin.empty())
			{
				for(std::multimap<macroPin, pinMoveIter>::iterator ref = orderPin.begin(); ref != orderPin.end(); )
				{
					//cout<<itt->first.macroName<<", pin"<<itt->second->first.pinLabel<<", "<<itt->first.pinLabel<<endl;
					if (ref->first.macroName != currentMacroName)
					{
						cout<<"first.macroName: "<<ref->first.macroName<<" currentMacroName: "<<currentMacroName<<endl;
						assert(ref->first.macroName == currentMacroName);
					}
					oaPoint refPoint = _relativePos[ref->first];
					std::multimap<macroPin, pinMoveIter>::iterator cursor = ref;

					std::multimap<macroPin, pinMoveIter>::iterator tempIt = ref;
					
					if(ref != orderPin.begin())
					{
						tempIt--;
						while(tempIt->first.pinLabel >= ref->first.pinLabel && tempIt != orderPin.begin())
						{
							cursor--;
							tempIt--;
						}
					}

					//cursor++;
					if(cursor == orderPin.end()) break;
					assert(_relativePos.find(cursor->first)!=_relativePos.end());
					
					oaPoint currentPoint = _relativePos[cursor->first];
					//cout<<2<<endl;
					int dis = abs(currentPoint.x() - refPoint.x()) + abs(currentPoint.y() - refPoint.y());

					std::multimap<macroPin, pinMoveIter>::iterator refNext = ref;
					refNext++;
					int refNextlabel = -1;
					int refNextx = -1;
					int refNexty = -1;
					if(refNext != orderPin.end())
					{
						refNextlabel = refNext->second->first.pinLabel;
						refNextx = _relativePos[refNext->first].x();
						refNexty = _relativePos[refNext->first].y();
					}
					//cout<<"refPin: "<<ref->second->first.pinLabel<<" ("<<refPoint.x()<<','<<refPoint.y()<<") currentPin: "<<cursor->second->first.pinLabel<<" ("<<currentPoint.x()<<','<<currentPoint.y()<<") refNext: "<<refNextlabel<<" ("<<refNextx<<','<<refNexty<<')'<<endl;
					//cout<<"dis: "<<dis<<endl;

					queue<std::multimap<macroPin, pinMoveIter>::iterator> movingPin;
					queue<int> dists;

					while(dis < minPinPitch)
					{
						if(cursor != ref)
						{
							movingPin.push(cursor);
							dists.push(dis);
						}
						cursor++;
						if(cursor == orderPin.end()) break;
						oaPoint currentPoint = _relativePos[cursor->first];
						dis = abs(currentPoint.x() - refPoint.x()) + abs(currentPoint.y() - refPoint.y());
					}
					//cout<<1<<endl;

					bool firstNewNode = true;

					while(!movingPin.empty())
					{
						std::multimap<macroPin, pinMoveIter>::iterator head = movingPin.front();
						movingPin.pop();
						int currentDis = dists.front();
						dists.pop();
						head->second->second += ceil((float)(minPinPitch - currentDis)/pinMoveStep);
						if(head->second->second >= _macroMaxPos[head->first.macroName])
							head->second->second %= _macroMaxPos[head->first.macroName];
						macroPin newNode(currentMacroName, head->second->second);
						if(firstNewNode)
						{
							firstNewNode = false;
							refNext = orderPin.insert(std::pair<macroPin, pinMoveIter>(newNode, head->second));
						}
						else{
							orderPin.insert(std::pair<macroPin, pinMoveIter>(newNode, head->second));
						}
						orderPin.erase(head);
					}
					if(!firstNewNode)
					{
						ref = refNext;
					}
					else
					{
						ref++;
					}
				}
				pinMoveIter temp1 = orderPin.begin()->second;
				macroPin newFirstPin(currentMacroName, temp1->second);
				std::multimap<macroPin, pinMoveIter>::iterator temp2 = orderPin.end();
				temp2--;
				pinMoveIter temp3 = temp2->second;
				macroPin newLastPin(currentMacroName, temp3->second);
				oaPoint p1 = _relativePos[newFirstPin];
				oaPoint p2 = _relativePos[newLastPin];
				int checkDis = abs(p1.x() - p2.x()) + abs(p1.y() - p2.y());
				if(checkDis < minPinPitch)
				{
					cout<<"checkDis: "<<checkDis<<" minPinPitch:"<<minPinPitch<<endl;
					assert(checkDis >= minPinPitch);
				}
				orderPin.clear();
			}
			currentMacroName = it->first.macroName;
			macroPin pinPos(currentMacroName, it->second);
			orderPin.insert(std::pair<macroPin, pinMoveIter>(pinPos, it));
		}
		else
		{
			macroPin pinPos(currentMacroName, it->second);
			orderPin.insert(std::pair<macroPin, pinMoveIter>(pinPos, it));
		}
		if(it == _pinPos.end()) break;
	}
}

float PAsolution::evaluate(oaBlock* block)
{
	oaIter<oaNet> netIter(block->getNets());
	int maxWirelength = 0;
	int totalWirelength = 0;
	oaSigType power("power");
	oaSigType ground("ground");
	while (oaNet* net = netIter.getNext()){
		oaSigType sigType = net->getSigType();
		if(sigType == power || sigType == ground) 
			continue;
		int netLength = getHPWL(net);
		if(netLength > maxWirelength)
		{
			maxWirelength = netLength;
		}
		totalWirelength += netLength;
	}

	int pinNum = 0;
	float avgPinPerturbation = 0;
	for(pinMoveIter it = _pinPos.begin(); it != _pinPos.end(); it++)
	{
		pinNum++;
		oaString macroName = it->first.macroName;
		macroPin currentPinPos(macroName, it->second);
		macroPin newPinPos(macroName, _originalPinPos[it->first]);
		assert(_relativePos.find(currentPinPos) != _relativePos.end());
		assert(_relativePos.find(newPinPos) != _relativePos.end());
		oaPoint currentPinCenter = _relativePos[currentPinPos];
		oaPoint newPinCenter = _relativePos[newPinPos];
		avgPinPerturbation += abs(newPinCenter.x() - currentPinCenter.x()) + abs(newPinCenter.y() - currentPinCenter.y());
	}
	avgPinPerturbation /= pinNum;

	cout<<maxWirelength<<", "<<totalWirelength<<", "<<avgPinPerturbation<<endl;

	return alpha*maxWirelength + beta*totalWirelength + gamma*avgPinPerturbation;

}

void PAsolution::printStaticData()
{
	cout<<"==============="<<endl;
	cout<<"static data:"<<endl;
	cout<<"pinWidth: "<<pinWidth<<endl;
	cout<<"pinHeight: "<<pinHeight<<endl;
	cout<<"alpha: "<<alpha<<endl;
	cout<<"beta: "<<beta<<endl;
	cout<<"gamma: "<<gamma<<endl;
	cout<<"pinMoveStep: "<<pinMoveStep<<endl;
	cout<<"minPinPitch: "<<minPinPitch<<endl;
	cout<<"maxPerturbation: "<<maxPerturbation<<endl;
	cout<<"==============="<<endl;
}

bool PAsolution::checkPerturbation()
{
	if(maxPerturbation <= 0)
		return true;
	for (pinMoveIter it = _pinPos.begin(); it != _pinPos.end(); it++)
	{
		oaString macroName = it->first.macroName;
		macroPin originalPinPos(macroName, _originalPinPos[it->first]);
		macroPin newPinPos(macroName, it->second);
		assert(_relativePos.find(originalPinPos) != _relativePos.end());
		assert(_relativePos.find(newPinPos) != _relativePos.end());
		oaPoint originalPinCenter = _relativePos[originalPinPos];
		oaPoint newPinCenter = _relativePos[newPinPos];
		int pertubation = abs(newPinCenter.x() - originalPinCenter.x()) + abs(newPinCenter.y() - originalPinCenter.y());
		if(pertubation > maxPerturbation)
		{
			return false;
		}		
	}
	return true;
}