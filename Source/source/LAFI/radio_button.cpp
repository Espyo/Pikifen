#define _USE_MATH_DEFINES

#include <math.h>

#include "const.h"
#include "label.h"
#include "radio_button.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a radio button.
 */
radio_button::radio_button(
    const int x1, const int y1, const int x2, const int y2, const string &text,
    const int group, const bool selected,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    selected(selected),
    text(text),
    group(group) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Creates a radio button.
 */
radio_button::radio_button(
    const string &text, const int group, const bool selected
) :
    widget(),
    selected(selected),
    text(text),
    group(group) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Initializes the radio button. Creates a radio button button
 * and a label.
 */
void radio_button::init() {
    add("rbb_circle", new radio_button_button(
            x1,
            y1 + (((y2 - y1) / 2) - RADIO_BUTTON_BUTTON_SIZE / 2),
            selected,
            style,
            flags
        ));
        
    int label_width = RADIO_BUTTON_BUTTON_SIZE + RADIO_BUTTON_BUTTON_PADDING;
    add("lbl_text", new label(
            x1 + label_width,
            y1,
            x2,
            y2,
            text,
            ALLEGRO_ALIGN_LEFT,
            true,
            style,
            flags
        ));
    flags |= FLAG_WUM_NO_CHILDREN;
}


/* ----------------------------------------------------------------------------
 * Destroys a radio button.
 */
radio_button::~radio_button() {
    /*
    //TODO
    vector<widget*>* radio_button_group =
        &((lafi::container*) parent)->groups[group];
    
    size_t n_radio_buttons = radio_button_group->size();
    for(size_t r=0; r<n_radio_buttons; ++r){
        if(radio_button_group->operator[](r) == this){
            radio_button_group->erase(radio_button_group->begin() + r);
            return;
        }
    }
    */
}


/* ----------------------------------------------------------------------------
 * When the user clicks, this radio button is selected.
 */
void radio_button::widget_on_left_mouse_click(const int, const int) {
    select();
}


/* ----------------------------------------------------------------------------
 * Selects this radio button and unselects all others
 * in the same group.
 */
void radio_button::select() {
    selected = true;
    ((radio_button_button*) widgets["rbb_circle"])->selected = true;
    
    if(!parent) return;
    for(auto b = parent->widgets.begin(); b != parent->widgets.end(); ++b) {
        widget* b_ptr = b->second;
        if(typeid(*b_ptr) == typeid(radio_button)) {
            radio_button* rb_ptr = (radio_button*) b->second;
            if(rb_ptr->group == group && rb_ptr != this) rb_ptr->unselect();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Unselects this radio button.
 */
void radio_button::unselect() {
    selected = false;
    ((radio_button_button*) widgets["rbb_circle"])->selected = false;
}


/* ----------------------------------------------------------------------------
 * Draws the radio button. Because the radio button is
 * consisted of a radio button button and a label, nothing
 * is really drawn for the actual "radio button" widget.
 */
void radio_button::draw_self() { }





/* ----------------------------------------------------------------------------
 * Creates a radio button button.
 */
radio_button_button::radio_button_button(
    const int x1, const int y1, const bool selected,
    lafi::style* style, const unsigned char flags
) :
    widget(
        x1, y1, x1 + RADIO_BUTTON_BUTTON_SIZE,
        y1 + RADIO_BUTTON_BUTTON_SIZE, style, flags
    ),
    selected(selected) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a radio button button.
 */
radio_button_button::~radio_button_button() {}


/* ----------------------------------------------------------------------------
 * Draws the radio button button. It's just a circle, and if
 * it is selected, it also draws a dot.
 */
void radio_button_button::draw_self() {
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    al_draw_filled_circle(x1 + w / 2, y1 + h / 2, w / 2, get_bg_color());
    al_draw_arc(
        x1 + w / 2, y1 + h / 2, w / 2,
        -(TAU / 8), TAU / 2, get_lighter_bg_color(), 1
    );
    al_draw_arc(
        x1 + w / 2, y1 + h / 2, w / 2,
        (TAU / 4 + TAU / 8),  TAU / 2, get_darker_bg_color(), 1
    );
    
    if(selected) {
        al_draw_filled_circle(x1 + w / 2, y1 + h / 2, w * 0.25, get_fg_color());
    }
}

}
