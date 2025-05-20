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

#pragma once

#include <map>

#include "../../core/drawing.h"

using std::map;


using DrawInfo = GuiItem::DrawInfo;


//Methods for a HUD bubble to move.
enum HUD_BUBBLE_MOVE_METHOD {

    //In a straight line.
    HUD_BUBBLE_MOVE_METHOD_STRAIGHT,
    
    //In a circular path.
    HUD_BUBBLE_MOVE_METHOD_CIRCLE,
    
};


/**
 * @brief Manages the contents of "bubbles" in the HUD that have the ability to
 * move around, or fade in/out of existence, depending on what the player
 * swapped, and how.
 *
 * I'm calling these "bubbles" because this slide/shrink/grow behavior is
 * typically used by HUD items that are drawn inside some bubble.
 * When a transition happens, in the first half, bubbles use their old data,
 * and for the second half, the new data.
 * So the actual GUI item that contains a given thing depends on the half
 * of the transition. For thing X, for the first half, it's the old GUI item
 * that is in charge of showing it moving. For the second half, it's the new
 * GUI item.

 * @tparam ContentT Type of content the bubble holds.
 */
template<typename ContentT>
struct HudBubbleManager {

    public:
    
    //--- Misc. declarations ---
    
    /**
     * @brief Represents a bubble GUI item.
     */
    struct Bubble {
    
        //--- Members ---
        
        //GUI item.
        GuiItem* bubble = nullptr;
        
        //Reference to base its existence off of.
        void* ref = nullptr;
        
        //Content that it holds.
        ContentT content = ContentT();
        
        //Reference pre-transition.
        void* preTransitionRef = nullptr;
        
        //Content that it held, pre-transition.
        ContentT preTransitionContent = ContentT();
        
        
        //--- Function definitions ---
        
        /**
         * @brief Constructs a new bubble info object.
         *
         * @param bubble The bubble GUI item.
         */
        explicit Bubble(GuiItem* bubble = nullptr) :
            bubble(bubble) {
        }
        
    };
    
    
    //--- Members ---
    
    //GUI manager the HUD belongs to.
    GuiManager* hud = nullptr;
    
    //How long a transition lasts for.
    float transitionDuration = 0.0f;
    
    //How to move the bubbles around during a transition.
    HUD_BUBBLE_MOVE_METHOD moveMethod = HUD_BUBBLE_MOVE_METHOD_STRAIGHT;
    
    
    //--- Function definitions ---
    
    /**
     * @brief Constructs a new HUD bubble manager object.
     *
     * @param hud The HUD manager it belongs to.
     */
    explicit HudBubbleManager(GuiManager* hud) :
        hud(hud) {
        
    }
    
    /**
    * @brief Returns the necessary information for the bubble to know how
    * to draw itself.
    *
    * @param id ID of the registered bubble.
    * @param content The content the bubble should use is returned here.
    * A default-constructed object is returned on error.
    * @param draw The final drawing information it should use is returned here.
    */
    void getDrawingInfo(
        size_t id,
        ContentT* content, DrawInfo* draw
    ) {
        float transitionAnimRatio = transitionTimer / transitionDuration;
        
        auto it = bubbles.find(id);
        if(it == bubbles.end()) {
            *content = ContentT();
            return;
        }
        
        bool visible = hud->getItemDrawInfo(it->second.bubble, draw);
        
        if(!visible) {
            *content = ContentT();
            return;
        }
        
        typename map<size_t, Bubble>::iterator matchIt;
        GuiItem* matchPtr = nullptr;
        Point matchPos;
        Point matchSize;
        
        //First, check if there's any matching bubble that
        //we can move to/from.
        for(
            matchIt = bubbles.begin();
            matchIt != bubbles.end();
            ++matchIt
        ) {
            if(
                transitionAnimRatio > 0.5f &&
                it->second.preTransitionRef &&
                matchIt->second.ref ==
                it->second.preTransitionRef
            ) {
                //In the first half of the animation, we want to search
                //for a bubble that has the contents that our bubble
                //had pre-transition.
                break;
            }
            if(
                transitionAnimRatio <= 0.5f &&
                it->second.ref &&
                matchIt->second.preTransitionRef ==
                it->second.ref
            ) {
                //In the second half, the match had the contents that
                //our bubble has.
                break;
            }
        }
        
        if(matchIt != bubbles.end()) {
            matchPtr = matchIt->second.bubble;
            DrawInfo matchDraw;
            if(
                hud->getItemDrawInfo(matchPtr, &matchDraw)
            ) {
                matchPos = matchDraw.center;
                matchSize = matchDraw.size;
            } else {
                matchPtr = nullptr;
            }
        }
        
        //Figure out how to animate it, if we even should animate it.
        if(matchPtr) {
            //This bubble is heading to a new spot.
            
            Point matchPivot(
                (draw->center.x + matchPos.x) / 2.0f,
                (draw->center.y + matchPos.y) / 2.0f
            );
            float movRatio =
                ease(EASE_METHOD_IN_OUT_BACK, 1.0f - transitionAnimRatio);
            float pivotDist =
                Distance(draw->center, matchPivot).toFloat();
                
            if(transitionAnimRatio > 0.5f) {
                //First half of the animation. Move to the first half.
                switch(moveMethod) {
                case HUD_BUBBLE_MOVE_METHOD_STRAIGHT: {
                    draw->center =
                        interpolatePoint(
                            ease(EASE_METHOD_OUT, 1.0f - transitionAnimRatio),
                            0.0f, 1.0f,
                            draw->center, matchPos
                        );
                    break;
                } case HUD_BUBBLE_MOVE_METHOD_CIRCLE: {
                    float matchStartAngle = getAngle(matchPivot, draw->center);
                    draw->center =
                        matchPivot +
                        rotatePoint(
                            Point(pivotDist, 0.0f),
                            matchStartAngle + movRatio * TAU / 2.0f
                        );
                    break;
                }
                }
                draw->size =
                    interpolatePoint(
                        ease(EASE_METHOD_OUT, 1.0f - transitionAnimRatio),
                        0.0f, 1.0f,
                        draw->size, matchSize
                    );
                    
            } else {
                //Second half of the animation. Move from the first half.
                switch(moveMethod) {
                case HUD_BUBBLE_MOVE_METHOD_STRAIGHT: {
                    draw->center =
                        interpolatePoint(
                            ease(EASE_METHOD_OUT, 1.0f - transitionAnimRatio),
                            0.0f, 1.0f,
                            matchPos, draw->center
                        );
                    break;
                } case HUD_BUBBLE_MOVE_METHOD_CIRCLE: {
                    float matchStartAngle = getAngle(matchPivot, matchPos);
                    draw->center =
                        matchPivot +
                        rotatePoint(
                            Point(pivotDist, 0.0f),
                            matchStartAngle + movRatio * TAU / 2.0f
                        );
                    break;
                }
                }
                draw->size =
                    interpolatePoint(
                        ease(EASE_METHOD_OUT, 1.0f - transitionAnimRatio),
                        0.0f, 1.0f,
                        matchSize, draw->size
                    );
                    
            }
            
        } else {
            //This bubble has no equivalent to go to.
            
            if(transitionAnimRatio > 0.5f) {
                //First half of the animation. Fade out.
                draw->size *=
                    ease(EASE_METHOD_OUT, (transitionAnimRatio - 0.5f) * 2.0f);
                
            } else {
                //Second half of the animation. Fade in.
                draw->size *=
                    ease(EASE_METHOD_OUT, 1 - transitionAnimRatio * 2.0f);
                
            }
        }
        
        //Set the content.
        if(transitionAnimRatio > 0.5f) {
            *content = it->second.preTransitionContent;
        } else {
            *content = it->second.content;
        }
    }
    
    
    /**
    * @brief Registers a bubble.
    *
    * @param id ID of this item in its "family". For instance, if
    * this is the icon for the second leader, this value is 1 (0-indexed).
    * @param bubble GUI item that represents this bubble.
    */
    void registerBubble(size_t id, GuiItem* bubble) {
        bubbles[id] = Bubble(bubble);
    }
    
    /**
    * @brief Ticks time by one frame of logic.
    *
    * @param deltaT How long the frame's tick is, in seconds.
    */
    void tick(float deltaT) {
        if(transitionTimer > 0.0f) {
            transitionTimer -= deltaT;
            transitionTimer = std::max(transitionTimer, 0.0f);
            transitionIsSetup = false;
        }
    }
    
    /**
    * @brief Updates the reference and content of a given bubble.
    *
    * @param id ID of the registered bubble.
    * @param newRef New reference.
    * @param newContent New content.
    */
    void update(size_t id, void* newRef, ContentT newContent) {
        auto it = bubbles.find(id);
        if(it == bubbles.end()) return;
        if(it->second.ref != newRef && !transitionIsSetup) {
            for(auto &b : bubbles) {
                b.second.preTransitionRef = b.second.ref;
                b.second.preTransitionContent = b.second.content;
            }
            transitionTimer = transitionDuration;
            transitionIsSetup = true;
        }
        it->second.ref = newRef;
        it->second.content = newContent;
    }
    
private:

    //--- Members ---
    
    //List of all registered bubble GUI items.
    map<size_t, Bubble> bubbles;
    
    //Time left in the current transition, or 0 if none.
    float transitionTimer = 0.0f;
    
    //Have we set each bubble's "pre-transition" class members yet?
    bool transitionIsSetup = false;
    
};
