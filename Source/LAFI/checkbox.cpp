#include "checkbox.h"
#include "const.h"
#include "label.h"

namespace lafi {

checkbox::checkbox(int x1, int y1, int x2, int y2, string text, bool checked, lafi::style* style, unsigned char flags)
    : widget(x1, y1, x2, y2, style, flags) {
    
    this->checked = checked;
    this->text = text;
    
    needs_init = true;
}

void checkbox::init() {
    add("box_square", new checkbox_box(
            x1,
            y1 + (((y2 - y1) / 2) - CHECKBOX_BOX_SIZE / 2),
            checked,
            style,
            flags
        ));
        
    int label_width = CHECKBOX_BOX_SIZE + CHECKBOX_BOX_PADDING;
    add("lbl_text", new label(
            x1 + label_width,
            y1,
            x2,
            y2,
            text,
            ALLEGRO_ALIGN_LEFT,
            style,
            flags
        ));
    flags |= FLAG_WUM_NO_CHILDREN;
}

checkbox::~checkbox() { }

void checkbox::widget_on_left_mouse_click(int, int) {
    if(checked) uncheck(); else check();
}

void checkbox::check() {
    checked = true;
    ((checkbox_box*) widgets["box_square"])->checked = checked;
}

void checkbox::uncheck() {
    checked = false;
    ((checkbox_box*) widgets["box_square"])->checked = checked;
}

void checkbox::set(bool value) {
    if(value) check(); else uncheck();
}

void checkbox::draw_self() { }





checkbox_box::checkbox_box(int x1, int y1, bool checked, lafi::style* style, unsigned char flags)
    : widget(x1, y1, x1 + CHECKBOX_BOX_SIZE, y1 + CHECKBOX_BOX_SIZE, style, flags) {
    
    this->checked = checked;
}

checkbox_box::~checkbox_box() { }

void checkbox_box::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    draw_line(this, DRAW_LINE_TOP,    0, 1, 0, get_darker_bg_color());  //Top line.
    draw_line(this, DRAW_LINE_LEFT,   0, 1, 0, get_darker_bg_color());  //Left line.
    draw_line(this, DRAW_LINE_BOTTOM, 1, 0, 0, get_lighter_bg_color()); //Bottom line.
    draw_line(this, DRAW_LINE_RIGHT,  1, 0, 0, get_lighter_bg_color()); //Right line.
    
    if(checked) {
        al_draw_line(x1 + 2.5, y1 + 6.5, x1 + 5.5, y1 + 9.5, get_fg_color(), 3); //Southeast-going line.
        al_draw_line(x1 + 3.5, y1 + 9.5, x1 + 10,  y1 + 3,   get_fg_color(), 3); //Northeast-going line.
    }
}

}