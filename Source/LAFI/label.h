#ifndef LAFI_LABEL_INCLUDED
#define LAFI_LABEL_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

class label : public widget {
public:
    int text_flags;
    
    string text;
    
    label(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, string text = "", int text_flags = 0, lafi::style* style = NULL, unsigned char flags = 0);
    ~label();
    
    void draw_self();
};

}

#endif //ifndef LAFI_LABEL_INCLUDED
