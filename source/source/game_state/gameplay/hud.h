/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the in-game HUD class and
 * in-game HUD-related functions.
 */

#pragma once

#include "../../content/other/gui.h"
#include "../../util/drawing_utils.h"
#include "hud_bubble_manager.h"


namespace HUD {
extern const float GOAL_INDICATOR_SMOOTHNESS_MULT;
extern const string GUI_FILE_NAME;
extern const float HEALTH_SHAKE_MAX_OFFSET;
extern const float LEADER_SWAP_JUICE_DURATION;
extern const float MEDAL_GOT_IT_JUICE_DURATION;
extern const float MEDAL_ICON_SCALE_CUR;
extern const float MEDAL_ICON_SCALE_MULT;
extern const float MEDAL_ICON_SCALE_NEXT;
extern const float MEDAL_ICON_SCALE_TIME_MULT;
extern const float SCORE_INDICATOR_SMOOTHNESS_MULT;
extern const float SCORE_RULER_RATIO_RANGE;
extern const float SPRAY_SWAP_JUICE_DURATION;
extern const float STANDBY_SWAP_JUICE_DURATION;
extern const float SUN_METER_SUN_SPIN_SPEED;
extern const float UNNECESSARY_ITEMS_FADE_IN_SPEED;
extern const float UNNECESSARY_ITEMS_FADE_OUT_DELAY;
extern const float UNNECESSARY_ITEMS_FADE_OUT_SPEED;
}


//Types of bubble GUI items that refer to a previous, current, and next thing.
enum BUBBLE_RELATION {

    //Previous.
    BUBBLE_RELATION_PREVIOUS,
    
    //Current item.
    BUBBLE_RELATION_CURRENT,
    
    //Next.
    BUBBLE_RELATION_NEXT,
    
};


struct Player;


/**
 * @brief Holds information about the in-game HUD.
 */
struct Hud {

    //--- Misc. declarations ---
    
    /**
     * @brief Bitmap and color of a leader's icon.
     */
    struct LeaderIconBubble {
    
        //--- Members ---
        
        //Leader icon bitmap.
        ALLEGRO_BITMAP* bmp = nullptr;
        
        //Leader icon color.
        ALLEGRO_COLOR color = COLOR_EMPTY;
        
    };
    
    /**
     * @brief Health ratio and caution animation timer of a leader's health.
     */
    struct LeaderHealthBubble {
    
        //--- Members ---
        
        //Ratio of health left in the health wheel.
        float ratio = 0.0f;
        
        //Timer for the low-health caution animation.
        float cautionTimer = 0.0f;
        
        //Offset, for when the leader takes damage.
        Point offset;
        
        //Redness amount, for when the leader takes damage. 0 to 1.
        float redness = 0.0f;
        
    };
    
    
    //--- Members ---
    
    //GUI manager.
    GuiManager gui;
    
    //Whose player this HUD belongs to.
    Player* player = nullptr;
    
    //Bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpBubble = nullptr;
    
    //Group counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpCounterBubbleGroup = nullptr;
    
    //Field counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpCounterBubbleField = nullptr;
    
    //Standby counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpCounterBubbleStandby = nullptr;
    
    //Total counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpCounterBubbleTotal = nullptr;
    
    //Day counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpDayBubble = nullptr;
    
    //Distant Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpDistantPikminMarker = nullptr;
    
    //Hard bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpHardBubble = nullptr;
    
    //No Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpNoPikminBubble = nullptr;
    
    //Sun icon graphic, used for the HUD.
    ALLEGRO_BITMAP* bmpSun = nullptr;
    
    //Bubble manager for leader icon items.
    HudBubbleManager<LeaderIconBubble> leaderIconMgr;
    
    //Bubble manager for leader health items.
    HudBubbleManager<LeaderHealthBubble> leaderHealthMgr;
    
    //Bubble manager for the standby type.
    HudBubbleManager<ALLEGRO_BITMAP*> standbyIconMgr;
    
    //Bubble manager for the spray icons.
    HudBubbleManager<ALLEGRO_BITMAP*> sprayIconMgr;
    
    //Opacity of the standby HUD items.
    float standbyItemsOpacity = 0.0f;
    
    //Time left before the standby items start fading out.
    float standbyItemsFadeTimer = 0.0f;
    
    //Opacity of the spray HUD items.
    float sprayItemsOpacity = 0.0f;
    
    //Time left before the spray items start fading out.
    float sprayItemsFadeTimer = 0.0f;
    
    //Standby type in the previous frame.
    SubgroupType* prevStandbyType = nullptr;
    
    //Maturity icon in the previous frame.
    ALLEGRO_BITMAP* prevMaturityIcon = nullptr;
    
    //Current spray 1 count.
    size_t spray1CountNr = 0;
    
    //Current spray 2 count.
    size_t spray2CountNr = 0;
    
    //Spray 1 amount text. Cache for convenience.
    GuiItem* spray1Amount = nullptr;
    
    //Spray 2 amount text. Cache for convenience.
    GuiItem* spray2Amount = nullptr;
    
    //Current standby count.
    size_t standbyCountNr = 0;
    
    //Standby count text. Cache for convenience.
    GuiItem* standbyAmount = nullptr;
    
    //Current group count.
    size_t groupCountNr = 0;
    
    //Group count text. Cache for convenience.
    GuiItem* groupAmount = nullptr;
    
    //Current field count.
    size_t fieldCountNr = 0;
    
    //Field count text. Cache for convenience.
    GuiItem* fieldAmount = nullptr;
    
    //Current total count.
    size_t totalCountNr = 0;
    
    //Total count text. Cache for convenience.
    GuiItem* totalAmount = nullptr;
    
    
    //--- Function declarations ---
    
    Hud();
    ~Hud();
    void tick(float deltaT);
    
private:

    //--- Function declarations ---
    
    void createMissionFailCondItems(bool primary);
    void drawStandbyIcon(BUBBLE_RELATION which);
    void drawSprayIcon(BUBBLE_RELATION which);
    
};
