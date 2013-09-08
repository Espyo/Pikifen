#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

class sector{
public:

	sector();

	float floor;
	float flying_floor;
	unsigned short type;
};

enum SECTOR_TYPES{
	SECTOR_TYPE_NORMAL,
	SECTOR_TYPE_BOTTOMLESS_PIT,
};

#endif //ifndef SECTOR_INCLUDED