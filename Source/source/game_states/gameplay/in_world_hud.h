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

#ifndef IN_WORLD_HUD_INCLUDED
#define IN_WORLD_HUD_INCLUDED

#include <allegro5/allegro.h>


//In-world HUD item transitions.
enum IN_WORLD_HUD_TRANSITIONS {
    //Not transitioning.
    IN_WORLD_HUD_TRANSITION_NONE,
    //Fading in.
    IN_WORLD_HUD_TRANSITION_IN,
    //Fading out.
    IN_WORLD_HUD_TRANSITION_OUT,
};


class mob;


/* ----------------------------------------------------------------------------
 * Information about some HUD item that is located in the game world.
 * Sort of. Instead of being in a fixed position on-screen, these follow
 * mobs around.
 */
class in_world_hud_item {
public:
    //Associated mob, if any.
    mob* m;
    //Current transition.
    IN_WORLD_HUD_TRANSITIONS transition;
    //Time left in the current transition, if any.
    float transition_timer;
    //Does it need to be deleted?
    bool to_delete;
    
    //Constructor.
    in_world_hud_item(mob* m);
    //Destructor.
    virtual ~in_world_hud_item() = default;
    //Draw the item.
    virtual void draw() = 0;
    //Start fading away.
    virtual void start_fading() = 0;
    //Tick.
    virtual void tick(const float delta_t);
};


/* ----------------------------------------------------------------------------
 * Information about a fraction in the game world, placed atop an enemy.
 */
class in_world_fraction : public in_world_hud_item {
public:
    static const float GROW_JUICE_DURATION;
    static const float GROW_JUICE_AMOUNT;
    static const float REQ_MET_JUICE_DURATION;
    static const float REQ_MET_GROW_JUICE_AMOUNT;
    static const float TRANSITION_IN_DURATION;
    static const float TRANSITION_OUT_DURATION;
    
    //Constructor.
    in_world_fraction(mob* m);
    //Draw the item.
    void draw();
    //Sets the color.
    void set_color(const ALLEGRO_COLOR &new_color);
    //Sets the requirement number.
    void set_requirement_number(const float new_req_nr);
    //Sets the value number.
    void set_value_number(const float new_value_nr);
    //Start fading away.
    void start_fading();
    //Tick.
    void tick(const float delta_t);
    
private:
    //Upper number, the one representing the current value.
    float value_number;
    //Lower number, the one representing the requirement.
    float requirement_number;
    //Color to use.
    ALLEGRO_COLOR color;
    //Value change growth juice timer. 0 means not animating.
    float grow_juice_timer;
    //Requirement met flash juice timer. 0 means not animating.
    float req_met_juice_timer;
};


/* ----------------------------------------------------------------------------
 * Information about a health wheel in the game world, placed atop an enemy.
 */
class in_world_health_wheel : public in_world_hud_item {
public:
    static const float SMOOTHNESS_MULT;
    static const float TRANSITION_IN_DURATION;
    static const float TRANSITION_OUT_DURATION;
    
    //How much the health wheel is filled. Gradually moves to the target amount.
    float visible_ratio;
    
    //Constructor.
    in_world_health_wheel(mob* m);
    //Draw the item.
    void draw();
    //Start fading away.
    void start_fading();
    //Tick.
    void tick(const float delta_t);
};


#endif //ifndef IN_WORLD_HUD_INCLUDED
