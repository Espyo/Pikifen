/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Title screen state class and title screen state-related functions.
 */

#include <algorithm>

#include "title_screen.h"

#include "../core/drawing.h"
#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
#include "../util/allegro_utils.h"
#include "../util/os_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace TITLE_SCREEN {

//Name of the GUI information file.
const string GUI_FILE_NAME = "main_menu";

//How long the menu items take to move when switching pages.
const float HUD_MOVE_TIME = 0.5f;

//Name of the make page GUI information file.
const string MAKE_GUI_FILE_NAME = "main_menu_make";

//Name of the play page GUI information file.
const string PLAY_GUI_FILE_NAME = "main_menu_play";

//Name of the tutorial question page GUI information file.
const string TUTORIAL_GUI_FILE_NAME = "main_menu_tutorial";

}


/**
 * @brief Draws the title screen.
 */
void TitleScreen::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    if(game.debug.show_dear_imgui_demo) return;
    
    draw_bitmap(
        bmp_menu_bg, Point(game.win_w * 0.5, game.win_h * 0.5),
        Point(game.win_w, game.win_h)
    );
    
    //Draw the logo Pikmin.
    Point pik_size = logo_pikmin_size;
    pik_size.x *= game.win_w / 100.0f;
    pik_size.y *= game.win_h / 100.0f;
    
    for(size_t p = 0; p < logo_pikmin.size(); p++) {
        LogoPikmin* pik = &logo_pikmin[p];
        
        draw_bitmap_in_box(
            pik->top, pik->pos, pik_size, true, pik->angle
        );
    }
    
    draw_text(
        "Pikifen and contents are fan works. Pikmin is (c) Nintendo.",
        game.sys_content.fnt_slim,
        Point(8.0f),
        Point(game.win_w * 0.45f, game.win_h * 0.02f), map_alpha(192),
        ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
    );
    string version_text;
    if(!game.config.name.empty()) {
        version_text = game.config.name;
        if(!game.config.version.empty()) {
            version_text += " " + game.config.version;
        }
        version_text += ", powered by ";
    }
    version_text +=
        "Pikifen " + get_engine_version_string();
    draw_text(
        version_text, game.sys_content.fnt_slim,
        Point(game.win_w - 8, 8),
        Point(game.win_w * 0.45f, game.win_h * 0.02f), map_alpha(192),
        ALLEGRO_ALIGN_RIGHT, V_ALIGN_MODE_TOP
    );
    
    main_gui.draw();
    play_gui.draw();
    make_gui.draw();
    tutorial_gui.draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks a frame's worth of logic.
 */
void TitleScreen::do_logic() {
    if(game.debug.show_dear_imgui_demo) return;
    
    //Animate the logo Pikmin.
    for(size_t p = 0; p < logo_pikmin.size(); p++) {
        LogoPikmin* pik = &logo_pikmin[p];
        
        if(!pik->reached_destination) {
            float a = get_angle(pik->pos, pik->destination);
            float speed =
                std::min(
                    (float) (pik->speed * game.delta_t),
                    Distance(pik->pos, pik->destination).to_float() *
                    logo_pikmin_speed_smoothness
                );
            pik->pos.x += cos(a) * speed;
            pik->pos.y += sin(a) * speed;
            if(
                fabs(pik->pos.x - pik->destination.x) < 1.0 &&
                fabs(pik->pos.y - pik->destination.y) < 1.0
            ) {
                pik->destination = pik->pos;
                pik->reached_destination = true;
            }
            
        } else {
            pik->sway_var += pik->sway_speed * game.delta_t;
            pik->pos.x =
                pik->destination.x +
                sin(pik->sway_var) * logo_pikmin_sway_amount;
        }
    }
    
    vector<PlayerAction> player_actions = game.controls.new_frame();
    if(!game.fade_mgr.is_fading()) {
        for(size_t a = 0; a < player_actions.size(); a++) {
            main_gui.handle_player_action(player_actions[a]);
            play_gui.handle_player_action(player_actions[a]);
            make_gui.handle_player_action(player_actions[a]);
            tutorial_gui.handle_player_action(player_actions[a]);
            game.maker_tools.handle_player_action(player_actions[a]);
        }
    }
    
    main_gui.tick(game.delta_t);
    play_gui.tick(game.delta_t);
    make_gui.tick(game.delta_t);
    tutorial_gui.tick(game.delta_t);
    
    //Fade manager needs to come last, because if
    //the fade finishes and the state changes, and
    //after that we still attempt to do stuff in
    //this function, we're going to have a bad time.
    game.fade_mgr.tick(game.delta_t);
    
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string TitleScreen::get_name() const {
    return "title screen";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void TitleScreen::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    main_gui.handle_allegro_event(ev);
    play_gui.handle_allegro_event(ev);
    make_gui.handle_allegro_event(ev);
    tutorial_gui.handle_allegro_event(ev);
}


/**
 * @brief Loads the GUI elements for the main menu's main page.
 */
void TitleScreen::init_gui_main_page() {
    DataNode* gui_file = &game.content.gui_defs.list[TITLE_SCREEN::GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool play_icon_left = icon_left("play", "true");
    bool make_icon_left = icon_left("make", "false");
    bool help_icon_left = icon_left("help", "true");
    bool options_icon_left = icon_left("options", "true");
    bool stats_icon_left = icon_left("statistics", "true");
    bool quit_icon_left = icon_left("quit", "false");
    
#undef icon_left
    
    //Menu items.
    main_gui.register_coords("play",       42, 58, 44, 12);
    main_gui.register_coords("make",       58, 72, 44, 12);
    main_gui.register_coords("help",       24, 83, 24,  6);
    main_gui.register_coords("options",    50, 83, 24,  6);
    main_gui.register_coords("stats",      76, 83, 24,  6);
    main_gui.register_coords("discord",    74, 91,  4,  5);
    main_gui.register_coords("github",     80, 91,  4,  5);
    main_gui.register_coords("exit",       91, 91, 14,  6);
    main_gui.register_coords("exit_input", 97, 93,  4,  4);
    main_gui.register_coords("tooltip",    50, 96, 96,  4);
    main_gui.read_coords(gui_file->getChildByName("positions"));
    
    //Play button.
    ButtonGuiItem* play_button =
        new ButtonGuiItem("Play", game.sys_content.fnt_area_name);
    play_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_PLAY, draw.center, draw.size, play_icon_left
        );
        draw_button(
            draw.center, draw.size,
            play_button->text, play_button->font,
            play_button->color, play_button->selected,
            play_button->get_juice_value()
        );
    };
    play_button->on_activate =
    [this] (const Point &) {
        main_gui.responsive = false;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        if(game.statistics.area_entries == 0) {
            tutorial_gui.responsive = true;
            tutorial_gui.start_animation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                TITLE_SCREEN::HUD_MOVE_TIME
            );
        } else {
            play_gui.responsive = true;
            play_gui.start_animation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                TITLE_SCREEN::HUD_MOVE_TIME
            );
        }
    };
    play_button->on_get_tooltip =
    [] () { return "Choose an area to play in."; };
    main_gui.add_item(play_button, "play");
    
    //Make button.
    ButtonGuiItem* make_button =
        new ButtonGuiItem("Make", game.sys_content.fnt_area_name);
    make_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_MAKE, draw.center, draw.size, make_icon_left
        );
        draw_button(
            draw.center, draw.size,
            make_button->text, make_button->font,
            make_button->color, make_button->selected,
            make_button->get_juice_value()
        );
    };
    make_button->on_activate =
    [this] (const Point &) {
        main_gui.responsive = false;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        make_gui.responsive = true;
        make_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    make_button->on_get_tooltip =
    [] () { return "Make your own content, like areas or animations."; };
    main_gui.add_item(make_button, "make");
    
    //Help button.
    ButtonGuiItem* help_button =
        new ButtonGuiItem("Help", game.sys_content.fnt_area_name);
    help_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_HELP, draw.center, draw.size, help_icon_left
        );
        draw_button(
            draw.center, draw.size,
            help_button->text, help_button->font,
            help_button->color, help_button->selected,
            help_button->get_juice_value()
        );
    };
    help_button->on_activate =
    [this] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.annex_screen);
        });
    };
    help_button->on_get_tooltip =
    [] () {
        return
            "Quick help and tips about how to play. "
            "You can also find this in the pause menu.";
    };
    main_gui.add_item(help_button, "help");
    
    //Options button.
    ButtonGuiItem* options_button =
        new ButtonGuiItem("Options", game.sys_content.fnt_area_name);
    options_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_OPTIONS, draw.center, draw.size, options_icon_left
        );
        draw_button(
            draw.center, draw.size,
            options_button->text, options_button->font,
            options_button->color, options_button->selected,
            options_button->get_juice_value()
        );
    };
    options_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.states.annex_screen->menu_to_load =
                ANNEX_SCREEN_MENU_OPTIONS;
            game.change_state(game.states.annex_screen);
        });
    };
    options_button->on_get_tooltip =
    [] () {
        return
            "Customize your playing experience. "
            "You can also find this in the pause menu.";
    };
    main_gui.add_item(options_button, "options");
    
    //Statistics button.
    ButtonGuiItem* stats_button =
        new ButtonGuiItem("Statistics", game.sys_content.fnt_area_name);
    stats_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_STATISTICS, draw.center, draw.size, stats_icon_left
        );
        draw_button(
            draw.center, draw.size,
            stats_button->text, stats_button->font,
            stats_button->color, stats_button->selected,
            stats_button->get_juice_value()
        );
    };
    stats_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.states.annex_screen->menu_to_load =
                ANNEX_SCREEN_MENU_STATS;
            game.change_state(game.states.annex_screen);
        });
    };
    stats_button->on_get_tooltip =
    [] () {
        return
            "Check out some fun lifetime statistics. "
            "You can also find this in the pause menu.";
    };
    main_gui.add_item(stats_button, "stats");
    
    //Discord server button.
    ButtonGuiItem* discord_button =
        new ButtonGuiItem("", game.sys_content.fnt_area_name);
    discord_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_bitmap_in_box(
            game.sys_content.bmp_discord_icon, draw.center, draw.size * 0.8f, true
        );
        draw_button(
            draw.center, draw.size,
            discord_button->text, discord_button->font,
            discord_button->color, discord_button->selected,
            discord_button->get_juice_value()
        );
    };
    discord_button->on_activate =
    [] (const Point &) {
        open_web_browser(DISCORD_SERVER_URL);
    };
    discord_button->on_get_tooltip =
    [] () {
        return
            "Open the project's Discord server! Discussions! Feedback! "
            "Questions! New content!";
    };
    main_gui.add_item(discord_button, "discord");
    
    //GitHub page button.
    ButtonGuiItem* github_button =
        new ButtonGuiItem("", game.sys_content.fnt_area_name);
    github_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_bitmap_in_box(
            game.sys_content.bmp_github_icon, draw.center, draw.size * 0.8f, true
        );
        draw_button(
            draw.center, draw.size,
            github_button->text, github_button->font,
            github_button->color, github_button->selected,
            github_button->get_juice_value()
        );
    };
    github_button->on_activate =
    [] (const Point &) {
        open_web_browser(GITHUB_PAGE_URL);
    };
    github_button->on_get_tooltip =
    [] () { return "Open the project's GitHub (development) page!"; };
    main_gui.add_item(github_button, "github");
    
    //Exit button.
    main_gui.back_item =
        new ButtonGuiItem("Exit", game.sys_content.fnt_area_name);
    main_gui.back_item->on_draw =
    [this, quit_icon_left] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_QUIT, draw.center, draw.size, quit_icon_left
        );
        draw_button(
            draw.center, draw.size,
            ((ButtonGuiItem*) main_gui.back_item)->text,
            ((ButtonGuiItem*) main_gui.back_item)->font,
            ((ButtonGuiItem*) main_gui.back_item)->color,
            main_gui.back_item->selected,
            main_gui.back_item->get_juice_value()
        );
    };
    main_gui.back_item->on_activate =
    [] (const Point &) {
        save_statistics();
        game.is_game_running = false;
    };
    main_gui.back_item->on_get_tooltip =
    [] () {
        return
            game.config.name.empty() ?
            "Quit Pikifen." :
            "Quit " + game.config.name + ".";
    };
    main_gui.add_item(main_gui.back_item, "exit");
    
    //Exit input icon.
    gui_add_back_input_icon(&main_gui, "exit_input");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&main_gui);
    main_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    main_gui.set_selected_item(play_button, true);
    main_gui.responsive = false;
    main_gui.hide_items();
}


/**
 * @brief Loads the GUI elements for the main menu's make page.
 */
void TitleScreen::init_gui_make_page() {
    DataNode* gui_file = &game.content.gui_defs.list[TITLE_SCREEN::MAKE_GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool anim_editor_icon_left = icon_left("animation_editor", "true");
    bool area_editor_icon_left = icon_left("area_editor", "false");
    bool particle_editor_icon_left = icon_left("particle_editor", "true");
    bool gui_editor_icon_left = icon_left("gui_editor", "false");
    
#undef icon_left
    
    //Menu items.
    make_gui.register_coords("animation_editor", 27.5, 63, 43, 12);
    make_gui.register_coords("area_editor",      72.5, 63, 43, 12);
    make_gui.register_coords("gui_editor",         69, 78, 34,  8);
    make_gui.register_coords("particle_editor",    31, 78, 34,  8);
    make_gui.register_coords("back",                9, 91, 14,  6);
    make_gui.register_coords("back_input",          3, 93,  4,  4);
    make_gui.register_coords("more",               91, 91, 14,  6);
    make_gui.register_coords("tooltip",            50, 96, 96,  4);
    make_gui.read_coords(gui_file->getChildByName("positions"));
    
    //Animation editor button.
    ButtonGuiItem* anim_ed_button =
        new ButtonGuiItem("Animations", game.sys_content.fnt_area_name);
    anim_ed_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_ANIM_EDITOR, draw.center, draw.size, anim_editor_icon_left
        );
        draw_button(
            draw.center, draw.size,
            anim_ed_button->text, anim_ed_button->font,
            anim_ed_button->color, anim_ed_button->selected,
            anim_ed_button->get_juice_value()
        );
    };
    anim_ed_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.animation_ed);
        });
    };
    anim_ed_button->on_get_tooltip =
    [] () { return "Make an animation for any object in the game."; };
    make_gui.add_item(anim_ed_button, "animation_editor");
    
    //Area editor button.
    ButtonGuiItem* area_ed_button =
        new ButtonGuiItem("Areas", game.sys_content.fnt_area_name);
    area_ed_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_AREA_EDITOR, draw.center, draw.size, area_editor_icon_left
        );
        draw_button(
            draw.center, draw.size,
            area_ed_button->text, area_ed_button->font,
            area_ed_button->color, area_ed_button->selected,
            area_ed_button->get_juice_value()
        );
    };
    area_ed_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.area_ed);
        });
    };
    area_ed_button->on_get_tooltip =
    [] () { return "Make an area to play on."; };
    make_gui.add_item(area_ed_button, "area_editor");
    
    //Particle editor button.
    ButtonGuiItem* part_ed_button =
        new ButtonGuiItem("Particles", game.sys_content.fnt_area_name);
    part_ed_button->on_draw =
    [ = ](const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_PARTICLE_EDITOR, draw.center, draw.size, particle_editor_icon_left
        );
        draw_button(
            draw.center, draw.size,
            part_ed_button->text, part_ed_button->font,
            part_ed_button->color, part_ed_button->selected,
            part_ed_button->get_juice_value()
        );
    };
    part_ed_button->on_activate =
    [](const Point &) {
        game.fade_mgr.start_fade(false, []() {
            game.change_state(game.states.particle_ed);
        });
    };
    part_ed_button->on_get_tooltip =
    []() { return "Make generators that create particles."; };
    make_gui.add_item(part_ed_button, "particle_editor");
    
    //GUI editor button.
    ButtonGuiItem* gui_ed_button =
        new ButtonGuiItem("GUI", game.sys_content.fnt_area_name);
    gui_ed_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_GUI_EDITOR, draw.center, draw.size, gui_editor_icon_left
        );
        draw_button(
            draw.center, draw.size,
            gui_ed_button->text, gui_ed_button->font,
            gui_ed_button->color, gui_ed_button->selected,
            gui_ed_button->get_juice_value()
        );
    };
    gui_ed_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.gui_ed);
        });
    };
    gui_ed_button->on_get_tooltip =
    [] () { return "Change the way menus and the gameplay HUD look."; };
    make_gui.add_item(gui_ed_button, "gui_editor");
    
    //Back button.
    make_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_area_name);
    make_gui.back_item->on_activate =
    [this] (const Point &) {
        make_gui.responsive = false;
        make_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        main_gui.responsive = true;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    make_gui.back_item->on_get_tooltip =
    [] () {
        return "Return to the main page.";
    };
    make_gui.add_item(make_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&make_gui);
    
    //More bullet point.
    BulletGuiItem* more_bullet =
        new BulletGuiItem("More...", game.sys_content.fnt_standard);
    more_bullet->on_activate =
    [] (const Point &) {
        open_manual("making.html");
    };
    more_bullet->on_get_tooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for "
            "more info on content making.";
    };
    make_gui.add_item(more_bullet, "more");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&make_gui);
    make_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    make_gui.set_selected_item(anim_ed_button, true);
    make_gui.responsive = false;
    make_gui.hide_items();
}


/**
 * @brief Loads the GUI elements for the main menu's play page.
 */
void TitleScreen::init_gui_play_page() {
    DataNode* gui_file = &game.content.gui_defs.list[TITLE_SCREEN::PLAY_GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool simple_areas_icon_left = icon_left("simple_areas", "true");
    bool missions_icon_left = icon_left("missions", "true");
    
#undef icon_left
    
    //Menu items.
    play_gui.register_coords("simple",     42, 60, 60, 12.5);
    play_gui.register_coords("mission",    44, 78, 60, 12.5);
    play_gui.register_coords("back",        9, 91, 14,    6);
    play_gui.register_coords("back_input",  3, 93,  4,    4);
    play_gui.register_coords("tooltip",    50, 96, 96,    4);
    play_gui.read_coords(gui_file->getChildByName("positions"));
    
    //Play a simple area button.
    ButtonGuiItem* simple_button =
        new ButtonGuiItem("Simple areas", game.sys_content.fnt_area_name);
    simple_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_SIMPLE_AREAS, draw.center, draw.size, simple_areas_icon_left
        );
        draw_button(
            draw.center, draw.size,
            simple_button->text, simple_button->font,
            simple_button->color, simple_button->selected,
            simple_button->get_juice_value()
        );
    };
    simple_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.states.annex_screen->area_menu_area_type =
                AREA_TYPE_SIMPLE;
            game.states.annex_screen->menu_to_load =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.change_state(game.states.annex_screen);
        });
    };
    simple_button->on_get_tooltip =
    [] () { return "Pick a simple area with no goal, and start playing!"; };
    play_gui.add_item(simple_button, "simple");
    
    //Play a mission area button.
    ButtonGuiItem* mission_button =
        new ButtonGuiItem("Missions", game.sys_content.fnt_area_name);
    mission_button->on_draw =
    [ = ] (const DrawInfo & draw) {
        draw_menu_button_icon(
            MENU_ICON_MISSIONS, draw.center, draw.size, missions_icon_left
        );
        draw_button(
            draw.center, draw.size,
            mission_button->text, mission_button->font,
            mission_button->color, mission_button->selected,
            mission_button->get_juice_value()
        );
    };
    mission_button->on_activate =
    [] (const Point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.states.annex_screen->area_menu_area_type =
                AREA_TYPE_MISSION;
            game.states.annex_screen->menu_to_load =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.change_state(game.states.annex_screen);
        });
    };
    mission_button->on_get_tooltip =
    [] () {
        return
            "Pick a mission area with goals and limitations, "
            "and start playing!";
    };
    play_gui.add_item(mission_button, "mission");
    
    //Back button.
    play_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_area_name);
    play_gui.back_item->on_activate =
    [this] (const Point &) {
        play_gui.responsive = false;
        play_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        main_gui.responsive = true;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    play_gui.back_item->on_get_tooltip =
    [] () {
        return "Return to the main page.";
    };
    play_gui.add_item(play_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&play_gui);
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&play_gui);
    play_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    play_gui.set_selected_item(simple_button, true);
    play_gui.responsive = false;
    play_gui.hide_items();
}


/**
 * @brief Loads the GUI elements for the main menu's tutorial question page.
 */
void TitleScreen::init_gui_tutorial_page() {
    DataNode* gui_file = &game.content.gui_defs.list[TITLE_SCREEN::TUTORIAL_GUI_FILE_NAME];
    
    //Menu items.
    tutorial_gui.register_coords("question", 50,     60, 60,  12.5);
    tutorial_gui.register_coords("no",       26, 80.875, 40, 10.25);
    tutorial_gui.register_coords("no_input",  7,     85,  4,     4);
    tutorial_gui.register_coords("yes",      74,     81, 40,    10);
    tutorial_gui.register_coords("tooltip",  50,     96, 96,     4);
    tutorial_gui.read_coords(gui_file->getChildByName("positions"));
    
    //Question text.
    TextGuiItem* question_text =
        new TextGuiItem(
        "If you're new to Pikifen, it is recommended to play the "
        "\"Tutorial Meadow\" mission first.\n\n"
        "Do you want to play there now?",
        game.sys_content.fnt_standard
    );
    question_text->line_wrap = true;
    tutorial_gui.add_item(question_text, "question");
    
    //No button.
    tutorial_gui.back_item =
        new ButtonGuiItem("No", game.sys_content.fnt_standard);
    tutorial_gui.back_item->on_activate =
    [this] (const Point &) {
        tutorial_gui.responsive = false;
        tutorial_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        play_gui.responsive = true;
        play_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    tutorial_gui.back_item->on_get_tooltip =
    [] () {
        return
            "Go to the standard area selection menu.";
    };
    tutorial_gui.add_item(tutorial_gui.back_item, "no");
    
    //No input icon.
    gui_add_back_input_icon(&tutorial_gui, "no_input");
    
    //Yes button.
    ButtonGuiItem* yes_button =
        new ButtonGuiItem("Yes", game.sys_content.fnt_standard);
    yes_button->on_activate =
    [] (const Point &) {
        game.states.gameplay->path_of_area_to_load =
            game.content.areas.manifest_to_path(
                ContentManifest(
                    FOLDER_NAMES::TUTORIAL_AREA, "", FOLDER_NAMES::BASE_PACK
                ), AREA_TYPE_MISSION
            );
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.gameplay);
        });
    };
    yes_button->on_get_tooltip =
    [] () {
        return
            "Play Tutorial Meadow now.";
    };
    tutorial_gui.add_item(yes_button, "yes");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&tutorial_gui);
    tutorial_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    tutorial_gui.set_selected_item(yes_button, true);
    tutorial_gui.responsive = false;
    tutorial_gui.hide_items();
}


/**
 * @brief Loads the title screen into memory.
 */
void TitleScreen::load() {
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Misc. initializations.
    init_gui_main_page();
    init_gui_play_page();
    init_gui_make_page();
    init_gui_tutorial_page();
    
    switch(page_to_load) {
    case MAIN_MENU_PAGE_MAIN: {
        main_gui.responsive = true;
        main_gui.show_items();
        break;
    } case MAIN_MENU_PAGE_PLAY: {
        play_gui.responsive = true;
        play_gui.show_items();
        break;
    } case MAIN_MENU_PAGE_MAKE: {
        make_gui.responsive = true;
        make_gui.show_items();
        break;
    }
    }
    page_to_load = MAIN_MENU_PAGE_MAIN;
    
    DataNode* settings_file = &game.content.gui_defs.list[TITLE_SCREEN::GUI_FILE_NAME];
    
    //Resources.
    bmp_menu_bg = game.content.bitmaps.list.get(game.sys_content_names.bmp_title_screen_bg);
    
    //Logo pikmin.
    DataNode* logo_node = settings_file->getChildByName("logo");
    ReaderSetter logo_rs(logo_node);
    
    DataNode* pik_types_node =
        logo_node->getChildByName("pikmin_types");
    for(size_t t = 0; t < pik_types_node->getNrOfChildren(); t++) {
        DataNode* type_node = pik_types_node->getChild(t);
        if(type_node->name.empty()) continue;
        logo_type_bitmaps[type_node->name[0]] =
            game.content.bitmaps.list.get(type_node->value, type_node);
    }
    
    DataNode* map_node =
        logo_node->getChildByName("map");
    size_t map_total_rows = map_node->getNrOfChildren();
    size_t map_total_cols = 0;
    for(size_t r = 0; r < map_total_rows; r++) {
        map_total_cols =
            std::max(map_total_cols, map_node->getChild(r)->name.size());
    }
    
    logo_rs.set("min_screen_limit", logo_min_screen_limit);
    logo_rs.set("max_screen_limit", logo_max_screen_limit);
    logo_rs.set("pikmin_max_speed", logo_pikmin_max_speed);
    logo_rs.set("pikmin_min_speed", logo_pikmin_min_speed);
    logo_rs.set("pikmin_speed_smoothness", logo_pikmin_speed_smoothness);
    logo_rs.set("pikmin_sway_amount", logo_pikmin_sway_amount);
    logo_rs.set("pikmin_sway_max_speed", logo_pikmin_sway_max_speed);
    logo_rs.set("pikmin_sway_min_speed", logo_pikmin_sway_min_speed);
    logo_rs.set("pikmin_size", logo_pikmin_size);
    
    bool map_ok = true;
    
    for(size_t r = 0; r < map_total_rows; r++) {
        string row = map_node->getChild(r)->name;
        
        for(size_t c = 0; c < row.size(); c++) {
            if(row[c] == '.') continue;
            if(logo_type_bitmaps.find(row[c]) == logo_type_bitmaps.end()) {
                map_ok = false;
                game.errors.report(
                    "Title screen Pikmin logo map has an unknown character \"" +
                    string(1, row[c]) + "\" on row " + i2s(r + 1) +
                    ", column " + i2s(c + 1) + "!"
                );
                break;
            }
            
            LogoPikmin pik;
            
            Point min_pos = logo_min_screen_limit;
            min_pos.x *= game.win_w / 100.0f;
            min_pos.y *= game.win_h / 100.0f;
            Point max_pos = logo_max_screen_limit;
            max_pos.x *= game.win_w / 100.0f;
            max_pos.y *= game.win_h / 100.0f;
            
            pik.top = logo_type_bitmaps[row[c]];
            pik.destination =
                Point(
                    min_pos.x +
                    (max_pos.x - min_pos.x) *
                    (c / (float) map_total_cols),
                    min_pos.y +
                    (max_pos.y - min_pos.y) *
                    (r / (float) map_total_rows)
                );
                
            unsigned char h_side = game.rng.i(0, 1);
            unsigned char v_side = game.rng.i(0, 1);
            
            pik.pos =
                Point(
                    game.rng.f(0, game.win_w * 0.5),
                    game.rng.f(0, game.win_h * 0.5)
                );
                
            if(h_side == 0) {
                pik.pos.x -= game.win_w * 1.2;
            } else {
                pik.pos.x += game.win_w * 1.2;
            }
            if(v_side == 0) {
                pik.pos.y -= game.win_h * 1.2;
            } else {
                pik.pos.y += game.win_h * 1.2;
            }
            
            pik.angle = game.rng.f(0, TAU);
            pik.speed = game.rng.f(logo_pikmin_min_speed, logo_pikmin_max_speed);
            pik.sway_speed =
                game.rng.f(logo_pikmin_sway_min_speed, logo_pikmin_sway_max_speed);
            pik.sway_var = 0;
            pik.reached_destination = false;
            logo_pikmin.push_back(pik);
        }
        
        if(!map_ok) break;
    }
    
    //Finishing touches.
    game.audio.set_current_song(game.sys_content_names.sng_menus);
    game.fade_mgr.start_fade(true, nullptr);
    if(game.debug.show_dear_imgui_demo) game.mouse_cursor.show();
}


/**
 * @brief Unloads the title screen from memory.
 */
void TitleScreen::unload() {
    //Resources.
    game.content.bitmaps.list.free(bmp_menu_bg);
    bmp_menu_bg = nullptr;
    for(const auto &t : logo_type_bitmaps) {
        game.content.bitmaps.list.free(t.second);
    }
    logo_type_bitmaps.clear();
    
    //Menu items.
    main_gui.destroy();
    play_gui.destroy();
    make_gui.destroy();
    tutorial_gui.destroy();
    
    //Misc.
    logo_pikmin.clear();
    
    //Game content.
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
}
