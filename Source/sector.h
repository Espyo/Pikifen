#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <vector>

#include "element.h"

using namespace std;

class sector{
public:

	sector();

	float floor;
	float flying_floor;
	unsigned short type;
	vector<element> elements;
};

enum SECTOR_TYPES{
	SECTOR_TYPE_NORMAL,
	SECTOR_TYPE_BOTTOMLESS_PIT,
	SECTOR_TYPE_BASE,
};

#endif //ifndef SECTOR_INCLUDED