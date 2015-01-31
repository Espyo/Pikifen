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
    frame(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, lafi::style* style = NULL, unsigned char flags = 0);
    frame(frame &f2);
    ~frame();
    
    void draw_self();
};

}

#endif // ifndef LAFI_FRAME_INCLUDED
