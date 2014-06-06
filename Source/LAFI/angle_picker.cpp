#define _USE_MATH_DEFINES

#include <math.h>

#include "angle_picker.h"
#include "../functions.h"
#include "textbox.h"

lafi_angle_picker::lafi_angle_picker(int x1, int y1, int x2, int y2, float angle, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->angle = angle;
    
    needs_init = true;
    dragging_pointer = false;
}

lafi_angle_picker::~lafi_angle_picker() {}

string lafi_angle_picker::angle_to_str(const float angle) {
    return to_string((long double) (angle * 180 / M_PI));
}

float lafi_angle_picker::str_to_angle(const string &s) {
    return atof(s.c_str()) * M_PI / 180;
}

void lafi_angle_picker::widget_on_mouse_down(int button, int x, int y) {
    if(button != 1) return;
    
    float circle_r = (y2 - y1) / 2;
    float circle_cx = x1 + circle_r;
    float circle_cy = y1 + circle_r;
    
    if(x > x1 + circle_r * 2) return;
    
    set_angle_rads(atan2(y - circle_cy, x - circle_cx));
    dragging_pointer = true;
}

void lafi_angle_picker::widget_on_mouse_up(int button, int x, int y) {
    dragging_pointer = false;
}

void lafi_angle_picker::widget_on_mouse_move(int x, int y) {
    if(!dragging_pointer) return;
    
    float circle_r = (y2 - y1) / 2;
    float circle_cx = x1 + circle_r;
    float circle_cy = y1 + circle_r;
    
    if(x > x1 + circle_r * 2) return;
    
    set_angle_rads(atan2(y - circle_cy, x - circle_cx));
}

void lafi_angle_picker::init() {
    lafi_textbox* t = new lafi_textbox(
        x1 + (y2 - y1) + LAFI_CHECKBOX_BOX_PADDING,
        y1, x2, y2,
        angle_to_str(angle),
        style,
        flags
    );
    
    t->lose_focus_handler = textbox_lose_focus_handler;
    
    add("txt_angle", t);
    
    flags |= LAFI_FLAG_WUM_NO_CHILDREN;
    
    set_angle_rads(angle);
}

void lafi_angle_picker::draw_self() {
    float circle_r = (y2 - y1) / 2;
    float circle_cx = x1 + circle_r;
    float circle_cy = y1 + circle_r;
    al_draw_filled_circle(circle_cx, circle_cy, circle_r, get_bg_color());
    al_draw_arc(circle_cx, circle_cy, circle_r, M_PI_2 + M_PI_4, M_PI, get_darker_bg_color(), 1);
    al_draw_arc(circle_cx, circle_cy, circle_r, M_PI_2 + M_PI_4 + M_PI, M_PI, get_lighter_bg_color(), 1);
    al_draw_line(
        circle_cx, circle_cy,
        circle_cx + cos(angle) * circle_r,
        circle_cy + sin(angle) * circle_r,
        get_fg_color(), 2
    );
}

void lafi_angle_picker::set_angle_rads(float a) {
    a = normalize_angle(a);
    angle = a;
    ((lafi_textbox*) widgets["txt_angle"])->text = angle_to_str(a);
}

float lafi_angle_picker::get_angle_rads() {
    return angle;
}

void lafi_angle_picker::textbox_lose_focus_handler(lafi_widget* w) {
    ((lafi_angle_picker*) w->parent)->set_angle_rads(str_to_angle(((lafi_textbox*) w)->text));
    w->parent->call_lose_focus_handler();
}