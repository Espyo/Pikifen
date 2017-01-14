#ifndef LAFI_RADIO_BUTTON_INCLUDED
#define LAFI_RADIO_BUTTON_INCLUDED

#include "widget.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * A radio button widget. Only one of these in a group can be selected.
 * Selecting one (with a mouse click) unselects any others in
 * the same group.
 */
class radio_button : public widget {
public:
    bool selected;
    string text;
    int group;
    
    void init();
    
    radio_button(
        const int x1, const int y1, const int x, const int y2,
        const string &text = "", const int group = 0,
        const bool selected = false, lafi::style* style = NULL,
        const unsigned char flags = 0
    );
    radio_button(
        const string &text = "", const int group = 0,
        const bool selected = false
    );
    ~radio_button();
    void widget_on_left_mouse_click(const int x, const int y);
    
    void select();
    void unselect();
    
    void draw_self();
};



/* ----------------------------------------------------------------------------
 * The actual sphere with the dot. The radio button widget is
 * consisted of this and a label.
 */
class radio_button_button : public widget {
public:
    bool selected;
    
    radio_button_button(
        const int x1 = 0, const int y1 = 0, const bool selected = false,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    ~radio_button_button();
    
    void draw_self();
};

}

#endif //ifndef LAFI_RADIO_BUTTON_INCLUDED
