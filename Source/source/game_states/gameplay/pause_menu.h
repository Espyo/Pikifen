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

#include <map>

#include "../../drawing.h"
#include "../../gui.h"
#include "../../mob_script.h"
#include "../../mobs/mob_utils.h"
#include "../../utils/geometry_utils.h"

class pikmin_type;


namespace PAUSE_MENU {
extern const string CONFIRMATION_GUI_FILE_PATH;
extern const string GUI_FILE_PATH;
extern const string HELP_GUI_FILE_PATH;
extern const string MISSION_GUI_FILE_PATH;
extern const ALLEGRO_COLOR RADAR_BG_COLOR;
extern const string RADAR_GUI_FILE_PATH;
extern const ALLEGRO_COLOR RADAR_HIGHEST_COLOR;
extern const ALLEGRO_COLOR RADAR_LOWEST_COLOR;
extern const float RADAR_MAX_ZOOM;
extern const float RADAR_MIN_ZOOM;
extern const float RADAR_ONION_COLOR_FADE_DUR;
extern const float RADAR_ONION_COLOR_FADE_CYCLE_DUR;
}


//Target to leave towards.
enum GAMEPLAY_LEAVE_TARGET {
    //Leave in order to retry the area.
    LEAVE_TO_RETRY,
    //Leave in order to end the exploration/mission.
    LEAVE_TO_END,
    //Leave in order to go to the area selection.
    LEAVE_TO_AREA_SELECT,
};


/* ----------------------------------------------------------------------------
 * Contains information about the pause menu currently being presented to
 * the player.
 */
struct pause_menu_struct {
    public:
    //Categories of help page tidbits.
    enum HELP_CATEGORIES {
        //Gameplay basics tidbits.
        HELP_CATEGORY_GAMEPLAY1,
        //Gameplay advanced tidbits.
        HELP_CATEGORY_GAMEPLAY2,
        //Control tidbits.
        HELP_CATEGORY_CONTROLS,
        //Player type tidbits.
        HELP_CATEGORY_PIKMIN,
        //Noteworthy object tidbits.
        HELP_CATEGORY_OBJECTS,
        
        //Total amount of help page tidbit categories.
        N_HELP_CATEGORIES
    };
    
    //GUI manager for the main pause menu.
    gui_manager gui;
    //GUI manager for the radar page.
    gui_manager radar_gui;
    //GUI manager for the help page.
    gui_manager help_gui;
    //GUI manager for the mission page.
    gui_manager mission_gui;
    //GUI manager for the leaving confirmation page.
    gui_manager confirmation_gui;
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
    void handle_player_action(const player_action &action);
    void start_closing();
    void tick(const float delta_t);
    
private:
    struct tidbit {
        //Name.
        string name;
        //Description.
        string description;
        //Image.
        ALLEGRO_BITMAP* image;
        //Constructor.
        tidbit() : image(nullptr) {}
    };
    
    //Is it currently closing?
    bool closing;
    //Help page category text GUI item.
    text_gui_item* help_category_text;
    //Help page tidbit list.
    list_gui_item* help_tidbit_list;
    //Confirmation page explanation text.
    text_gui_item* confirmation_explanation_text;
    //Radar GUI item.
    gui_item* radar_item;
    //All tidbits in the help page.
    map<HELP_CATEGORIES, vector<tidbit> > tidbits;
    //Currently shown help tidbit, if any.
    tidbit* cur_tidbit;
    //Where the player intends to go by leaving.
    GAMEPLAY_LEAVE_TARGET leave_target;
    //Z of the lowest sector.
    float lowest_sector_z;
    //Z of the highest sector.
    float highest_sector_z;
    //World coordinates to radar screen coordinates transformation.
    ALLEGRO_TRANSFORM world_to_radar_screen_transform;
    //Radar screen coordinates to world coordinates transformation.
    ALLEGRO_TRANSFORM radar_screen_to_world_transform;
    //Radar camera information.
    camera_info radar_cam;
    //Location of the radar cursor, in world coordinates.
    point radar_cursor;
    //Whether a mouse button is being held in the radar.
    bool radar_mouse_down;
    //Minimum coordinates the radar can pan to.
    point radar_min_coords;
    //Maximum coordinates the radar can pan to.
    point radar_max_coords;
    //Icon for the radar cursor.
    ALLEGRO_BITMAP* bmp_radar_cursor;
    //Icon for a Pikmin in the radar.
    ALLEGRO_BITMAP* bmp_radar_pikmin;
    //Icon for a treasure in the radar.
    ALLEGRO_BITMAP* bmp_radar_treasure;
    //Icon for an enemy in the radar.
    ALLEGRO_BITMAP* bmp_radar_enemy;
    //Bubble that surrounds a leader's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_leader_bubble;
    //Skeleton part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_onion_skeleton;
    //Bulb part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_onion_bulb;
    //Icon for a ship in the radar.
    ALLEGRO_BITMAP* bmp_radar_ship;
    
    void add_bullet(
        list_gui_item* list, const string &text,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    void confirm_or_leave();
    button_gui_item* create_page_button(
        const string &text, const string &tooltip_name,
        bool left,
        gui_manager* cur_gui, gui_manager* new_gui
    );
    void draw_radar(const point &center, const point &size);
    void draw_tidbit(
        const ALLEGRO_FONT* const font, const point &where,
        const point &max_size, const string &text
    );
    void fill_mission_fail_list(list_gui_item* list);
    void fill_mission_grading_list(list_gui_item* list);
    string get_mission_goal_status();
    void init_radar_page();
    void init_help_page();
    void init_main_pause_menu();
    void init_mission_page();
    void init_confirmation_page();
    void pan_radar(point amount);
    void populate_help_tidbits(const HELP_CATEGORIES category);
    void start_leaving_gameplay();
    void zoom_radar(float amount);
};


#endif //ifndef PAUSE_MENU_INCLUDED
