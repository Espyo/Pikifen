#ifndef LAFI_TEXTBOX_INCLUDED
#define LAFI_TEXTBOX_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

class textbox : public widget {
private:
    int scroll_x; //Scroll the text by this much. Used to keep track of the cursor.
    
public:

    static size_t cur_tab_index;
    
    unsigned int cursor;
    unsigned int sel_start;
    unsigned int sel_end;
    string text;
    bool editable;
    bool multi_line;
    size_t tab_index;
    widget* enter_key_widget; //If you press the Enter key, simulate a click on this widget.
    
    function<void(widget* w)> change_handler;
    
    void widget_on_key_char(int keycode, int unichar, unsigned int modifiers);
    void widget_on_mouse_down(int button, int x, int y);
    void widget_on_mouse_move(int x, int y);
    void call_change_handler();
    
    textbox(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, string text = "", lafi::style* style = NULL, unsigned char flags = 0);
    textbox(textbox &b2);
    ~textbox();
    
    void draw_self();
    
    unsigned int mouse_to_char(int mouse_x);
};

}

#endif //ifndef LAFI_TEXTBOX_INCLUDED
