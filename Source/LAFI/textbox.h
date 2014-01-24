#ifndef LAFI_TEXTBOX_INCLUDED
#define LAFI_TEXTBOX_INCLUDED

#include <string>

using namespace std;

#include "widget.h"

class lafi_textbox : public lafi_widget {
private:
    int scroll_x; //Scroll the text by this much. Used to keep track of the cursor.
public:
    ALLEGRO_BITMAP* normal_bitmap;
    
    unsigned int cursor;
    string text;
    bool editable;
    bool multi_line;
    
    function<void(lafi_widget* w)> change_handler;
    
    void widget_on_key_char(int keycode, int unichar, unsigned int modifiers);
    void call_change_handler();
    
    lafi_textbox(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, string text = "", lafi_style* style = NULL, unsigned char flags = 0);
    lafi_textbox(lafi_textbox &b2);
    ~lafi_textbox();
    
    void render();
    void draw_self();
};

#endif //ifndef LAFI_TEXTBOX_INCLUDED