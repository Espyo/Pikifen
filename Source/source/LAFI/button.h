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
        const int x1, const int y1, const int x2, const int y2,
        const string &text = "", const string &description = "",
        ALLEGRO_BITMAP* icon = NULL, lafi::style* style = NULL,
        const unsigned char flags = 0
    );
    button(
        const string &text = "", const string &description = "",
        ALLEGRO_BITMAP* icon = NULL
    );
    ~button();
    
    void draw_self();
};

}

#endif //ifndef LAFI_BUTTON_INCLUDED
