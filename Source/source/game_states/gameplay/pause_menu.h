/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pause menu class and related functions.
 */

#ifndef PAUSE_MENU_INCLUDED
#define PAUSE_MENU_INCLUDED


#include "../../utils/geometry_utils.h"
#include "../../gui.h"
#include "../../mob_script.h"
#include "../../mobs/mob_utils.h"

class pikmin_type;


/* ----------------------------------------------------------------------------
 * Contains information about the pause menu currently being presented to
 * the player.
 */
struct pause_menu_struct {
public:
    //GUI manager for the main pause menu.
    gui_manager gui;
    //GUI manager for the help page.
    gui_manager help_gui;
    //Multiply the background alpha by this much.
    float bg_alpha_mult;
    //Time left until the menu finishes closing.
    float closing_timer;
    //Is the struct meant to be deleted?
    bool to_delete;
    
    pause_menu_struct();
    ~pause_menu_struct();
    void draw();
    void handle_event(const ALLEGRO_EVENT &ev);
    void start_closing();
    void tick(const float delta_t);
    
private:
    //Is it currently closing?
    bool closing;
    //Help page category text GUI item.
    text_gui_item* help_category_text;
    
    void init_main_pause_menu();
    void init_help_page();
    
    static const string GUI_FILE_PATH;
};


#endif //ifndef PAUSE_MENU_INCLUDED
