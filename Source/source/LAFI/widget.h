//TODO const in functions and reference parameters.

#ifndef LAFI_WIDGET_INCLUDED
#define LAFI_WIDGET_INCLUDED

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "style.h"

using namespace std;

namespace lafi {
class widget;
}

namespace lafi {

/* ----------------------------------------------------------------------------
 * Container of information about a widget, for the
 * "easy widget" functionality.
 */
struct easy_widget_info {
    string name;
    widget* w;
    float width, height;
    unsigned char flags;
    easy_widget_info(string name, lafi::widget* w, float width, float height, unsigned char flags);
};



/* ----------------------------------------------------------------------------
 * An accelerator. This means that when the user presses the
 * specified key combination, the specified widget
 * is considered "clicked".
 */
struct accelerator {
    int key;
    unsigned int modifiers;
    widget* w;
    accelerator(int key, unsigned int modifiers, lafi::widget* w);
};



/* ----------------------------------------------------------------------------
 * A widget. This can be a button, a text box, etc.
 * Widgets may contain other widgets inside.
 * The "easy widget" functionality allows programmers to
 * easily add widgets to this one, without the hassle
 * of calculating coordinates and such.
 */
class widget {
private:

public:
    widget* parent;
    bool mouse_in;
    bool mouse_clicking;    //Mouse is clicking this widget. The cursor can be on top of the widget or not, though.
    
    int x1;  //Top-left corner, X, global coordinates.
    int y1;  //And Y.
    int x2;  //Bottom-right corner, X, global coordinates.
    int y2;  //And Y.
    int children_offset_x, children_offset_y;
    string description;
    unsigned char flags;   //Flags. Use lafi::FLAG_*.
    lafi::style* style;    //Widget style.
    
    ALLEGRO_COLOR get_bg_color();
    ALLEGRO_COLOR get_lighter_bg_color();
    ALLEGRO_COLOR get_darker_bg_color();
    ALLEGRO_COLOR get_fg_color();
    ALLEGRO_COLOR get_alt_color();
    
    map<string, widget*> widgets;
    widget* focused_widget;
    
    void add(string name, widget* w);
    void remove(string name);
    
    int easy_row(float vertical_padding = 8, float horizontal_padding = 8, float widget_padding = 8);
    void easy_add(string name, widget* w, float width, float height, unsigned char flags = 0);
    void easy_reset();
    vector<easy_widget_info> easy_row_widgets; //Widgets currently in the row buffer.
    float easy_row_y1, easy_row_y2;                 //Top and bottom of the row.
    float easy_row_vertical_padding;                //Padding after top of the current row.
    float easy_row_horizontal_padding;              //Padding to the left and right of the current row.
    float easy_row_widget_padding;                  //Padding between widgets on the current row.
    
    void register_accelerator(int key, unsigned int modifiers, widget* w);
    vector<accelerator> accelerators;
    
    widget* get_widget_under_mouse(int mx, int my);
    bool is_mouse_in(int mx, int my);
    void get_offset(int* ox, int* oy);
    
    function<void(widget* w, int x, int y)> mouse_move_handler;
    function<void(widget* w, int x, int y)> left_mouse_click_handler;
    function<void(widget* w, int button, int x, int y)> mouse_down_handler;
    function<void(widget* w, int button, int x, int y)> mouse_up_handler;
    function<void(widget* w, int dy, int dx)> mouse_wheel_handler;
    function<void(widget* w)> mouse_enter_handler;
    function<void(widget* w)> mouse_leave_handler;
    function<void(widget* w)> get_focus_handler;
    function<void(widget* w)> lose_focus_handler;
    
    //Functions for the widget classes to handle, if they want to.
    virtual void widget_on_mouse_move(int x, int y);
    virtual void widget_on_left_mouse_click(int x, int y);
    virtual void widget_on_mouse_down(int button, int x, int y);
    virtual void widget_on_mouse_up(int button, int x, int y);
    virtual void widget_on_mouse_wheel(int dy, int dx);
    virtual void widget_on_mouse_enter();
    virtual void widget_on_mouse_leave();
    virtual void widget_on_key_char(int keycode, int unichar, unsigned int modifiers);
    
    void call_mouse_move_handler(int x, int y);
    void call_left_mouse_click_handler(int x, int y);
    void call_mouse_down_handler(int button, int x, int y);
    void call_mouse_up_handler(int button, int x, int y);
    void call_mouse_wheel_handler(int dy, int dx);
    void call_mouse_enter_handler();
    void call_mouse_leave_handler();
    void call_get_focus_handler();
    void call_lose_focus_handler();
    
    bool needs_init;
    void lose_focus();
    void give_focus(widget* w);
    bool is_disabled();
    
    virtual void handle_event(ALLEGRO_EVENT ev);
    void draw();
    virtual void init();
    virtual void draw_self() = 0;    //Draws just the widget itself.
    
    widget(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, lafi::style* style = NULL, unsigned char flags = 0);
    widget(widget &w2);
    ~widget();
    
};


void draw_line(widget* w, unsigned char side, int start_offset, int end_offset, int location_offset, ALLEGRO_COLOR color);
void draw_text_lines(const ALLEGRO_FONT* const f, const ALLEGRO_COLOR &c, const float x, const float y, const int fl, const unsigned char va, const string &text);
vector<string> split(string text, const string &del = " ", const bool inc_empty = false, const bool inc_del = false);

}

#endif //ifndef LAFI_WIDGET_INCLUDED
