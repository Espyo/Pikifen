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
public:
    int x, y;
    int w, h;
    function<void()> click_handler;
    bool selected;
    
    menu_widget(const int x = 0, const int y = 0, const int w = 0, const int h = 0, function<void()> ch = nullptr);
    virtual void draw() = 0;
    
    bool mouse_on(const int mx, const int my);
};

class menu_button : public menu_widget {
public:
    string text;
    ALLEGRO_FONT* font;
    ALLEGRO_COLOR text_color;
    
    menu_button(
        const int x = 0, const int y = 0, const int w = 0, const int h = 0, function<void()> ch = nullptr,
        string t = "", ALLEGRO_FONT* f = NULL, const ALLEGRO_COLOR c = al_map_rgb(255, 255, 255)
    );
    virtual void draw();
};

#endif //ifndef MENU_WIDGETS_INCLUDED