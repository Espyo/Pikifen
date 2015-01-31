#ifndef LAFI_ANGLE_PICKER_INCLUDED
#define LAFI_ANGLE_PICKER_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * Angle picker. This allows the user to pick an angle,
 * by rotating a dial.
 */
class angle_picker : public widget {
private:
    float angle; // In radians.
    bool dragging_pointer;
    
public:

    angle_picker(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, float angle = 0, lafi::style* style = NULL, unsigned char flags = 0);
    ~angle_picker();
    
    void widget_on_mouse_down(int button, int x, int y);
    void widget_on_mouse_up(int button, int x, int y);
    void widget_on_mouse_move(int x, int y);
    
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

#endif // ifndef LAFI_ANGLE_PICKER_INCLUDED
