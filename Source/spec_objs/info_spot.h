#ifndef INFO_POINT_INCLUDED
#define INFO_POINT_INCLUDED

#include <string>

#include "../mob.h"

using namespace std;

class info_spot : public mob {
public:
    string text;
    bool opens_box; //If true, clicking while near this info spot opens a message box with the full text.
    unsigned int text_w; //Used instead of calculating the width every time.
    
    info_spot(float x, float y, sector* sec, string text, bool opens_box);
};

#endif //ifndef INFO_POINT_INCLUDED