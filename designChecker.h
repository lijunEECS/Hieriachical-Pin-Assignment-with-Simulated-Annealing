#ifndef CHECKER_H
#define CHECKER_H

#include <string>
#include "oaDesignDB.h"
using namespace std;
using namespace oa; 

class myBox
{
public:
	myBox(oaBox box);
	virtual ~myBox();
	bool operator<(const myBox& right)const;
	int getLeft()const;
	int getRight()const;
	int getTop()const;
	int getBottom()const;

private:
	oaBox bbox;
};

class designChecker
{
public:
	designChecker();
	virtual ~designChecker();
	static bool checkHierarchicalAssignment(oaDesign* design);

private:

};
#endif
