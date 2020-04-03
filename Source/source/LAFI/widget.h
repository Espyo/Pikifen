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
    easy_widget_info(
        const string &name, lafi::widget* w, const float width,
        const float height, const unsigned char flags
    );
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
    accelerator(const int key, const unsigned int modifiers, lafi::widget* w);
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
    //Mouse is clicking this widget.
    //The cursor can be on top of the widget or not, though.
    bool mouse_clicking;
    
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
    
    void add(const string &name, widget* w);
    void remove(const string &name);
    
    int easy_row(
        const float vertical_padding = 8, const float horizontal_padding = 8,
        const float widget_padding = 8
    );
    void easy_add(
        const string &name, widget* w, const float width, const float height,
        const unsigned char flags = 0
    );
    void easy_reset();
    //Widgets currently in the row buffer.
    vector<easy_widget_info> easy_row_widgets;
    //Top and bottom of the row.
    float easy_row_y1, easy_row_y2;
    //Padding after top of the current row.
    float easy_row_vertical_padding;
    //Padding to the left and right of the current row.
    float easy_row_horizontal_padding;
    //Padding between widgets on the current row.
    float easy_row_widget_padding;
    
    void register_accelerator(
        const int key, const unsigned int modifiers, widget* w
    );
    vector<accelerator> accelerators;
    
    widget* get_widget_under_mouse(const int mx, const int my);
    bool is_mouse_in(const int mx, const int my);
    void get_offset(int* ox, int* oy);
    
    function<void(widget* w, const int x, const int y)>
    mouse_move_handler;
    function<void(widget* w, const int x, const int y)>
    left_mouse_click_handler;
    function<void(widget* w, const int button, const int x, const int y)>
    mouse_down_handler;
    function<void(widget* w, const int button, const int x, const int y)>
    mouse_up_handler;
    function<void(widget* w, const int dy, const int dx)>
    mouse_wheel_handler;
    function<void(widget* w)>
    mouse_enter_handler;
    function<void(widget* w)>
    mouse_leave_handler;
    function<void(widget* w)>
    get_focus_handler;
    function<void(widget* w)>
    lose_focus_handler;
    
    //Functions for the widget classes to handle, if they want to.
    virtual void widget_on_mouse_move(
        const int x, const int y
    );
    virtual void widget_on_left_mouse_click(
        const int x, const int y
    );
    virtual void widget_on_mouse_down(
        const int button, const int x, const int y
    );
    virtual void widget_on_mouse_up(
        const int button, const int x, const int y
    );
    virtual void widget_on_mouse_wheel(
        const int dy, const int dx
    );
    virtual void widget_on_mouse_enter();
    virtual void widget_on_mouse_leave();
    virtual void widget_on_key_char(
        const int keycode, const int unichar, const unsigned int modifiers
    );
    virtual void widget_on_tick(const float time);
    
    void call_mouse_move_handler(const int x, const int y);
    void call_left_mouse_click_handler(const int x, const int y);
    void call_mouse_down_handler(const int button, const int x, const int y);
    void call_mouse_up_handler(const int button, const int x, const int y);
    void call_mouse_wheel_handler(const int dy, const int dx);
    void call_mouse_enter_handler();
    void call_mouse_leave_handler();
    void call_get_focus_handler();
    void call_lose_focus_handler();
    void simulate_click();
    
    bool needs_init;
    void lose_focus();
    void give_focus(widget* w);
    bool is_disabled();
    void show();
    void hide();
    
    virtual void handle_event(ALLEGRO_EVENT ev);
    void draw();
    void tick(const float time);
    virtual void init();
    virtual void draw_self() = 0;    //Draws just the widget itself.
    
    widget(
        const int x1 = 0, const int y1 = 0, const int x2 = 1, const int y2 = 1,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    widget(widget &w2);
    virtual ~widget() = default;
    
};


void draw_line(
    widget* w, const unsigned char side,
    const int start_offset, const int end_offset,
    const int location_offset, const ALLEGRO_COLOR color
);
void draw_text_lines(
    const ALLEGRO_FONT* const f, const ALLEGRO_COLOR &c,
    const float x, const float y, const int fl, const unsigned char va,
    const string &text
);
int get_text_width(
    const ALLEGRO_FONT* const f, const string &text
);
vector<string> split(
    const string &text, const string &del = " ", const bool inc_empty = false,
    const bool inc_del = false
);

}

#endif //ifndef LAFI_WIDGET_INCLUDED
