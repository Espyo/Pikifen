#ifndef LAFI_WIDGET_INCLUDED
#define LAFI_WIDGET_INCLUDED

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "style.h"

using namespace std;

class lafi_widget {
public:
    lafi_widget* parent;
    bool mouse_in;
    bool mouse_clicking;    //Mouse is clicking this widget. The cursor can be on top of the widget or not, though.
    
    int x1;  //Top-left corner, X, global coordinates.
    int y1;  //And Y.
    int x2;  //Bottom-right corner, X, global coordinates.
    int y2;  //And Y.
    int offset_x, offset_y;
    string description;
    unsigned char flags;   //Flags. Use LAFI_FLAG_*.
    lafi_style* style;     //Widget style.
    
    map<string, lafi_widget*> widgets;
    lafi_widget* focused_widget;
    lafi_widget* mouse_over_widget; //The mouse is over this widget.
    
    void add(string name, lafi_widget* widget);
    
    bool needs_init;
    bool needs_rerender;
    bool needs_redraw;
    
    function<void(lafi_widget* w, int x, int y)> mouse_move_handler;
    function<void(lafi_widget* w, int x, int y)> left_mouse_click_handler;
    function<void(lafi_widget* w, int button, int x, int y)> mouse_down_handler;
    function<void(lafi_widget* w, int button, int x, int y)> mouse_up_handler;
    function<void(lafi_widget* w)> mouse_enter_handler;
    function<void(lafi_widget* w)> mouse_leave_handler;
    function<void(lafi_widget* w)> get_focus_handler;
    function<void(lafi_widget* w)> lose_focus_handler;
    
    
    bool is_mouse_in(int mx, int my);
    void get_offset(int* ox, int* oy);
    
    //Functions for the widget classes to handle, if they want to.
    virtual void widget_on_mouse_move(int x, int y);
    virtual void widget_on_left_mouse_click(int x, int y);
    virtual void widget_on_mouse_down(int button, int x, int y);
    virtual void widget_on_mouse_up(int button, int x, int y);
    virtual void widget_on_mouse_enter();
    virtual void widget_on_mouse_leave();
    virtual void widget_on_key_char(int keycode, int unichar, unsigned int modifiers);
    virtual void handle_event(ALLEGRO_EVENT ev);
    void draw();
    virtual void init();
    
    lafi_widget(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, lafi_style* style = NULL, unsigned char flags = 0);
    lafi_widget(lafi_widget &w2);
    ~lafi_widget();
    
    void call_mouse_move_handler(int x, int y);
    void call_left_mouse_click_handler(int x, int y);
    void call_mouse_down_handler(int button, int x, int y);
    void call_mouse_up_handler(int button, int x, int y);
    void call_mouse_enter_handler();
    void call_mouse_leave_handler();
    void call_get_focus_handler();
    void call_lose_focus_handler();
    
    virtual void render() = 0;       //Renders the widget's graphics in memory.
    virtual void draw_self() = 0;    //Draws just the widget itself.
};

#endif //ifndef LAFI_WIDGET_INCLUDED