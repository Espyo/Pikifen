#include "checkbox.h"
#include "checkbox_box.h"
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

void lafi_checkbox::render() { }
void lafi_checkbox::draw_self() { }