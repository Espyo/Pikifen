/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin type class and Pikmin type-related functions.
 */

#include "pikmin_type.h"

#include "const.h"
#include "leader.h"
#include "mob_script.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin type.
 */
pikmin_type::pikmin_type() {
    // TODO
    attack_attribute = 0;
    carry_strength = 1;
    attack_power = 1;
    weight = 1;
    carry_speed = 1;
    attack_interval = 0.8;
    size = DEF_PIKMIN_SIZE;
    throw_height_mult = 1.0;
    has_onion = true;
    can_dig = false;
    can_fly = false;
    can_swim = false;
    can_latch = true;
    can_carry_bomb_rocks = false;
    bmp_top[0] = NULL;
    bmp_top[1] = NULL;
    bmp_top[2] = NULL;
    bmp_icon[0] = NULL;
    bmp_icon[1] = NULL;
    bmp_icon[2] = NULL;
    
    init_script();
}


void pikmin_type::init_script() {

    states.push_back(new mob_state("buried", PIKMIN_STATE_BURIED));
    //Burrowed state.
    {
    
        vector<mob_event*> events;
        
        //On enter.
        events.push_back(new mob_event(MOB_EVENT_ON_ENTER));
        {
            vector<mob_action*> actions;
            actions.push_back(new mob_action(pikmin::become_buried));
            events.back()->actions = actions;
        }
        
        //Plucked event.
        events.push_back(new mob_event(MOB_EVENT_PLUCKED));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::be_plucked));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_chasing");
            
            events.back()->actions = actions;
            
        }
        states.back()->events = events;
        
    }
    
    //In group, chasing leader state.
    states.push_back(new mob_state("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING));
    {
    
        vector<mob_event*> events;
        
        //On enter.
        events.push_back(new mob_event(MOB_EVENT_ON_ENTER));
        {
            vector<mob_action*> actions;
            actions.push_back(new mob_action(pikmin::chase_leader));
            events.back()->actions = actions;
        }
        
        //Grabbed by friend event.
        events.push_back(new mob_event(MOB_EVENT_GRABBED_BY_FRIEND));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::be_grabbed_by_friend));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("grabbed_by_leader");
            
            events.back()->actions = actions;
            
        }
        
        //Leader is near event.
        events.push_back(new mob_event(MOB_EVENT_LEADER_IS_NEAR));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_stopped");
            
            events.back()->actions = actions;
            
        }
        
        //Dismissed event.
        events.push_back(new mob_event(MOB_EVENT_DISMISSED));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::be_dismissed));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("idle");
            
            events.back()->actions = actions;
        }
        
        states.back()->events = events;
        
    }
    
    //In group, stopped state.
    states.push_back(new mob_state("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED));
    {
    
        vector<mob_event*> events;
        
        //On enter.
        events.push_back(new mob_event(MOB_EVENT_ON_ENTER));
        {
            vector<mob_action*> actions;
            actions.push_back(new mob_action(pikmin::stop_in_group));
            events.back()->actions = actions;
        }
        
        //Grabbed by friend event.
        events.push_back(new mob_event(MOB_EVENT_GRABBED_BY_FRIEND));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::be_grabbed_by_friend));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("grabbed_by_leader");
            
            events.back()->actions = actions;
            
        }
        
        //Leader is far away event.
        events.push_back(new mob_event(MOB_EVENT_LEADER_IS_FAR));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_chasing");
            
            events.back()->actions = actions;
            
        }
        
        //Dismissed event.
        events.push_back(new mob_event(MOB_EVENT_DISMISSED));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::be_dismissed));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("idle");
            
            events.back()->actions = actions;
        }
        
        states.back()->events = events;
        
    }
    
    //Grabbed by leader state.
    states.push_back(new mob_state("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER));
    {
        vector<mob_event*> events;
        
        //Thrown event.
        events.push_back(new mob_event(MOB_EVENT_THROWN));
        {
        
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::be_thrown));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("thrown");
            
            events.back()->actions = actions;
        }
        
        states.back()->events = events;
    }
    
    //Thrown state.
    states.push_back(new mob_state("thrown", PIKMIN_STATE_THROWN));
    {
        vector<mob_event*> events;
        
        //Landed event.
        events.push_back(new mob_event(MOB_EVENT_LANDED));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::land));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("idle");
            
            events.back()->actions = actions;
            
        }
        
        states.back()->events = events;
        
    }
    
    //Idle state.
    states.push_back(new mob_state("idle", PIKMIN_STATE_IDLE));
    {
        vector<mob_event*> events;
        
        //Near task event.
        events.push_back(new mob_event(MOB_EVENT_NEAR_TASK));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::go_to_task));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("going_to_task");
            
            events.back()->actions = actions;
            
        }
        
        //Whistled event.
        events.push_back(new mob_event(MOB_EVENT_WHISTLED));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::called));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_chasing");
            
            events.back()->actions = actions;
        }
        
        //Touched by leader.
        events.push_back(new mob_event(MOB_EVENT_TOUCHED_BY_LEADER));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::called));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_chasing");
            
            events.back()->actions = actions;
        }
        
        states.back()->events = events;
        
    }
    
    //Going to task state.
    states.push_back(new mob_state("going_to_task", PIKMIN_STATE_GOING_TO_TASK));
    {
        vector<mob_event*> events;
        
        //Reached task event.
        events.push_back(new mob_event(MOB_EVENT_REACH_DESTINATION));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::work_on_task));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("busy");
            
            events.back()->actions = actions;
            
        }
        
        //Whistled.
        events.push_back(new mob_event(MOB_EVENT_WHISTLED));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::called));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_chasing");
            
            events.back()->actions = actions;
        }
        
        states.back()->events = events;
        
    }
    
    //Busy state.
    states.push_back(new mob_state("busy", PIKMIN_STATE_BUSY));
    {
        vector<mob_event*> events;
        
        //Whistled.
        events.push_back(new mob_event(MOB_EVENT_WHISTLED));
        {
            vector<mob_action*> actions;
            
            actions.push_back(new mob_action(pikmin::called));
            actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
            actions.back()->vs.push_back("in_group_chasing");
            
            events.back()->actions = actions;
        }
        
        states.back()->events = events;
    }
    
    fix_states(states);
    
    first_state_nr = 0;
    for(size_t s = 0; s < states.size(); s++) {
        if(states[s]->name == "idle") {
            first_state_nr = s;
            break;
        }
    }
}
