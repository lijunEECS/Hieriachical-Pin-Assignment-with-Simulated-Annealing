/* EE201A Winter 2018 Course Project
 */

#include <iostream>
#include "oaDesignDB.h"
#include <vector>
#include "InputOutputHandler.h"
#include "ProjectInputRules.h"
#include "OAHelper.h"
#include "PAutils.h"
#include "PAsolution.h"


using namespace oa;
using namespace std;
static oaNativeNS ns;
/*
 * 
 */
int PAsolution::minPinPitch = -1;
int PAsolution::pinMoveStep = -1;
int PAsolution::maxPerturbation = -1;
int PAsolution::pinWidth = -1;
int PAsolution::pinHeight = -1;
float PAsolution::alpha = -1;
float PAsolution::beta = -1;
float PAsolution::gamma = -1;
oaNativeNS PAsolution::ns = ns;
pinDict PAsolution::_pinDict;
map<oaInst*, int> PAsolution::_maxPos;
map<oaInst*, int> PAsolution::_xPosNum;
map<oaInst*, int> PAsolution::_yPosNum;
map<oaInst*, int> PAsolution::_minX;
map<oaInst*, int> PAsolution::_minY;
map<oaString, int> PAsolution::_macroMaxPos;
map<oaInst*, int> PAsolution::_instHeight;
map<oaInst*, int> PAsolution::_instWidth;
map<macroPin, oaPoint> PAsolution::_relativePos;
pinMove PAsolution::_originalPinPos;

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
#define test false
#if test
	printDataForMatlab(block, "PAdata_golden.txt");
	pinDict globalDict;
	buildPinDict(block, globalDict);
	printPinDict(globalDict, "goldenDict.txt");
	PAsolution dummySolution;
	dummySolution.initializeStaticMember(block, globalDict, inputRules, ns);
	PAsolution::printStaticData();
	PAsolution initialSolution(block);
	PAsolution::_originalPinPos = initialSolution._pinPos;

	cout<<"=========inital solution========="<<endl;
	initialSolution.printSolution();
    cout<<"================================="<<endl;

	double currentEnergy = initialSolution.evaluate(block);
	double tempEnergy;
    //adjust orientations

    int invalidIterNum = 0;

    cout<<"=========Rotation begin=========="<<endl;
    for(int i = 0; i < 300; i++)
    {
    	if(invalidIterNum >= 50) break;
    	PAsolution tempSolution(initialSolution);
    	tempSolution.randomRotate();
    	tempSolution.applySolution(block);
    	tempEnergy = tempSolution.evaluate(block);
    	if(tempEnergy < currentEnergy)
    	{
    		cout<<i<<", "<<tempEnergy<<", "<<currentEnergy<<endl;
    		currentEnergy = tempEnergy;
    		invalidIterNum = 0;
    	}
    	else
    	{
    		invalidIterNum++;
    		PAsolution recoverSolution(initialSolution, tempSolution);
    		recoverSolution.applySolution(block);
    	}
    }
    cout<<"=========Rotation end=========="<<endl;


	int pertubateRange = (PAsolution::maxPerturbation / PAsolution::pinMoveStep)/2;

	invalidIterNum = 0;
	for (int i = 0; i < 800; i++) {
		if(invalidIterNum >= 100) break;
		PAsolution tempSolution(initialSolution);
		tempSolution.pertubate(pertubateRange);
		//tempSolution.printSolution();
		//cout<<"legalizePinPos"<<endl;
		bool tryLegalize = tempSolution.legalizePinPos();
		//tempSolution.printSolution();
		if(!tryLegalize || !tempSolution.checkPerturbation()) continue;
		tempSolution.applySolution(block);
		float tempEnergy = tempSolution.evaluate(block);
		if(tempEnergy < currentEnergy)
		{
			invalidIterNum = 0;
			cout<<"Improved!"<<endl;
			cout << i << "th iteration, tempEnergy: "<<tempEnergy<<", currentEnergy: "<<currentEnergy<<endl;
			initialSolution = tempSolution;
			currentEnergy = tempEnergy;
		}
		else
		{
			// double seed = rand() /double(RAND_MAX);
			// double gate = exp(-(tempEnergy - currentEnergy)/i);
			// if(seed<gate)
			// {
			// 	cout<<"Degenerated."<<endl;
			// 	cout << i << "th iteration, tempEnergy: "<<tempEnergy<<", currentEnergy: "<<currentEnergy<<endl;
			// 	initialSolution = tempSolution;
			// 	currentEnergy = tempEnergy;
			// }
			invalidIterNum++;
			initialSolution.applySolution(block);
		}
	}

	cout<<"========final solution========="<<endl;
	initialSolution.printSolution();
	printPinDict(globalDict, "finalDict.txt");
#else

#endif
	InputOutputHandler::SaveAndCloseAllDesigns(designInfo, design, block);

	if (lib)
		lib->close();

    cout << endl << "\nDone!" << endl;
    return 0;
}
