/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pause menu class and related functions.
 */

#pragma once

#include <map>

#include "../../content/mob/mob_utils.h"
#include "../../content/other/gui.h"
#include "../../content/other/mob_script.h"
#include "../../core/drawing.h"
#include "../../menu/help_menu.h"
#include "../../util/drawing_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"

class PikminType;


namespace PAUSE_MENU {
extern const string CONFIRMATION_GUI_FILE_PATH;
extern const float ENTRY_LOCKOUT_TIME;
extern const float GO_HERE_CALC_INTERVAL;
extern const string GUI_FILE_PATH;
extern const string MISSION_GUI_FILE_PATH;
extern const float RADAR_DEF_ZOOM;
extern const string RADAR_GUI_FILE_PATH;
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
    
    //Leave in order to go to the area selection menu.
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
struct PauseMenu {

    public:
    
    //--- Misc. declarations ---
    
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
    GuiManager gui;
    
    //GUI manager for the radar page.
    GuiManager radar_gui;
    
    //GUI manager for the status page.
    GuiManager status_gui;
    
    //GUI manager for the mission page.
    GuiManager mission_gui;
    
    //GUI manager for the leaving confirmation page.
    GuiManager confirmation_gui;
    
    //Multiply the background alpha by this much.
    float bg_alpha_mult = 0.0f;
    
    //Control lockout time left for when the menu opens.
    float opening_lockout_timer = 0.0f;
    
    //Time left until the menu finishes closing.
    float closing_timer = 0.0f;
    
    //Is the struct meant to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    
    explicit PauseMenu(bool start_on_radar);
    ~PauseMenu();
    void draw();
    void handleAllegroEvent(const ALLEGRO_EVENT &ev);
    void handlePlayerAction(const PlayerAction &action);
    void tick(float delta_t);
    
private:

    //--- Members ---
    
    //Is it currently closing?
    bool closing = false;
    
    //Confirmation page explanation text.
    TextGuiItem* confirmation_explanation_text = nullptr;
    
    //Radar GUI item.
    GuiItem* radar_item = nullptr;
    
    //Pikmin status list.
    ListGuiItem* pikmin_list = nullptr;
    
    //Where the player intends to go by leaving.
    GAMEPLAY_LEAVE_TARGET leave_target = GAMEPLAY_LEAVE_TARGET_AREA_SELECT;
    
    //Pages available, in order.
    vector<PAUSE_MENU_PAGE> pages;
    
    //Information about the current secondary menu, if any.
    Menu* secondary_menu = nullptr;
    
    //Z of the lowest sector.
    float lowest_sector_z = 0.0f;
    
    //Z of the highest sector.
    float highest_sector_z = 0.0f;
    
    //World coordinates to radar screen coordinates transformation.
    ALLEGRO_TRANSFORM world_to_radar_screen_transform;
    
    //Radar screen coordinates to world coordinates transformation.
    ALLEGRO_TRANSFORM radar_screen_to_world_transform;
    
    //Radar camera information.
    Camera radar_cam;
    
    //Location of the radar cursor, in world coordinates.
    Point radar_cursor;
    
    //Whether a mouse button is being held in the radar.
    bool radar_mouse_down = false;
    
    //Point where the mouse button was first held in the radar (screen coords).
    Point radar_mouse_down_point;
    
    //Whether the player is dragging the mouse. False for just a (fuzzy) click.
    bool radar_mouse_dragging = false;
    
    //Minimum coordinates the radar can pan to.
    Point radar_min_coords;
    
    //Maximum coordinates the radar can pan to.
    Point radar_max_coords;
    
    //Icon for the radar cursor.
    ALLEGRO_BITMAP* bmp_radar_cursor = nullptr;
    
    //Icon for a Pikmin in the radar.
    ALLEGRO_BITMAP* bmp_radar_pikmin = nullptr;
    
    //Icon for a treasure in the radar.
    ALLEGRO_BITMAP* bmp_radar_treasure = nullptr;
    
    //Icon for a living enemy in the radar.
    ALLEGRO_BITMAP* bmp_radar_enemy_alive = nullptr;
    
    //Icon for a dead enemy in the radar.
    ALLEGRO_BITMAP* bmp_radar_enemy_dead = nullptr;
    
    //Bubble that surrounds a leader's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_leader_bubble = nullptr;
    
    //X imposed on a KO'd leader's icon.
    ALLEGRO_BITMAP* bmp_radar_leader_x = nullptr;
    
    //Icon representing a generic obstacle that blocks paths.
    ALLEGRO_BITMAP* bmp_radar_obstacle = nullptr;
    
    //Skeleton part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_onion_skeleton = nullptr;
    
    //Bulb part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmp_radar_onion_bulb = nullptr;
    
    //Icon for a ship in the radar.
    ALLEGRO_BITMAP* bmp_radar_ship = nullptr;
    
    //Texture for a path in the radar.
    ALLEGRO_BITMAP* bmp_radar_path = nullptr;
    
    //Selected leader in the radar.
    Mob* radar_selected_leader = nullptr;
    
    //Leader under the cursor in the radar.
    Mob* radar_cursor_leader = nullptr;
    
    //Time left before another Go Here calculation.
    float go_here_calc_time = 0.0f;
    
    //Go Here path.
    vector<PathStop*> go_here_path;
    
    //Go Here path result.
    PATH_RESULT go_here_path_result = PATH_RESULT_NOT_CALCULATED;
    
    //Pan speed and amount.
    MovementInfo radar_pan;
    
    //Whether the radar zoom-in input is pressed.
    bool radar_zoom_in = false;
    
    //Whether the radar zoom-out input is pressed.
    bool radar_zoom_out = false;
    
    
    //--- Function declarations ---
    
    void addBullet(
        ListGuiItem* list, const string &text,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    void addPikminStatusLine(
        ListGuiItem* list,
        PikminType* pik_type,
        const string &group_text,
        const string &idle_text,
        const string &field_text,
        const string &onion_text,
        const string &total_text,
        const string &new_text,
        const string &lost_text,
        bool is_single, bool is_totals
    );
    void calculateGoHerePath();
    void confirmOrLeave();
    ButtonGuiItem* createPageButton(
        PAUSE_MENU_PAGE target_page, bool left, GuiManager* cur_gui
    );
    void createPageButtons(PAUSE_MENU_PAGE cur_page, GuiManager* cur_gui);
    void drawGoHereSegment(
        const Point &start, const Point &end,
        const ALLEGRO_COLOR &color, float* texture_point
    );
    void drawRadar(const Point &center, const Point &size);
    void fillMissionFailList(ListGuiItem* list);
    void fillMissionGradingList(ListGuiItem* list);
    string getMissionGoalStatus();
    void initConfirmationPage();
    void initRadarPage();
    void initMainPauseMenu();
    void initMissionPage();
    void initStatusPage();
    void panRadar(Point amount);
    void radarConfirm();
    void startClosing(GuiManager* cur_gui);
    void startLeavingGameplay();
    void switchPage(
        GuiManager* cur_gui, PAUSE_MENU_PAGE new_page, bool left
    );
    void updateRadarTransformations(
        const Point &radar_center, const Point &radar_size
    );
    void zoomRadar(float amount);
    void zoomRadarWithMouse(
        float amount, const Point &radar_center, const Point &radar_size
    );
    
};
