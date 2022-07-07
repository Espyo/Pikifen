/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the in-game HUD "bubble manager" class and
 * in-game HUD "bubble manager"-related functions.
 */

#ifndef HUD_BUBBLE_MANAGER_INCLUDED
#define HUD_BUBBLE_MANAGER_INCLUDED

#include <map>

#include "../../drawing.h"

using std::map;


enum HUD_BUBBLE_MOVE_METHODS {
    HUD_BUBBLE_MOVE_METHOD_STRAIGHT,
    HUD_BUBBLE_MOVE_METHOD_CIRCLE,
};


/* ----------------------------------------------------------------------------
 * Manages the contents of "bubbles" in the HUD that have the ability to move
 * around, or fade in/out of existence, depending on what the player swapped,
 * and how.
 * I'm calling these "bubbles" because this slide/shrink/grow behavior is
 * typically used by HUD items that are drawn inside some bubble.
 * When a transition happens, in the first half, bubbles use their old data,
 * and for the second half, the new data.
 * So the actual GUI item that contains a given thing depends on the half
 * of the transition. For thing X, for the first half, it's the old GUI item
 * that is in charge of showing it moving. For the second half, it's the new
 * GUI item.
 */
template<typename t>
struct hud_bubble_manager {
public:
    //Represents a bubble GUI item.
    struct bubble_info {
        //GUI item.
        gui_item* bubble;
        //Reference to base its existence off of.
        void* ref;
        //Content that it holds.
        t content;
        //Reference pre-transition.
        void* pre_transition_ref;
        //Content that it held, pre-transition.
        t pre_transition_content;
        
        /* ---------------------------------------------------------------------
        * Creates a HUD bubble manager bubble info instance.
        */
        bubble_info(gui_item* bubble = NULL) :
            bubble(bubble),
            ref(NULL),
            content(),
            pre_transition_ref(NULL),
            pre_transition_content() {
        }
    };
    
    //GUI manager the HUD belongs to.
    gui_manager* hud;
    //How long a transition lasts for.
    float transition_duration;
    //How to move the bubbles around during a transition.
    HUD_BUBBLE_MOVE_METHODS move_method;
    
    /* -------------------------------------------------------------------------
    * Creates a HUD bubble manager instance.
    */
    explicit hud_bubble_manager(gui_manager* hud) :
        hud(hud),
        transition_duration(0.0f),
        move_method(HUD_BUBBLE_MOVE_METHOD_STRAIGHT),
        transition_timer(0.0f),
        transition_is_setup(false) {
        
    }
    
    /* -------------------------------------------------------------------------
    * Returns the necessary information for the bubble to know how
    * to draw itself.
    * number:
    *   Number of the registered bubble.
    * content:
    *   The content the bubble should use is returned here.
    *   A default-constructed object is returned on error.
    * pos:
    *   The final position it should use is returned here.
    * size:
    *   The final size it should use is returned here.
    */
    void get_drawing_info(
        const size_t number,
        t* content, point* pos, point* size
    ) {
        float transition_anim_ratio = transition_timer / transition_duration;
        
        auto it = bubbles.find(number);
        if(it == bubbles.end()) {
            *content = t();
            return;
        }
        
        bool visible =
            hud->get_item_draw_info(
                it->second.bubble, pos, size
            );
            
        if(!visible) {
            *content = t();
            return;
        }
        
        typename map<size_t, bubble_info>::iterator match_it;
        gui_item* match_ptr = NULL;
        point match_pos;
        point match_size;
        
        //First, check if there's any matching bubble that
        //we can move to/from.
        for(
            match_it = bubbles.begin();
            match_it != bubbles.end();
            ++match_it
        ) {
            if(
                transition_anim_ratio > 0.5f &&
                it->second.pre_transition_ref &&
                match_it->second.ref ==
                it->second.pre_transition_ref
            ) {
                //In the first half of the animation, we want to search
                //for a bubble that has the contents that our bubble
                //had pre-transition.
                break;
            }
            if(
                transition_anim_ratio <= 0.5f &&
                it->second.ref &&
                match_it->second.pre_transition_ref ==
                it->second.ref
            ) {
                //In the second half, the match had the contents that
                //our bubble has.
                break;
            }
        }
        
        if(match_it != bubbles.end()) {
            match_ptr = match_it->second.bubble;
            if(
                !hud->get_item_draw_info(
                    match_ptr, &match_pos, &match_size
                )
            ) {
                match_ptr = NULL;
            }
        }
        
        //Figure out how to animate it, if we even should animate it.
        if(match_ptr) {
            //This bubble is heading to a new spot.
            
            point match_pivot(
                (pos->x + match_pos.x) / 2.0f,
                (pos->y + match_pos.y) / 2.0f
            );
            float mov_ratio =
                ease(EASE_IN_ELASTIC, 1.0f - transition_anim_ratio);
            float pivot_dist =
                dist(*pos, match_pivot).to_float();
                
            if(transition_anim_ratio > 0.5f) {
                //First half of the animation. Move to the first half.
                switch(move_method) {
                case HUD_BUBBLE_MOVE_METHOD_STRAIGHT: {
                    *pos =
                        interpolate_point(
                            ease(EASE_OUT, 1.0f - transition_anim_ratio),
                            0.0f, 1.0f,
                            *pos, match_pos
                        );
                    break;
                } case HUD_BUBBLE_MOVE_METHOD_CIRCLE: {
                    float match_start_angle = get_angle(match_pivot, *pos);
                    *pos =
                        match_pivot +
                        rotate_point(
                            point(pivot_dist, 0.0f),
                            match_start_angle + mov_ratio * TAU / 2.0f
                        );
                    break;
                }
                }
                *size =
                    interpolate_point(
                        ease(EASE_OUT, 1.0f - transition_anim_ratio),
                        0.0f, 1.0f,
                        *size, match_size
                    );
                    
            } else {
                //Second half of the animation. Move from the first half.
                switch(move_method) {
                case HUD_BUBBLE_MOVE_METHOD_STRAIGHT: {
                    *pos =
                        interpolate_point(
                            ease(EASE_OUT, 1.0f - transition_anim_ratio),
                            0.0f, 1.0f,
                            match_pos, *pos
                        );
                    break;
                } case HUD_BUBBLE_MOVE_METHOD_CIRCLE: {
                    float match_start_angle = get_angle(match_pivot, match_pos);
                    *pos =
                        match_pivot +
                        rotate_point(
                            point(pivot_dist, 0.0f),
                            match_start_angle + mov_ratio * TAU / 2.0f
                        );
                    break;
                }
                }
                *size =
                    interpolate_point(
                        ease(EASE_OUT, 1.0f - transition_anim_ratio),
                        0.0f, 1.0f,
                        match_size, *size
                    );
                    
            }
            
        } else {
            //This bubble has no equivalent to go to.
            
            if(transition_anim_ratio > 0.5f) {
                //First half of the animation. Fade out.
                *size *= ease(EASE_OUT, (transition_anim_ratio - 0.5f) * 2.0f);
                
            } else {
                //Second half of the animation. Fade in.
                *size *= ease(EASE_OUT, 1 - transition_anim_ratio * 2.0f);
                
            }
        }
        
        //Set the content.
        if(transition_anim_ratio > 0.5f) {
            *content = it->second.pre_transition_content;
        } else {
            *content = it->second.content;
        }
    }
    
    
    /* -------------------------------------------------------------------------
    * Registers a bubble.
    * bubble:
    *   GUI item that represents this bubble.
    * number:
    *   Number of this item in its "family". For instance, if this is the icon
    *   for the second leader, this value is 1 (0-indexed).
    */
    void register_bubble(const size_t number, gui_item* bubble) {
        bubbles[number] = bubble_info(bubble);
    }
    
    /* -------------------------------------------------------------------------
    * Ticks time by one frame of logic.
    * delta_t:
    *   How long the frame's tick is, in seconds.
    */
    void tick(const float delta_t) {
        if(transition_timer > 0.0f) {
            transition_timer -= delta_t;
            transition_timer = std::max(transition_timer, 0.0f);
            transition_is_setup = false;
        }
    }
    
    /* -------------------------------------------------------------------------
    * Updates the reference and content of a given bubble.
    * number:
    *   Number of the registered bubble.
    * new_ref:
    *   New reference.
    * new_content:
    *   New content.
    */
    void update(const size_t number, void* new_ref, t new_content) {
        auto it = bubbles.find(number);
        if(it == bubbles.end()) return;
        if(it->second.ref != new_ref && !transition_is_setup) {
            for(auto &b : bubbles) {
                b.second.pre_transition_ref = b.second.ref;
                b.second.pre_transition_content = b.second.content;
            }
            transition_timer = transition_duration;
            transition_is_setup = true;
        }
        it->second.ref = new_ref;
        it->second.content = new_content;
    }
    
private:
    //List of all registered bubble GUI items.
    map<size_t, bubble_info> bubbles;
    //Time left in the current transition, or 0 if none.
    float transition_timer;
    //Have we set each bubble's "pre-transition" class members yet?
    bool transition_is_setup;
    
};


#endif //ifndef HUD_BUBBLE_MANAGER_INCLUDED
