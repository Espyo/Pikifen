/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Day results state class and results state-related functions.
 */

#include "results.h"

#include "../drawing.h"
#include "../game.h"
#include "../functions.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a "results" state.
 */
results_state::results_state() :
    game_state(),
    enemies_beaten(0),
    enemies_total(0),
    pikmin_born(0),
    pikmin_deaths(0),
    points_obtained(0),
    points_total(0),
    time_taken(0.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the results state.
 */
void results_state::do_drawing() {
    //Background.
    al_clear_to_color(al_map_rgb(143, 149, 62));
    
    float logo_width = al_get_bitmap_width(game.sys_assets.bmp_icon);
    float logo_height = al_get_bitmap_height(game.sys_assets.bmp_icon);
    logo_height = game.win_w * 0.08f * (logo_width / logo_height);
    logo_width = game.win_w * 0.08f;
    draw_background_logos(
        time_spent, 6, 6, point(logo_width, logo_height), map_alpha(75),
        point(-60.0f, 30.0f), -TAU / 6.0f
    );
    
    al_draw_filled_rounded_rectangle(
        game.win_w * 0.10f, game.win_h * 0.25f,
        game.win_w * 0.90f, game.win_h * 0.75f,
        20.0f, 20.0f,
        al_map_rgba(112, 106, 193, 96)
    );
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void results_state::do_logic() {
    time_spent += game.delta_t;
    
    if(floor(time_spent / 2.0f - game.delta_t) < floor(time_spent / 2.0f)) {
        //Make the area name grow every two seconds.
        area_name_widget->start_juicy_grow();
    }
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(game.delta_t);
    }
    
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string results_state::get_name() const {
    return "results";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void results_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
}


/* ----------------------------------------------------------------------------
 * Leaves the results menu and goes to the area menu, or retries.
 */
void results_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        //TODO
    });
}


/* ----------------------------------------------------------------------------
 * Loads the results state into memory.
 */
void results_state::load() {
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    
    //Menu widgets.
    menu_button* retry_widget =
        new menu_button(
        point(game.win_w * 0.20, game.win_h * 0.90),
        point(game.win_w * 0.25, game.win_h * 0.06),
    [this] () {
        leave();
    },
    "Retry", game.fonts.main
    );
    menu_widgets.push_back(retry_widget);
    
    menu_button* continue_widget =
        new menu_button(
        point(game.win_w * 0.50, game.win_h * 0.90),
        point(game.win_w * 0.25, game.win_h * 0.06),
    [this] () {
        leave();
    },
    "Keep playing", game.fonts.main
    );
    menu_widgets.push_back(continue_widget);
    
    back_widget =
        new menu_button(
        point(game.win_w * 0.80, game.win_h * 0.90),
        point(game.win_w * 0.25, game.win_h * 0.06),
    [this] () {
        leave();
    },
    "Pick an area", game.fonts.main
    );
    menu_widgets.push_back(back_widget);
    
    area_name_widget =
        new menu_text(
        point(game.win_w * 0.5, game.win_h * 0.10),
        point(game.win_w * 1.0, game.win_h * 0.10),
        area_name,
        game.fonts.area_name
    );
    menu_widgets.push_back(area_name_widget);
    
    menu_text* time_l_widget =
        new menu_text(
        point(game.win_w * 0.35, game.win_h * 0.30),
        point(game.win_w * 0.40, game.win_h * 0.10),
        "Time taken:",
        game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(time_l_widget);
    
    unsigned char ms = fmod(time_taken * 100, 100);
    unsigned char seconds = fmod(time_taken, 60);
    size_t minutes = time_taken / 60.0f;
    menu_text* time_t_widget =
        new menu_text(
        point(game.win_w * 0.70, game.win_h * 0.30),
        point(game.win_w * 0.50, game.win_h * 0.10),
        i2s(minutes) + ":" + pad_string(i2s(seconds), 2, '0') + "." + i2s(ms),
        game.fonts.counter
    );
    menu_widgets.push_back(time_t_widget);
    
    menu_text* points_l_widget =
        new menu_text(
        point(game.win_w * 0.35, game.win_h * 0.40),
        point(game.win_w * 0.40, game.win_h * 0.10),
        "Treasure points:",
        game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(points_l_widget);
    
    menu_text* points_t_widget =
        new menu_text(
        point(game.win_w * 0.70, game.win_h * 0.40),
        point(game.win_w * 0.50, game.win_h * 0.10),
        i2s(points_obtained) + " / " + i2s(points_total),
        game.fonts.counter
    );
    menu_widgets.push_back(points_t_widget);
    
    menu_text* enemies_l_widget =
        new menu_text(
        point(game.win_w * 0.35, game.win_h * 0.50),
        point(game.win_w * 0.40, game.win_h * 0.10),
        "Enemies:",
        game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(enemies_l_widget);
    
    menu_text* enemies_t_widget =
        new menu_text(
        point(game.win_w * 0.70, game.win_h * 0.50),
        point(game.win_w * 0.50, game.win_h * 0.10),
        i2s(enemies_beaten) + " / " + i2s(enemies_total),
        game.fonts.counter
    );
    menu_widgets.push_back(enemies_t_widget);
    
    menu_text* pikmin_born_l_widget =
        new menu_text(
        point(game.win_w * 0.35, game.win_h * 0.60),
        point(game.win_w * 0.40, game.win_h * 0.10),
        "Pikmin born:",
        game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(pikmin_born_l_widget);
    
    menu_text* pikmin_born_t_widget =
        new menu_text(
        point(game.win_w * 0.70, game.win_h * 0.60),
        point(game.win_w * 0.50, game.win_h * 0.10),
        i2s(pikmin_born),
        game.fonts.counter
    );
    menu_widgets.push_back(pikmin_born_t_widget);
    
    menu_text* pikmin_deaths_l_widget =
        new menu_text(
        point(game.win_w * 0.35, game.win_h * 0.70),
        point(game.win_w * 0.40, game.win_h * 0.10),
        "Pikmin deaths:",
        game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(pikmin_deaths_l_widget);
    
    menu_text* pikmin_deaths_t_widget =
        new menu_text(
        point(game.win_w * 0.70, game.win_h * 0.70),
        point(game.win_w * 0.50, game.win_h * 0.10),
        i2s(pikmin_deaths),
        game.fonts.counter
    );
    menu_widgets.push_back(pikmin_deaths_t_widget);
    
}


/* ----------------------------------------------------------------------------
 * Unloads the results state from memory.
 */
void results_state::unload() {
}
