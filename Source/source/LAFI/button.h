#ifndef LAFI_BUTTON_INCLUDED
#define LAFI_BUTTON_INCLUDED

#include <string>

using namespace std;

#include "widget.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * A standard clickable button widget. Upon being clicked (mouse button down
 * and mouse button up), it'll run some code.
 */
class button : public widget {
public:
    string text;
    ALLEGRO_BITMAP* icon;
    
    button(
        int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1,
        string text = "", string description = "", ALLEGRO_BITMAP* icon = NULL,
        lafi::style* style = NULL, unsigned char flags = 0
    );
    ~button();
    
    void draw_self();
};

}

#endif //ifndef LAFI_BUTTON_INCLUDED
