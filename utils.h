#include <string>
#include "oaDesignDB.h"
using namespace oa;
using namespace std;

class utils
{
public:
	utils();
	virtual ~utils();

	oaString getMacroName(oaInst* inst);

private:
};