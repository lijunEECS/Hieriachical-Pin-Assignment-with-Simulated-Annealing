/* EE201A Winter 2018 Course Project
 */

#include <iostream>
#include "oaDesignDB.h"
#include <vector>
#include "InputOutputHandler.h"
#include "ProjectInputRules.h"
#include "OAHelper.h"
#include "designChecker.h"
#include "PAutils.h"
#include "PAsolution.h"
#include "SAassigner.h"

using namespace oa;
using namespace std;
static oaNativeNS ns;
/*
 * 
 */
int PAsolution::minPinPitch = -1;
int PAsolution::pinMoveStep = -1;
oaNativeNS PAsolution::ns = ns;
pinDict PAsolution::_pinDict;
map<oaInst*, int> PAsolution::_maxPos;
map<oaInst*, int> PAsolution::_xPosNum;
map<oaInst*, int> PAsolution::_yPosNum;
map<oaInst*, int> PAsolution::_minX;
map<oaInst*, int> PAsolution::_minY;
map<oaString, int> PAsolution::_macroMaxPos;

int main(int argc, char *argv[])
{
    //Hello World
    cout << "=================================================" << endl;
    cout << "Automated Inter-Chip Pin Assignment" << endl;
    cout << "UCLA EE 201A Winter 2017 Course Project" << endl;
    cout << endl;
    cout << "<YOUR TEAM NUMBER HERE>" << endl;
    cout << "<YOUR NAMES HERE>" << endl;
    cout << "<YOUR STUDENT IDS HERE>" << endl;
    cout << "=================================================" << endl << endl;
   
    //Usage
    cout << "Ensure you have an existing OA design database before running this tool. Also please adhere to the following command-line usage:" << endl;
    cout << "./PinAssign <DESIGN NAME> <OUTPUT DESIGN NAME> <INPUT RULE FILE NAME> <MACRO INFO FILENAME>" << endl;
    cout << "For example:" << endl;            
    cout << "./PinAssign sbox_x2 sbox_x2_minrule benchmarks/sbox_x2/min.inputrules logs/sbox_x2/pinassign_sbox_x2_minrule.macros" << endl;

	// Initialize OA with data model 3
	oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);
    oaRegionQuery::init("oaRQSystem");

    //Read in design library
    cout << "\nReading design library..." << endl;
    DesignInfo designInfo;
    InputOutputHandler::ReadInputArguments(argv, designInfo);
	oaLib* lib;
    oaDesign* design= InputOutputHandler::ReadOADesign(designInfo, lib);

	// Get the TopBlock for this design.
    oaBlock* block= InputOutputHandler::ReadTopBlock(design);

	// Fetch all instances in top block and save a unique master design copy for each
	cout << "\nSaving copies of each unique macro instance..." << endl;
	InputOutputHandler::SaveMacroDesignCopies(designInfo, block);

    //now, get the input rules from file
    cout << "\nReading input rules..." << endl;
    ProjectInputRules inputRules(designInfo.inputRuleFileName); 
    inputRules.print();
    
    cout << "\nBeginning pin assignment..." << endl;
	//=====================================================================
    // All pin assignment code should be handled here
	// The scratch code below covers basic traversal and some useful functions provided
	// You are free to edit everything in this section (marked by ==)

	oaString netName, instName, masterCellName, assocTermName, termName;
#define test true
#if test

	/*oaIter<oaInst> instIter(block->getInsts());
	while (oaInst* inst = instIter.getNext())
	{
		oaDesign* master = inst->getMaster();
		master->getCellName(ns, masterCellName);
		oaBox bbox;
		inst->getBBox(bbox);
		oaPoint origin;
		inst->getOrigin(origin);
		cout << masterCellName << ", " << inst->getOrient().getName() << ", " << bbox.left() << ", " << bbox.right() << ", " << bbox.top() << ", " << bbox.bottom() << endl;
		cout << origin.x() << ", " << origin.y() << endl;
	}*/

	// 100000 iterations take 610ms
	printDataForMatlab(block, "PAdata_golden.txt");
	pinDict globalDict;
	buildPinDict(block, globalDict);
	cout << "=================inital==========================" << endl;
	printPinDict(globalDict);
	PAsolution dummySolution;
	dummySolution.initializeStaticMember(block, globalDict, inputRules, ns);
	PAsolution initialSolution(block);
	initialSolution.printSolution();
	clock_t startTime, endTime;
	startTime = clock();
	for (int i = 0; i < 100000; i++) {
		initialSolution.pertubate(100);
		initialSolution.legalizePinPos();
	}
	endTime = clock();
	initialSolution.printSolution();
	initialSolution.applySolution(block);
	printPinDict(globalDict);
	printDataForMatlab(block, "PAdata_1.txt");
	cout << "Duration: " << double(endTime - startTime) / CLOCKS_PER_SEC * 1000 << "ms." << endl;

	/*printDataForMatlab(block, "PAdata_golden.txt");
	pinDict globalDict;
	buildPinDict(block, globalDict);
	printPinDict(globalDict);
	PAsolution dummySolution;
	dummySolution.initializeStaticMember(block, globalDict, inputRules, ns);
	PAsolution initialSolution(block);
	initialSolution.printSolution();
	cout << 1 << endl;
	PAsolution perturbatedSolution(initialSolution);
	perturbatedSolution.pertubate(100);
	perturbatedSolution.printSolution();
	cout << 2 << endl;
	SAassigner assigner(perturbatedSolution, inputRules, globalDict, ns);
	cout << 3 << endl;
	perturbatedSolution.applySolution(block);
	cout << 4 << endl;
	printDataForMatlab(block, "PAdata.txt");
	printPinDict(globalDict);*/

#else

	//=====================================================================
#endif
	//=====================================================================
	//check legality of pin assginment
	//=====================================================================
	//  check if hierarchical pin assginment is obtained
	bool hierarchicalPA = designChecker::checkHierarchicalAssignment(design);
	if (hierarchicalPA) { cout << "Hierarchical pin assignment obtained." << endl; }
	else { cout << "Pin assignment is not hierarchical." << endl; }

	//Save the improved version of the design
	InputOutputHandler::SaveAndCloseAllDesigns(designInfo, design, block);

	if (lib)
		lib->close();

    cout << endl << "\nDone!" << endl;
    return 0;
}
