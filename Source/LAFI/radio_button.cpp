#define _USE_MATH_DEFINES

#include <math.h>

#include "const.h"
#include "label.h"
#include "radio_button.h"

lafi_radio_button::lafi_radio_button(int x1, int y1, int x2, int y2, string text, int group, bool selected, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->selected = selected;
    this->text = text;
    this->group = group;
    
    needs_init = true;
}

void lafi_radio_button::init() {
    add("rbb_circle", new lafi_radio_button_button(
            x1,
            y1 + (((y2 - y1) / 2) - LAFI_RADIO_BUTTON_BUTTON_SIZE / 2),
            selected,
            style,
            flags
        ));
        
    int label_width = LAFI_RADIO_BUTTON_BUTTON_SIZE + LAFI_RADIO_BUTTON_BUTTON_PADDING;
    add("lbl_text", new lafi_label(
            x1 + label_width,
            y1,
            x2,
            y2,
            text,
            ALLEGRO_ALIGN_LEFT,
            style,
            flags
        ));
        
}

lafi_radio_button::~lafi_radio_button() {
    /*
    //ToDo
    vector<lafi_widget*>* radio_button_group = &((lafi_container*) parent)->groups[group];
    
    size_t n_radio_buttons = radio_button_group->size();
    for(size_t r=0; r<n_radio_buttons; r++){
        if(radio_button_group->operator[](r) == this){
            radio_button_group->erase(radio_button_group->begin() + r);
            return;
        }
    }
    */
}

void lafi_radio_button::widget_on_left_mouse_click(int, int) {
    select();
}

void lafi_radio_button::select() {
    selected = true;
    ((lafi_radio_button_button*) widgets["rbb_circle"])->selected = true;
    
    if(!parent) return;
    for(auto b = parent->widgets.begin(); b != parent->widgets.end(); b++) {
        if(typeid(*(b->second)) == typeid(lafi_radio_button)) {
            lafi_radio_button* rb_ptr = (lafi_radio_button*) b->second;
            if(rb_ptr->group == group && rb_ptr != this) rb_ptr->unselect();
        }
    }
}

void lafi_radio_button::unselect() {
    selected = false;
    ((lafi_radio_button_button*) widgets["rbb_circle"])->selected = false;
}

void lafi_radio_button::draw_self() { }





lafi_radio_button_button::lafi_radio_button_button(int x1, int y1, bool selected, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x1 + LAFI_RADIO_BUTTON_BUTTON_SIZE, y1 + LAFI_RADIO_BUTTON_BUTTON_SIZE, style, flags) {
    this->selected = selected;
    
}

lafi_radio_button_button::~lafi_radio_button_button() {}

void lafi_radio_button_button::draw_self() {
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    al_draw_filled_circle(x1 + w / 2, y1 + h / 2, w / 2, get_bg_color());
    al_draw_arc(x1 + w / 2, y1 + h / 2, w / 2, -M_PI * 0.25, M_PI, get_lighter_bg_color(), 1);
    al_draw_arc(x1 + w / 2, y1 + h / 2, w / 2, M_PI * 0.75,  M_PI, get_darker_bg_color(),  1);
    
    if(selected) {
        al_draw_filled_circle(x1 + w / 2, y1 + h / 2, w * 0.25, get_fg_color());
    }
}