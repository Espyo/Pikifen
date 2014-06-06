#ifndef LAFI_ANGLE_PICKER_INCLUDED
#define LAFI_ANGLE_PICKER_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

class lafi_angle_picker : public lafi_widget {
private:
    float angle; //In radians.
    bool dragging_pointer;
    
public:

    lafi_angle_picker(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, float angle = 0, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_angle_picker();
    
    void widget_on_mouse_down(int button, int x, int y);
    void widget_on_mouse_up(int button, int x, int y);
    void widget_on_mouse_move(int x, int y);
    
    static void textbox_lose_focus_handler(lafi_widget* w);
    
    void set_angle_rads(const float a);
    float get_angle_rads();
    
    static string angle_to_str(const float angle);
    static float str_to_angle(const string &s);
    
    void init();
    void draw_self();
    
};

#endif //ifndef LAFI_ANGLE_PICKER_INCLUDED