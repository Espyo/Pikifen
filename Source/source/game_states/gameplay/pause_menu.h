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
extern const float ENTRY_LOCKOUT_TIME;
extern const float GO_HERE_CALC_INTERVAL;
extern const string GUI_FILE_PATH;
extern const string HELP_GUI_FILE_PATH;
extern const string MISSION_GUI_FILE_PATH;
extern const ALLEGRO_COLOR RADAR_BG_COLOR;
extern const float RADAR_DEF_ZOOM;
extern const string RADAR_GUI_FILE_PATH;
extern const ALLEGRO_COLOR RADAR_HIGHEST_COLOR;
extern const ALLEGRO_COLOR RADAR_LOWEST_COLOR;
extern const float RADAR_MAX_ZOOM;
extern const float RADAR_MIN_ZOOM;
extern const float RADAR_ONION_COLOR_FADE_CYCLE_DUR;
extern const float RADAR_ONION_COLOR_FADE_DUR;
extern const float RADAR_PAN_SPEED;
extern const float RADAR_ZOOM_SPEED;
extern const string STATUS_GUI_FILE_PATH;
}


//Target to leave towards.
enum GAMEPLAY_LEAVE_TARGET {

    //Leave in order to retry the area.
    GAMEPLAY_LEAVE_TARGET_RETRY,
    
    //Leave in order to end the exploration/mission.
    GAMEPLAY_LEAVE_TARGET_END,
    
    //Leave in order to go to the area selection.
    GAMEPLAY_LEAVE_TARGET_AREA_SELECT,

};


//Pages.
enum PAUSE_MENU_PAGE {

    //System.
    PAUSE_MENU_PAGE_SYSTEM,
    
    //Radar.
    PAUSE_MENU_PAGE_RADAR,
    
    //Status.
    PAUSE_MENU_PAGE_STATUS,
    
    //Mission.
    PAUSE_MENU_PAGE_MISSION

};


/**
 * @brief Info about the pause menu currently being presented to
 * the player.
 *
 */
struct pause_menu_t {

public:

    //--- Misc. declarations ---

    //Categories of help page tidbits.
    enum HELP_CATEGORY {

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

    //Types of spots for each Go Here drawn path segment.
    enum GO_HERE_SEGMENT_SPOT {
        
        //The selected leader.
        GO_HERE_SEGMENT_SPOT_LEADER,

        //The cursor position.
        GO_HERE_SEGMENT_SPOT_CURSOR,

        //A path stop.
        GO_HERE_SEGMENT_SPOT_STOP,
        
    };
    

    //--- Members ---

    //GUI manager for the main pause menu.
    gui_manager gui;

    //GUI manager for the radar page.
    gui_manager radar_gui;

    //GUI manager for the status page.
    gui_manager status_gui;

    //GUI manager for the mission page.
    gui_manager mission_gui;

    //GUI manager for the help page.
    gui_manager help_gui;

    //GUI manager for the leaving confirmation page.
    gui_manager confirmation_gui;

    //Multiply the background alpha by this much.
    float bg_alpha_mult = 0.0f;

    //Control lockout time left for when the menu opens.
    float opening_lockout_timer = 0.0f;

    //Time left until the menu finishes closing.
    float closing_timer = 0.0f;

    //Is the struct meant to be deleted?
    bool to_delete = false;
    

    //--- Function declarations ---

    explicit pause_menu_t(bool start_on_radar);
    ~pause_menu_t();
    void draw();
    void handle_event(const ALLEGRO_EVENT &ev);
    void handle_player_action(const player_action &action);
    void tick(const float delta_t);
    
private:

    //--- Misc. declarations ---

    /**
     * @brief One of the help menu's tidbits.
     */
    struct tidbit {
        
        //--- Members ---

        //Name.
        string name;
        
        //Description.
        string description;
        
        //Image.
        ALLEGRO_BITMAP* image = nullptr;

    };
    

    //--- Members ---

    //Is it currently closing?
    bool closing = false;

    //Help page category text GUI item.
    text_gui_item* help_category_text = nullptr;

    //Help page tidbit list.
    list_gui_item* help_tidbit_list = nullptr;

    //Confirmation page explanation text.
    text_gui_item* confirmation_explanation_text = nullptr;

    //Radar GUI item.
    gui_item* radar_item = nullptr;

    //Pikmin status list.
    list_gui_item* pikmin_list = nullptr;

    //All tidbits in the help page.
    map<HELP_CATEGORY, vector<tidbit> > tidbits;

    //Currently shown help tidbit, if any.
    tidbit* cur_tidbit = nullptr;

    //Where the player intends to go by leaving.
    GAMEPLAY_LEAVE_TARGET leave_target = GAMEPLAY_LEAVE_TARGET_AREA_SELECT;

    //Pages available, in order.
    vector<PAUSE_MENU_PAGE> pages;

    //Z of the lowest sector.
    float lowest_sector_z = 0.0f;

    //Z of the highest sector.
    float highest_sector_z = 0.0f;

    //World coordinates to radar screen coordinates transformation.
    ALLEGRO_TRANSFORM world_to_radar_screen_transform;

    //Radar screen coordinates to world coordinates transformation.
    ALLEGRO_TRANSFORM radar_screen_to_world_transform;

    //Radar camera information.
    camera_t radar_cam;

    //Location of the radar cursor, in world coordinates.
    point radar_cursor;

    //Whether a mouse button is being held in the radar.
    bool radar_mouse_down = false;

    //Point where the mouse button was first held in the radar (screen coords).
    point radar_mouse_down_point;

    //Whether the player is dragging the mouse. False for just a (fuzzy) click.
    bool radar_mouse_dragging = false;

    //Minimum coordinates the radar can pan to.
    point radar_min_coords;

    //Maximum coordinates the radar can pan to.
    point radar_max_coords;

    //Icon for the radar cursor.
    ALLEGRO_BITMAP* bmp_radar_cursor = nullptr;

    //Icon for a Pikmin in the radar.
    ALLEGRO_BITMAP* bmp_radar_pikmin = nullptr;

    //Icon for a treasure in the radar.
    ALLEGRO_BITMAP* bmp_radar_treasure = nullptr;

    //Icon for an enemy in the radar.
    ALLEGRO_BITMAP* bmp_radar_enemy = nullptr;

    //Bubble that surrounds a leader's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_leader_bubble = nullptr;

    //Skeleton part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_onion_skeleton = nullptr;

    //Bulb part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_onion_bulb = nullptr;

    //Icon for a ship in the radar.
    ALLEGRO_BITMAP* bmp_radar_ship = nullptr;

    //Texture for a path in the radar.
    ALLEGRO_BITMAP* bmp_radar_path = nullptr;

    //Selected leader in the radar.
    mob* radar_selected_leader = nullptr;

    //Leader under the cursor in the radar.
    mob* radar_cursor_leader = nullptr;

    //Time left before another Go Here calculation.
    float go_here_calc_time = 0.0f;

    //Go Here path.
    vector<path_stop*> go_here_path;

    //Go Here path result.
    PATH_RESULT go_here_path_result = PATH_RESULT_NOT_CALCULATED;

    //Pan speed and amount.
    movement_t radar_pan;

    //Whether the radar zoom-in input is pressed.
    bool radar_zoom_in = false;

    //Whether the radar zoom-out input is pressed.
    bool radar_zoom_out = false;
    

    //--- Function declarations ---
    
    void add_bullet(
        list_gui_item* list, const string &text,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    void add_pikmin_status_line(
        list_gui_item* list,
        pikmin_type* pik_type,
        const string &group_text,
        const string &idle_text,
        const string &field_text,
        const string &onion_text,
        const string &total_text,
        const string &new_text,
        const string &lost_text,
        bool is_single, bool is_totals
    );
    void calculate_go_here_path();
    void confirm_or_leave();
    button_gui_item* create_page_button(
        PAUSE_MENU_PAGE target_page, bool left, gui_manager* cur_gui
    );
    void create_page_buttons(PAUSE_MENU_PAGE cur_page, gui_manager* cur_gui);
    void draw_go_here_segment(
        const point &start, const point &end,
        const ALLEGRO_COLOR &color, float* texture_point
    );
    void draw_radar(const point &center, const point &size);
    void draw_tidbit(
        const ALLEGRO_FONT* const font, const point &where,
        const point &max_size, const string &text
    );
    void fill_mission_fail_list(list_gui_item* list);
    void fill_mission_grading_list(list_gui_item* list);
    string get_mission_goal_status();
    void init_confirmation_page();
    void init_radar_page();
    void init_help_page();
    void init_main_pause_menu();
    void init_mission_page();
    void init_status_page();
    void pan_radar(point amount);
    void populate_help_tidbits(const HELP_CATEGORY category);
    void radar_confirm();
    void start_closing(gui_manager* cur_gui);
    void start_leaving_gameplay();
    void switch_page(
        gui_manager* cur_gui, PAUSE_MENU_PAGE new_page, bool left
    );
    void update_radar_transformations(
        const point &radar_center, const point &radar_size
    );
    void zoom_radar(float amount);
    void zoom_radar_with_mouse(
        float amount, const point &radar_center, const point &radar_size
    );
    
};


#endif //ifndef PAUSE_MENU_INCLUDED
