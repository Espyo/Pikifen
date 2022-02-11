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
 * Information about a health wheel in the game world, placed atop an enemy.
 */
class in_world_health_wheel : public in_world_hud_item {
public:
    static const float SMOOTHNESS_MULT;
    static const float TRANSITION_IN_TIME;
    static const float TRANSITION_OUT_TIME;
    
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
