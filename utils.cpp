#include "utils.h"
#include <iostream>
#include <string>
using namespace oa;
using namespace std;

utils::utils()
{
}

utils::~utils()
{
}

oaString utils::getMacroName(oaInst* inst) {
	oaNativeNS ns;
	oaString instName, masterCellName, macroName;
	oaDesign* masterDesign = inst->getMaster();
	masterDesign->getCellName(ns, masterCellName);
	inst->getName(ns, instName);
	int masterCellNameLength = masterCellName.getLength();
	int instNameLength = instName.getLength();
	macroName = masterCellName;
	macroName[masterCellName.getLength() - instName.getLength()] = NULL;
	return macroName;
}
