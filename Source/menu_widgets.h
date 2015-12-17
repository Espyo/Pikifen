/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menu widgets.
 */

#ifndef MENU_WIDGETS_INCLUDED
#define MENU_WIDGETS_INCLUDED

#include <functional>
#include <string>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

using namespace std;

class menu_widget {
protected:
    virtual void on_click() = 0;
    
    //Juicy.
    float juicy_grow_time_left;
    static const float ICON_SWAY_TIME_SCALE;
    static const float ICON_SWAY_DELTA;
    static const float JUICY_GROW_DURATION;
    static const float JUICY_GROW_DELTA;
    
    
public:
    int x, y;
    int w, h;
    function<void()> click_handler;
    bool selected;
    bool enabled;
    
    menu_widget(const int x = 0, const int y = 0, const int w = 0, const int h = 0, function<void()> click_handler = nullptr);
    virtual void tick(const float time);
    virtual void draw(const float time_spent) = 0;
    virtual bool is_clickable() = 0;
    virtual void start_juicy_grow();
    
    void click();
    
    bool mouse_on(const int mx, const int my);
};

class menu_button : public menu_widget {
private:
    virtual void on_click();
    
public:
    string text;
    ALLEGRO_FONT* font;
    ALLEGRO_COLOR text_color;
    int text_align;
    
    menu_button(
        const int x = 0, const int y = 0, const int w = 0, const int h = 0, function<void()> click_handler = nullptr,
        string text = "", ALLEGRO_FONT* font = NULL, const ALLEGRO_COLOR color = al_map_rgb(255, 255, 255),
        const int align = ALLEGRO_ALIGN_CENTER
    );
    virtual void draw(const float time_spent);
    virtual bool is_clickable();
};

class menu_checkbox : public menu_widget {
private:
    virtual void on_click();
    
public:
    string text;
    ALLEGRO_FONT* font;
    ALLEGRO_COLOR text_color;
    bool checked;
    int text_align;
    
    menu_checkbox(
        const int x = 0, const int y = 0, const int w = 0, const int h = 0, function<void()> click_handler = nullptr,
        string text = "", ALLEGRO_FONT* font = NULL, const ALLEGRO_COLOR color = al_map_rgb(255, 255, 255),
        const int align = ALLEGRO_ALIGN_LEFT
    );
    virtual void draw(const float time_spent);
    virtual bool is_clickable();
};

class menu_text : public menu_widget {
private:
    virtual void on_click();
    
public:
    string text;
    ALLEGRO_FONT* font;
    ALLEGRO_COLOR text_color;
    int text_align;
    
    menu_text(
        const int x = 0, const int y = 0, const int w = 0, const int h = 0,
        string text = "", ALLEGRO_FONT* font = NULL, const ALLEGRO_COLOR color = al_map_rgb(255, 255, 255),
        const int align = ALLEGRO_ALIGN_CENTER
    );
    virtual void draw(const float time_spent);
    virtual bool is_clickable();
};

#endif //ifndef MENU_WIDGETS_INCLUDED
