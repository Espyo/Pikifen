#ifndef LAFI_CHECKBOX_INCLUDED
#define LAFI_CHECKBOX_INCLUDED

#include "widget.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * A button widget with two states: on and off. These can be toggled
 * when the user clicks on the checkbox.
 */
class checkbox : public widget {
public:
    string text;
    bool checked;
    
    void check();
    void uncheck();
    void set(bool value);
    
    void init();
    
    checkbox(
        const int x1, const int y1, const int x2, const int y2,
        const string &text = "", const bool checked = false,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    checkbox(const string &text = "", const bool checked = false);
    ~checkbox();
    void widget_on_left_mouse_click(const int x, const int y);
    
    void draw_self();
};



/* ----------------------------------------------------------------------------
 * The actual box that can be ticked. The whole checkbox widget
 * is this plus a label.
 */
class checkbox_box : public widget {
private:

public:
    bool checked;
    
    checkbox_box(
        const int x1 = 0, const int y1 = 0, const bool checked = false,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    ~checkbox_box();
    
    void draw_self();
};

}

#endif //ifndef LAFI_CHECKBOX_INCLUDED
