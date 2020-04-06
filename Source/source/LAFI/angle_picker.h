#ifndef LAFI_ANGLE_PICKER_INCLUDED
#define LAFI_ANGLE_PICKER_INCLUDED

#include <string>

#include "widget.h"


namespace lafi {

/* ----------------------------------------------------------------------------
 * Angle picker widget. This allows the user to pick an angle,
 * by rotating a dial.
 */
class angle_picker : public widget {
private:
    float angle; //In radians.
    bool dragging_pointer;
    
public:

    angle_picker(
        const int x1, const int y1, const int x2, const int y2,
        const float angle = 0, lafi::style* style = NULL,
        const unsigned char flags = 0
    );
    angle_picker(const float angle = 0);
    
    void widget_on_mouse_down(const int button, const int x, const int y);
    void widget_on_mouse_up(const int button, const int x, const int y);
    void widget_on_mouse_move(const int x, const int y);
    
    static void textbox_lose_focus_handler(widget* w);
    
    void set_angle_rads(const float a);
    float get_angle_rads();
    
    static string angle_to_str(const float angle);
    static float str_to_angle(const string &s);
    
    void init();
    void draw_self();
    
};


float normalize_angle(float a);

}

#endif //ifndef LAFI_ANGLE_PICKER_INCLUDED
