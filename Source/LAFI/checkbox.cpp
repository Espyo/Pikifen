#include "checkbox.h"
#include "const.h"
#include "label.h"

lafi_checkbox::lafi_checkbox(int x1, int y1, int x2, int y2, string text, bool checked, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->checked = checked;
    this->text = text;
    
    needs_init = true;
}

void lafi_checkbox::init() {
    add("box_square", new lafi_checkbox_box(
            x1,
            y1 + (((y2 - y1) / 2) - LAFI_CHECKBOX_BOX_SIZE / 2),
            checked,
            style,
            flags
        ));
        
    int label_width = LAFI_CHECKBOX_BOX_SIZE + LAFI_CHECKBOX_BOX_PADDING;
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
    flags |= LAFI_FLAG_WUM_NO_CHILDREN;
}

lafi_checkbox::~lafi_checkbox() { }

void lafi_checkbox::widget_on_left_mouse_click(int, int) {
    if(checked) uncheck(); else check();
}

void lafi_checkbox::check() {
    checked = true;
    ((lafi_checkbox_box*) widgets["box_square"])->checked = checked;
}

void lafi_checkbox::uncheck() {
    checked = false;
    ((lafi_checkbox_box*) widgets["box_square"])->checked = checked;
}

void lafi_checkbox::set(bool value) {
    if(value) check(); else uncheck();
}

void lafi_checkbox::draw_self() { }





lafi_checkbox_box::lafi_checkbox_box(int x1, int y1, bool checked, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x1 + LAFI_CHECKBOX_BOX_SIZE, y1 + LAFI_CHECKBOX_BOX_SIZE, style, flags) {
    
    this->checked = checked;
}

lafi_checkbox_box::~lafi_checkbox_box() { }

void lafi_checkbox_box::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    lafi_draw_line(this, LAFI_DRAW_LINE_TOP,    0, 1, 0, get_darker_bg_color());  //Top line.
    lafi_draw_line(this, LAFI_DRAW_LINE_LEFT,   0, 1, 0, get_darker_bg_color());  //Left line.
    lafi_draw_line(this, LAFI_DRAW_LINE_BOTTOM, 1, 0, 0, get_lighter_bg_color()); //Bottom line.
    lafi_draw_line(this, LAFI_DRAW_LINE_RIGHT,  1, 0, 0, get_lighter_bg_color()); //Right line.
    
    if(checked) {
        al_draw_line(x1 + 2.5, y1 + 6.5, x1 + 5.5, y1 + 9.5, get_fg_color(), 3); //Southeast-going line.
        al_draw_line(x1 + 3.5, y1 + 9.5, x1 + 10,  y1 + 3,   get_fg_color(), 3); //Northeast-going line.
    }
}