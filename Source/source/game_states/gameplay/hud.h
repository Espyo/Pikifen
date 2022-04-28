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
extern const float SUN_METER_SUN_SPIN_SPEED;
}


//Types of bubble GUI items that refer to a previous, current, and next thing.
enum BUBBLE_RELATIONS {
    BUBBLE_PREVIOUS = 0,
    BUBBLE_CURRENT = 1,
    BUBBLE_NEXT = 2,
};


/* ----------------------------------------------------------------------------
 * Holds information about the in-game HUD.
 */
struct hud_struct {
    static const string HUD_FILE_NAME;
    static const float LEADER_SWAP_JUICE_DURATION;
    static const float SPRAY_SWAP_JUICE_DURATION;
    static const float STANDBY_SWAP_JUICE_DURATION;
    static const float UNNECESSARY_ITEMS_FADE_IN_SPEED;
    static const float UNNECESSARY_ITEMS_FADE_OUT_DELAY;
    static const float UNNECESSARY_ITEMS_FADE_OUT_SPEED;
    
    //Bitmap and color of a leader's icon.
    struct leader_icon_bubble {
        //Leader icon bitmap.
        ALLEGRO_BITMAP* bmp;
        //Leader icon color.
        ALLEGRO_COLOR color;
        //Constructor.
        leader_icon_bubble() :
            bmp(NULL),
            color(COLOR_EMPTY) {}
    };
    
    //Health ratio and caution animation timer of a leader's health.
    struct leader_health_bubble {
        //Ratio of health left in the health wheel.
        float ratio;
        //Timer for the low-health caution animation.
        float caution_timer;
        //Constructor.
        leader_health_bubble() :
            ratio(0.0f),
            caution_timer(0.0f) {}
    };
    
    //GUI manager.
    gui_manager gui;
    //Bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_bubble;
    //Group counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_group;
    //Field counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_field;
    //Standby counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_standby;
    //Total counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_counter_bubble_total;
    //Day counter bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_day_bubble;
    //Distant Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_distant_pikmin_marker;
    //Hard bubble graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_hard_bubble;
    //No Pikmin marker graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_no_pikmin_bubble;
    //Sun icon graphic, used for the HUD.
    ALLEGRO_BITMAP* bmp_sun;
    //Bubble manager for leader icon items.
    hud_bubble_manager<leader_icon_bubble> leader_icon_mgr;
    //Bubble manager for leader health items.
    hud_bubble_manager<leader_health_bubble> leader_health_mgr;
    //Bubble manager for the standby type.
    hud_bubble_manager<ALLEGRO_BITMAP*> standby_icon_mgr;
    //Bubble manager for the spray icons.
    hud_bubble_manager<ALLEGRO_BITMAP*> spray_icon_mgr;
    //Opacity of the standby HUD items.
    float standby_items_opacity;
    //Time left before the standby items start fading out.
    float standby_items_fade_timer;
    //Opacity of the spray HUD items.
    float spray_items_opacity;
    //Time left before the spray items start fading out.
    float spray_items_fade_timer;
    //Spray 1 amount text. Cache for convenience.
    gui_item* spray_1_amount;
    //Spray 2 amount text. Cache for convenience.
    gui_item* spray_2_amount;
    //Current standby count.
    size_t standby_count_nr;
    //Standby count text. Cache for convenience.
    gui_item* standby_amount;
    //Current group count.
    size_t group_count_nr;
    //Group count text. Cache for convenience.
    gui_item* group_amount;
    //Current field count.
    size_t field_count_nr;
    //Field count text. Cache for convenience.
    gui_item* field_amount;
    //Current total count.
    size_t total_count_nr;
    //Total count text. Cache for convenience.
    gui_item* total_amount;
    
    hud_struct();
    ~hud_struct();
    void tick(const float delta_t);
    
private:
    void draw_standby_icon(BUBBLE_RELATIONS which);
    void draw_spray_icon(BUBBLE_RELATIONS which);
};



#endif //ifndef HUD_INCLUDED
