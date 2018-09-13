#ifndef LAFI_TEXTBOX_INCLUDED
#define LAFI_TEXTBOX_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * A box widget with user-insertable text. The user must first click
 * on the box, and then type.
 */
class textbox : public widget {
private:
    //Scroll the text by this much. Used to keep track of the cursor.
    int scroll_x;
    //Is the cursor currently visible?
    bool cursor_visible;
    //Time left until the cursor swaps states.
    float cursor_change_time_left;
    
    static const float CURSOR_CHANGE_INTERVAL;
    
public:

    static size_t cur_tab_index;
    
    string text;
    unsigned int cursor;
    unsigned int sel_start;
    unsigned int sel_end;
    bool editable;
    bool multi_line;
    size_t tab_index;
    //If you press the Enter key, simulate a click on this widget.
    widget* enter_key_widget;
    //When there's no text, write this.
    string placeholder;
    
    function<void(widget* w)> change_handler;
    
    void widget_on_key_char(
        const int keycode, const int unichar, const unsigned int modifiers
    );
    void widget_on_mouse_down(const int button, const int x, const int y);
    void widget_on_mouse_move(const int x, const int y);
    void widget_on_tick(const float time);
    void call_change_handler();
    
    textbox(
        const int x1, const int y1, const int x2, const int y2,
        const string &text = "",
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    textbox(const string &text = "");
    textbox(textbox &b2);
    ~textbox();
    
    void draw_self();
    
    unsigned int mouse_to_char(const int mouse_x);
};

}

#endif //ifndef LAFI_TEXTBOX_INCLUDED
