/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the in-world HUD class and
 * in-world HUD related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../core/const.h"
#include "../../util/drawing_utils.h"


namespace IN_WORLD_FRACTION {
extern const float GROW_JUICE_DURATION;
extern const float GROW_JUICE_AMOUNT;
extern const float PADDING;
extern const float REQ_MET_GROW_JUICE_AMOUNT;
extern const float REQ_MET_JUICE_DURATION;
extern const float ROW_HEIGHT;
extern const float TRANSITION_IN_DURATION;
extern const float TRANSITION_OUT_DURATION;
}


namespace IN_WORLD_HEALTH_WHEEL {
extern const float OPACITY;
extern const float PADDING;
extern const float SMOOTHNESS_MULT;
extern const float TRANSITION_IN_DURATION;
extern const float TRANSITION_OUT_DURATION;
}


//In-world HUD item transitions.
enum IN_WORLD_HUD_TRANSITION {

    //Not transitioning.
    IN_WORLD_HUD_TRANSITION_NONE,
    
    //Fading in.
    IN_WORLD_HUD_TRANSITION_IN,
    
    //Fading out.
    IN_WORLD_HUD_TRANSITION_OUT,
    
};


class Mob;


/**
 * @brief Info about some HUD item that is located in the game world.
 * Sort of. Instead of being in a fixed position on-screen, these follow
 * mobs around.
 */
class InWorldHudItem {

public:

    //--- Members ---
    
    //Associated mob, if any.
    Mob* m = nullptr;
    
    //Current transition.
    IN_WORLD_HUD_TRANSITION transition = IN_WORLD_HUD_TRANSITION_IN;
    
    //Time left in the current transition, if any.
    float transition_timer = 0.0f;
    
    //Does it need to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    
    InWorldHudItem(Mob* m);
    virtual ~InWorldHudItem() = default;
    virtual void draw() = 0;
    virtual void startFading() = 0;
    virtual void tick(float delta_t);
    
};


/**
 * @brief Info about a fraction in the game world, placed atop an enemy.
 */
class InWorldFraction : public InWorldHudItem {

public:

    //--- Function declarations ---
    
    explicit InWorldFraction(Mob* m);
    void draw() override;
    void setColor(const ALLEGRO_COLOR &new_color);
    void setRequirementNumber(float new_req_nr);
    void setValueNumber(float new_value_nr);
    void startFading() override;
    void tick(float delta_t) override;
    
private:

    //--- Members ---
    
    //Upper number, the one representing the current value.
    float value_number = 0.0f;
    
    //Lower number, the one representing the requirement.
    float requirement_number = 0.0f;
    
    //Color to use.
    ALLEGRO_COLOR color = COLOR_BLACK;
    
    //Value change growth juice timer. 0 means not animating.
    float grow_juice_timer = 0.0f;
    
    //Requirement met flash juice timer. 0 means not animating.
    float req_met_juice_timer = 0.0f;
    
};


/**
 * @brief Info about a health wheel in the game world, placed
 * atop an enemy.
 */
class InWorldHealthWheel : public InWorldHudItem {

public:

    //--- Members ---
    
    //How much the health wheel is filled. Gradually moves to the target amount.
    float visible_ratio = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit InWorldHealthWheel(Mob* m);
    void draw() override;
    void startFading() override;
    void tick(float delta_t) override;
    
};
