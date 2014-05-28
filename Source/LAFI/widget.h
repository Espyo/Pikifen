//ToDo const in functions and reference parameters.

#ifndef LAFI_WIDGET_INCLUDED
#define LAFI_WIDGET_INCLUDED

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "style.h"

using namespace std;

class lafi_widget;

struct lafi_easy_widget_info {
    string name;
    lafi_widget* widget;
    float width, height;
    unsigned char flags;
    lafi_easy_widget_info(string name, lafi_widget* widget, float width, float height, unsigned char flags);
};

struct lafi_accelerator {
    int key;
    unsigned int modifiers;
    lafi_widget* widget;
    lafi_accelerator(int key, unsigned int modifiers, lafi_widget* widget);
};

class lafi_widget {
private:

public:
    lafi_widget* parent;
    bool mouse_in;
    bool mouse_clicking;    //Mouse is clicking this widget. The cursor can be on top of the widget or not, though.
    
    int x1;  //Top-left corner, X, global coordinates.
    int y1;  //And Y.
    int x2;  //Bottom-right corner, X, global coordinates.
    int y2;  //And Y.
    int children_offset_x, children_offset_y;
    string description;
    unsigned char flags;   //Flags. Use LAFI_FLAG_*.
    lafi_style* style;     //Widget style.
    
    ALLEGRO_COLOR get_bg_color();
    ALLEGRO_COLOR get_lighter_bg_color();
    ALLEGRO_COLOR get_darker_bg_color();
    ALLEGRO_COLOR get_fg_color();
    ALLEGRO_COLOR get_alt_color();
    
    map<string, lafi_widget*> widgets;
    lafi_widget* focused_widget;
    
    void add(string name, lafi_widget* widget);
    void remove(string name);
    
    int easy_row(float vertical_padding = 8, float horizontal_padding = 8, float widget_padding = 8);
    void easy_add(string name, lafi_widget* widget, float width, float height, unsigned char flags = 0);
    void easy_reset();
    vector<lafi_easy_widget_info> easy_row_widgets; //Widgets currently in the row buffer.
    float easy_row_y1, easy_row_y2;                 //Top and bottom of the row.
    float easy_row_vertical_padding;                //Padding after top of the current row.
    float easy_row_horizontal_padding;              //Padding to the left and right of the current row.
    float easy_row_widget_padding;                  //Padding between widgets on the current row.
    
    void register_accelerator(int key, unsigned int modifiers, lafi_widget* widget);
    vector<lafi_accelerator> accelerators;
    
    bool needs_init;
    bool needs_redraw;
    void lose_focus();
    
    function<void(lafi_widget* w, int x, int y)> mouse_move_handler;
    function<void(lafi_widget* w, int x, int y)> left_mouse_click_handler;
    function<void(lafi_widget* w, int button, int x, int y)> mouse_down_handler;
    function<void(lafi_widget* w, int button, int x, int y)> mouse_up_handler;
    function<void(lafi_widget* w, int dy, int dx)> mouse_wheel_handler;
    function<void(lafi_widget* w)> mouse_enter_handler;
    function<void(lafi_widget* w)> mouse_leave_handler;
    function<void(lafi_widget* w)> get_focus_handler;
    function<void(lafi_widget* w)> lose_focus_handler;
    
    lafi_widget* get_widget_under_mouse(int mx, int my);
    bool is_mouse_in(int mx, int my);
    void get_offset(int* ox, int* oy);
    
    //Functions for the widget classes to handle, if they want to.
    virtual void widget_on_mouse_move(int x, int y);
    virtual void widget_on_left_mouse_click(int x, int y);
    virtual void widget_on_mouse_down(int button, int x, int y);
    virtual void widget_on_mouse_up(int button, int x, int y);
    virtual void widget_on_mouse_wheel(int dy, int dx);
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
    void call_mouse_wheel_handler(int dy, int dx);
    void call_mouse_enter_handler();
    void call_mouse_leave_handler();
    void call_get_focus_handler();
    void call_lose_focus_handler();
    
    virtual void draw_self() = 0;    //Draws just the widget itself.
};

void lafi_draw_line(lafi_widget* widget, unsigned char side, int start_offset, int end_offset, int location_offset, ALLEGRO_COLOR color);

#endif //ifndef LAFI_WIDGET_INCLUDED