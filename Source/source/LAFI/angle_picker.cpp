#define _USE_MATH_DEFINES

#include <math.h>

#include "angle_picker.h"

#include "textbox.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates an angle picker.
 */
angle_picker::angle_picker(
    const int x1, const int y1, const int x2, const int y2,
    const float angle, lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    angle(angle),
    dragging_pointer(false) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Creates an angle picker.
 */
angle_picker::angle_picker(const float angle) :
    widget(),
    angle(angle),
    dragging_pointer(false) {
    
    needs_init = true;
}


//Destroys an angle picker.
angle_picker::~angle_picker() {}


/* ----------------------------------------------------------------------------
 * Converts an angle in radians to a string representation, in degrees.
 */
string angle_picker::angle_to_str(const float angle) {
    return to_string((long double) (angle * 180 / M_PI));
}


/* ----------------------------------------------------------------------------
 * Draws the circle and the pointer.
 */
void angle_picker::draw_self() {
    float circle_r = (y2 - y1) / 2;
    float circle_cx = x1 + circle_r;
    float circle_cy = y1 + circle_r;
    al_draw_filled_circle(circle_cx, circle_cy, circle_r, get_bg_color());
    al_draw_arc(
        circle_cx, circle_cy, circle_r,
        TAU / 4 + TAU / 8, TAU / 2, get_darker_bg_color(), 1
    );
    al_draw_arc(
        circle_cx, circle_cy, circle_r,
        TAU / 4 + TAU / 8 + TAU / 2, TAU / 2, get_lighter_bg_color(), 1
    );
    al_draw_line(
        circle_cx, circle_cy,
        circle_cx + cos(angle) * circle_r,
        circle_cy + sin(angle) * circle_r,
        get_fg_color(), 2
    );
}


/* ----------------------------------------------------------------------------
 * Returns the current angle, in radians.
 */
float angle_picker::get_angle_rads() {
    return angle;
}


/* ----------------------------------------------------------------------------
 * Initialize the widget. Creates a textbox (with the
 * angle's string representation).
 */
void angle_picker::init() {
    textbox* t = new textbox(
        x1 + (y2 - y1) + CHECKBOX_BOX_PADDING,
        y1, x2, y2,
        angle_to_str(angle),
        style,
        flags
    );
    
    t->lose_focus_handler = textbox_lose_focus_handler;
    
    add("txt_angle", t);
    
    flags |= FLAG_WUM_NO_CHILDREN;
    
    set_angle_rads(angle);
}


/* ----------------------------------------------------------------------------
 * Sets the widget's angle to a value (in radians), updating both
 * the textbox and the circle's pointer.
 */
void angle_picker::set_angle_rads(const float a) {
    angle = normalize_angle(a);
    ((textbox*) widgets["txt_angle"])->text = angle_to_str(a);
}


/* ----------------------------------------------------------------------------
 * Converts a string representation of an angle in degrees
 * to an angle in radians.
 */
float angle_picker::str_to_angle(const string &s) {
    return atof(s.c_str()) * M_PI / 180;
}


/* ----------------------------------------------------------------------------
 * When the text box's focus is lost, update the pointer
 * on the circle.
 */
void angle_picker::textbox_lose_focus_handler(widget* w) {
    ((angle_picker*) w->parent)->set_angle_rads(
        str_to_angle(((textbox*) w)->text)
    );
    w->parent->call_lose_focus_handler();
}


/* ----------------------------------------------------------------------------
 * On mouse down, check the angle to set it to, judging from the
 * position of the click in comparison to the center of
 * the circle.
 */
void angle_picker::widget_on_mouse_down(
    const int button, const int x, const int y
) {
    if(button != 1) return;
    
    float circle_r = (y2 - y1) / 2;
    float circle_cx = x1 + circle_r;
    float circle_cy = y1 + circle_r;
    
    if(x > x1 + circle_r * 2) return;
    
    set_angle_rads(atan2(y - circle_cy, x - circle_cx));
    dragging_pointer = true;
}


/* ----------------------------------------------------------------------------
 * If the mouse moves while the button is held on it,
 * move the pointer about.
 */
void angle_picker::widget_on_mouse_move(const int x, const int y) {
    if(!dragging_pointer) return;
    
    float circle_r = (y2 - y1) / 2;
    float circle_cx = x1 + circle_r;
    float circle_cy = y1 + circle_r;
    
    if(x > x1 + circle_r * 2) return;
    
    set_angle_rads(atan2(y - circle_cy, x - circle_cx));
}


/* ----------------------------------------------------------------------------
 * On mouse up, just mark the fact that the user is not
 * dragging the pointer around.
 */
void angle_picker::widget_on_mouse_up(const int, const int, const int) {
    dragging_pointer = false;
}


/* ----------------------------------------------------------------------------
 * Normalizes an angle so that it's between 0 and TAU (M_PI * 2).
 */
float normalize_angle(const float a) {
    float new_angle = a;
    new_angle = fmod(new_angle, (float) TAU);
    if(new_angle < 0) new_angle += TAU;
    return new_angle;
}

}
