#include "checkbox.h"

#include "const.h"
#include "label.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a checkbox.
 */
checkbox::checkbox(
    const int x1, const int y1, const int x2, const int y2,
    const string &text, const bool checked, lafi::style* style,
    const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    text(text),
    checked(checked) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Creates a checkbox.
 */
checkbox::checkbox(const string &text, const bool checked) :
    widget(),
    text(text),
    checked(checked) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Destroys a checkbox.
 */
checkbox::~checkbox() { }


/* ----------------------------------------------------------------------------
 * Sets the state to "checked".
 */
void checkbox::check() {
    checked = true;
    ((checkbox_box*) widgets["box_square"])->checked = checked;
}


/* ----------------------------------------------------------------------------
 * Draws the widget. Because the widget is entirely consisted
 * of the checkbox box and a label, two independent widgets,
 * the "checkbox" widget itself draws nothing.
 */
void checkbox::draw_self() { }


/* ----------------------------------------------------------------------------
 * Initializes the checkbox. Creates a checkbox box
 * and a label.
 */
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
            true,
            style,
            flags
        ));
    flags |= FLAG_WUM_NO_CHILDREN;
}


/* ----------------------------------------------------------------------------
 * Sets the state to the specified value.
 * (true = checked, false = unchecked.)
 */
void checkbox::set(bool value) {
    if(value) check(); else uncheck();
}


/* ----------------------------------------------------------------------------
 * Sets the state to "unchecked".
 */
void checkbox::uncheck() {
    checked = false;
    ((checkbox_box*) widgets["box_square"])->checked = checked;
}


/* ----------------------------------------------------------------------------
 * When the mouse is clicked, toggle the state.
 */
void checkbox::widget_on_left_mouse_click(const int, const int) {
    if(checked) uncheck(); else check();
}





/* ----------------------------------------------------------------------------
 * Creates a checkbox box.
 */
checkbox_box::checkbox_box(
    const int x1, const int y1, const bool checked, lafi::style* style,
    const unsigned char flags
) :
    widget(
        x1, y1, x1 + CHECKBOX_BOX_SIZE,
        y1 + CHECKBOX_BOX_SIZE, style, flags
    ),
    checked(checked) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a checkbox box.
 */
checkbox_box::~checkbox_box() { }


/* ----------------------------------------------------------------------------
 * Draws the checkbox box. It's just a square with a fancy border.
 * The latter is drawn line by line. It also draws the checkmark,
 * if the box is ticked.
 */
void checkbox_box::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    //Top line.
    draw_line(this, DRAW_LINE_TOP,    0, 1, 0, get_darker_bg_color());
    //Left line.
    draw_line(this, DRAW_LINE_LEFT,   0, 1, 0, get_darker_bg_color());
    //Bottom line.
    draw_line(this, DRAW_LINE_BOTTOM, 1, 0, 0, get_lighter_bg_color());
    //Right line.
    draw_line(this, DRAW_LINE_RIGHT,  1, 0, 0, get_lighter_bg_color());
    
    if(checked) {
        //Southeast-going line.
        al_draw_line(x1 + 2.5, y1 + 6.5, x1 + 5.5, y1 + 9.5, get_fg_color(), 3);
        //Northeast-going line.
        al_draw_line(x1 + 3.5, y1 + 9.5, x1 + 10,  y1 + 3,   get_fg_color(), 3);
    }
}

}
