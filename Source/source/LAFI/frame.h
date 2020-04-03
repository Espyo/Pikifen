#ifndef LAFI_FRAME_INCLUDED
#define LAFI_FRAME_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * A basic widget container -- it has other widgets inside.
 * It also looks a bit like a painting's frame.
 */
class frame : public widget {
public:
    bool solid_color_only;
    frame(
        const int x1, const int y1, const int x2, const int y2,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    frame();
    frame(frame &f2);
    
    void draw_self();
};

}

#endif //ifndef LAFI_FRAME_INCLUDED
