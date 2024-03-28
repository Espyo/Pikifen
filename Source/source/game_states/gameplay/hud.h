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

#ifndef HUD_INCLUDED
#define HUD_INCLUDED


#include "../../gui.h"
#include "hud_bubble_manager.h"


namespace HUD {
extern const float GOAL_INDICATOR_SMOOTHNESS_MULT;
extern const string GUI_FILE_NAME;
extern const float LEADER_SWAP_JUICE_DURATION;
extern const float MEDAL_ICON_SCALE;
extern const float MEDAL_ICON_SCALE_MULT;
extern const float MEDAL_ICON_SCALE_TIME_MULT;
extern const float SCORE_INDICATOR_SMOOTHNESS_MULT;
extern const int SCORE_RULER_RANGE;
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


/**
 * @brief Holds information about the in-game HUD.
 */
struct hud_t {

    //--- Misc. declarations ---

    /**
     * @brief Bitmap and color of a leader's icon.
     */
    struct leader_icon_bubble {

        //--- Members ---

        //Leader icon bitmap.
        ALLEGRO_BITMAP* bmp = nullptr;

        //Leader icon color.
        ALLEGRO_COLOR color = COLOR_EMPTY;

    };
    
    /**
     * @brief Health ratio and caution animation timer of a leader's health.
     */
    struct leader_health_bubble {

        //--- Members ---

        //Ratio of health left in the health wheel.
        float ratio = 0.0f;

        //Timer for the low-health caution animation.
        float caution_timer = 0.0f;

    };
    

    //--- Members ---

    //GUI manager.
    gui_manager gui;

    //Bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_bubble = nullptr;

    //Group counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_group = nullptr;

    //Field counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_field = nullptr;

    //Standby counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_standby = nullptr;

    //Total counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_total = nullptr;

    //Day counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_day_bubble = nullptr;

    //Distant Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_distant_pikmin_marker = nullptr;

    //Hard bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_hard_bubble = nullptr;

    //No Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_no_pikmin_bubble = nullptr;

    //Sun icon graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_sun = nullptr;

    //Bubble manager for leader icon items.
    hud_bubble_manager<leader_icon_bubble> leader_icon_mgr;

    //Bubble manager for leader health items.
    hud_bubble_manager<leader_health_bubble> leader_health_mgr;

    //Bubble manager for the standby type.
    hud_bubble_manager<ALLEGRO_BITMAP*> standby_icon_mgr;

    //Bubble manager for the spray icons.
    hud_bubble_manager<ALLEGRO_BITMAP*> spray_icon_mgr;

    //Opacity of the standby HUD items.
    float standby_items_opacity = 0.0f;

    //Time left before the standby items start fading out.
    float standby_items_fade_timer = 0.0f;

    //Opacity of the spray HUD items.
    float spray_items_opacity = 0.0f;

    //Time left before the spray items start fading out.
    float spray_items_fade_timer = 0.0f;

    //Standby type in the previous frame.
    subgroup_type* prev_standby_type = nullptr;

    //Maturity icon in the previous frame.
    ALLEGRO_BITMAP* prev_maturity_icon = nullptr;

    //Spray 1 amount text. Cache for convenience.
    gui_item* spray_1_amount = nullptr;

    //Spray 2 amount text. Cache for convenience.
    gui_item* spray_2_amount = nullptr;

    //Current standby count.
    size_t standby_count_nr = 0;

    //Standby count text. Cache for convenience.
    gui_item* standby_amount = nullptr;

    //Current group count.
    size_t group_count_nr = 0;

    //Group count text. Cache for convenience.
    gui_item* group_amount = nullptr;

    //Current field count.
    size_t field_count_nr = 0;

    //Field count text. Cache for convenience.
    gui_item* field_amount = nullptr;

    //Current total count.
    size_t total_count_nr = 0;

    //Total count text. Cache for convenience.
    gui_item* total_amount = nullptr;
    

    //--- Function declarations ---

    hud_t();
    ~hud_t();
    void tick(const float delta_t);
    
private:
    
    //--- Function declarations ---
    
    void create_mission_fail_cond_items(const bool primary);
    void draw_standby_icon(BUBBLE_RELATION which);
    void draw_spray_icon(BUBBLE_RELATION which);
    
};



#endif //ifndef HUD_INCLUDED
