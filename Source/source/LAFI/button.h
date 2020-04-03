#ifndef LAFI_BUTTON_INCLUDED
#define LAFI_BUTTON_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * A standard clickable button widget. Upon being clicked (mouse button down
 * and mouse button up), it'll run some code.
 */
class button : public widget {
private:
    string prev_text;
    float offset;
    float offset_start_time_left;
    float offset_reset_time_left;
    
    static const float OFFSET_START_DELAY;
    static const float OFFSET_RESET_DELAY;
    static const float OFFSET_SPEED;
    
public:
    string text;
    ALLEGRO_BITMAP* icon;
    bool autoscroll;
    
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
    
    void widget_on_tick(const float time);
    void draw_self();
};

}

#endif //ifndef LAFI_BUTTON_INCLUDED
