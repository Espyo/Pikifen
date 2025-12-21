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
extern const float ENTRY_LOCKOUT_TIME;
extern const float GO_HERE_CALC_INTERVAL;
extern const string GUI_FILE_PATH;
extern const string MISSION_GUI_FILE_PATH;
extern const float MISSION_MOB_MARKER_TIME_MULT;
extern const float MISSION_MOB_MARKER_SIZE;
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
        
        //The radar cursor position.
        GO_HERE_SEGMENT_SPOT_CURSOR,
        
        //A path stop.
        GO_HERE_SEGMENT_SPOT_STOP,
        
    };
    
    
    //--- Members ---
    
    //GUI manager for the main pause menu.
    GuiManager gui;
    
    //GUI manager for the radar page.
    GuiManager radarGui;
    
    //GUI manager for the status page.
    GuiManager statusGui;
    
    //GUI manager for the mission page.
    GuiManager missionGui;
    
    //Multiply the background alpha by this much.
    float bgAlphaMult = 0.0f;
    
    //Control lockout time left for when the menu opens.
    float openingLockoutTimer = 0.0f;
    
    //Time left until the menu finishes closing.
    float closingTimer = 0.0f;
    
    //Is the struct meant to be deleted?
    bool toDelete = false;
    
    
    //--- Function declarations ---
    
    explicit PauseMenu(bool startOnRadar);
    ~PauseMenu();
    void draw();
    void handleAllegroEvent(const ALLEGRO_EVENT& ev);
    void handlePlayerAction(const Inpution::Action& action);
    void tick(float deltaT);
    
private:

    //--- Members ---
    
    //Is it currently closing?
    bool closing = false;
    
    //Radar GUI item.
    GuiItem* radarItem = nullptr;
    
    //Pikmin status list.
    ListGuiItem* pikminList = nullptr;
    
    //Left page buttons for each GUI.
    map<GuiManager*, ButtonGuiItem*> leftPageButtons;
    
    //Right page buttons for each GUI.
    map<GuiManager*, ButtonGuiItem*> rightPageButtons;
    
    //Where the player intends to go by leaving.
    GAMEPLAY_LEAVE_TARGET leaveTarget = GAMEPLAY_LEAVE_TARGET_AREA_SELECT;
    
    //Pages available, in order.
    vector<PAUSE_MENU_PAGE> pages;
    
    //Information about the current secondary menu, if any.
    Menu* secondaryMenu = nullptr;
    
    //Z of the lowest sector.
    float lowestSectorZ = 0.0f;
    
    //Z of the highest sector.
    float highestSectorZ = 0.0f;
    
    //Radar viewport information.
    Viewport radarView;
    
    //Location of the radar cursor, in world coordinates.
    Point radarCursor;
    
    //Whether a mouse button is being held in the radar.
    bool radarMouseDown = false;
    
    //Point where the mouse button was first held in the radar (window coords).
    Point radarMouseDownPoint;
    
    //Whether the player is dragging the mouse. False for just a (fuzzy) click.
    bool radarMouseDragging = false;
    
    //Minimum coordinates the radar can pan to.
    Point radarMinCoords;
    
    //Maximum coordinates the radar can pan to.
    Point radarMaxCoords;
    
    //Icon for the radar cursor.
    ALLEGRO_BITMAP* bmpRadarCursor = nullptr;
    
    //Icon for a Pikmin in the radar.
    ALLEGRO_BITMAP* bmpRadarPikmin = nullptr;
    
    //Icon for a treasure in the radar.
    ALLEGRO_BITMAP* bmpRadarTreasure = nullptr;
    
    //Icon for a living enemy in the radar.
    ALLEGRO_BITMAP* bmpRadarEnemyAlive = nullptr;
    
    //Icon for a dead enemy in the radar.
    ALLEGRO_BITMAP* bmpRadarEnemyDead = nullptr;
    
    //Bubble that surrounds a leader's icon in the radar.
    ALLEGRO_BITMAP* bmpRadarLeaderBubble = nullptr;
    
    //X imposed on a KO'd leader's icon.
    ALLEGRO_BITMAP* bmpRadarLeaderX = nullptr;
    
    //Icon representing a generic obstacle that blocks paths.
    ALLEGRO_BITMAP* bmpRadarObstacle = nullptr;
    
    //Skeleton part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmpRadarOnionSkeleton = nullptr;
    
    //Bulb part of an Onion's icon in the radar.
    ALLEGRO_BITMAP* bmpRadarOnionBulb = nullptr;
    
    //Icon for a ship in the radar.
    ALLEGRO_BITMAP* bmpRadarShip = nullptr;
    
    //Texture for a path in the radar.
    ALLEGRO_BITMAP* bmpRadarPath = nullptr;
    
    //Selected leader in the radar.
    Mob* radarSelectedLeader = nullptr;
    
    //Leader under the radar cursor.
    Mob* radarCursorLeader = nullptr;
    
    //Time left before another Go Here calculation.
    float goHereCalcTime = 0.0f;
    
    //Go Here path.
    vector<PathStop*> goHerePath;
    
    //Go Here path result.
    PATH_RESULT goHerePathResult = PATH_RESULT_NOT_CALCULATED;
    
    //Radar pan inputs direction and amount.
    MovementInfo radarPan;
    
    //Radar zoom inputs direction and amount.
    MovementInfo radarZoom;
    
    
    //--- Function declarations ---
    
    void addBullet(
        ListGuiItem* list, const string& text,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    void addPikminStatusLine(
        ListGuiItem* list,
        PikminType* pikType,
        const string& groupText,
        const string& idleText,
        const string& fieldText,
        const string& onionText,
        const string& totalText,
        const string& newText,
        const string& lostText,
        bool isSingle, bool isTotals
    );
    void calculateGoHerePath();
    void confirmOrLeave();
    ButtonGuiItem* createPageButton(
        PAUSE_MENU_PAGE targetPage, bool left, GuiManager* curGui
    );
    void createPageButtons(PAUSE_MENU_PAGE curPage, GuiManager* curGui);
    void drawGoHereSegment(
        const Point& start, const Point& end,
        const ALLEGRO_COLOR& color, float* texturePoint
    );
    void drawRadar(const Point& center, const Point& size);
    void fillMissionFailList(ListGuiItem* list);
    void fillMissionGradingList(ListGuiItem* list);
    string getMissionGoalStatus();
    void initRadarPage();
    void initMainPauseMenu();
    void initMissionPage();
    void initStatusPage();
    void panRadar(Point amount);
    void radarConfirm();
    void startClosing(GuiManager* curGui);
    void startLeavingGameplay();
    void switchPage(
        GuiManager* curGui, PAUSE_MENU_PAGE newPage, bool left
    );
    void zoomRadar(float amount);
    void zoomRadarWithMouse(
        float amount, const Point& radarCenter, const Point& radarSize
    );
    
};
