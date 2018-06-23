/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script classes and
 * related functions.
 */

//#define DEBUG_FSM

#include <algorithm>
#include <iostream>

#include "functions.h"
#include "mobs/mob.h"
#include "mobs/mob_fsm.h"
#include "mob_script.h"
#include "mobs/mob_type.h"
#include "particle.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a new mob action, given a data node.
 * dn:     the data node.
 * states: if this action messes with states, this points to the external
 *   vector containing the states.
 * mt:     mob type this action's fsm belongs to.
 */
mob_action::mob_action(
    data_node* dn, vector<mob_state*>* states, mob_type* mt
) :
    type(MOB_ACTION_UNKNOWN),
    code(nullptr),
    valid(true) {
    
    vector<string> words = split(dn->name);
    vector<string> v_words;
    string n = words[0];
    string v = "";
    if(words.size() > 1) {
        v_words = words;
        v_words.erase(v_words.begin());
        v = dn->name.substr(n.size(), string::npos);
        v = trim_spaces(v);
    }
    
    if(n == "delete") {
    
        type = MOB_ACTION_DELETE;
        
    } else if(n == "else") {
    
        type = MOB_ACTION_ELSE;
        if(!v.empty()) {
            log_error("The \"else\" action shouldn't have anything after it!");
        }
        
        
    } else if(n == "end_if") {
    
        type = MOB_ACTION_END_IF;
        if(!v.empty()) {
            log_error(
                "The \"end_if\" action shouldn't have anything after it!"
            );
        }
        
        
    } else if(n == "finish_dying") {
    
        type = MOB_ACTION_FINISH_DYING;
        
        
    } else if(n == "focus") {
    
        type = MOB_ACTION_FOCUS;
        
        
    } else if(n == "if") {
    
        //TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF;
        
        if(v_words.size() >= 4 && v_words[0] == "var") {
            vi.push_back(MOB_ACTION_IF_LHS_VAR);
            v_words.erase(v_words.begin());
        } else {
            vi.push_back(MOB_ACTION_IF_LHS_INFO);
        }
        
        if(v_words.size() >= 3) {
            if(v_words[1] == "=") {
                vi.push_back(MOB_ACTION_IF_OP_EQUAL);
            } else if(v_words[1] == "!=") {
                vi.push_back(MOB_ACTION_IF_OP_NOT);
            } else if(v_words[1] == "<") {
                vi.push_back(MOB_ACTION_IF_OP_LESS);
            } else if(v_words[1] == ">") {
                vi.push_back(MOB_ACTION_IF_OP_MORE);
            } else if(v_words[1] == "<=") {
                vi.push_back(MOB_ACTION_IF_OP_LESS_E);
            } else if(v_words[1] == ">=") {
                vi.push_back(MOB_ACTION_IF_OP_MORE_E);
            } else {
                log_error(
                    "Unknown operator \"" + v_words[2] + "\"!",
                    dn
                );
                valid = false;
            }
            
            string rhs =
                v.substr(v.find(v_words[1]) + v_words[1].size(), string::npos);
            rhs = trim_spaces(rhs);
            
            if(vi[0] == MOB_ACTION_IF_LHS_VAR) {
                vs.push_back(v_words[0]);
                
            } else {
            
                if(v_words[0] == "body_part") {
                    vi.push_back(MOB_ACTION_COMPARAND_BODY_PART);
                } else if(v_words[0] == "chomped_pikmin") {
                    vi.push_back(MOB_ACTION_COMPARAND_CHOMPED_PIKMIN);
                } else if(v_words[0] == "day_minutes") {
                    vi.push_back(MOB_ACTION_COMPARAND_DAY_MINUTES);
                } else if(v_words[0] == "frame_signal") {
                    vi.push_back(MOB_ACTION_COMPARAND_FRAME_SIGNAL);
                } else if(v_words[0] == "health") {
                    vi.push_back(MOB_ACTION_COMPARAND_HEALTH);
                } else if(v_words[0] == "latched_pikmin") {
                    vi.push_back(MOB_ACTION_COMPARAND_LATCHED_PIKMIN);
                } else if(v_words[0] == "latched_pikmin_weight") {
                    vi.push_back(MOB_ACTION_COMPARAND_LATCHED_PIKMIN_WEIGHT);
                } else if(v_words[0] == "mob_category") {
                    vi.push_back(MOB_ACTION_COMPARAND_MOB_CATEGORY);
                } else if(v_words[0] == "mob_type") {
                    vi.push_back(MOB_ACTION_COMPARAND_MOB_TYPE);
                } else if(v_words[0] == "other_body_part") {
                    vi.push_back(MOB_ACTION_COMPARAND_OTHER_BODY_PART);
                } else {
                    log_error(
                        "Unknown comparand \"" + v_words[0] + "\"!",
                        dn
                    );
                    valid = false;
                }
                
            }
            
            vs.push_back(rhs);
            
        } else {
            log_error(
                "This \"if\" is badly formed! Format: \"if <comparand> "
                "<operator> <value>\".",
                dn
            );
            valid = false;
        }
        
        
    } else if(n == "increment_var") {
    
        type = MOB_ACTION_INC_VAR;
        
        if(v_words.empty()) {
            log_error(
                "The increment_var action needs to know "
                "what variable to increment!", dn
            );
            valid = false;
        } else {
            vs = v_words;
        }
        
        
    } else if(n == "move") {
    
        type = MOB_ACTION_MOVE;
        
        if(v.empty()) {
            valid = false;
            log_error("The move action has no location specified!", dn);
        } else if(v == "focused_mob") {
            vi.push_back(MOB_ACTION_MOVE_FOCUSED_MOB);
        } else if(v == "focused_mob_position") {
            vi.push_back(MOB_ACTION_MOVE_FOCUSED_MOB_POS);
        } else if(v == "home") {
            vi.push_back(MOB_ACTION_MOVE_HOME);
        } else if(v == "randomly") {
            vi.push_back(MOB_ACTION_MOVE_RANDOMLY);
        } else if(v_words[0] == "relative") {
            vi.push_back(MOB_ACTION_MOVE_REL_COORDS);
            v_words.erase(v_words.begin());
            if(v_words.size() >= 2) {
                for(size_t c = 0; c < v_words.size(); ++c) {
                    vf.push_back(s2f(v_words[c]));
                }
            } else {
                valid = false;
            }
        } else if(v_words[0] == "absolute") {
            vi.push_back(MOB_ACTION_MOVE_COORDS);
            if(v_words.size() >= 2) {
                for(size_t c = 0; c < v_words.size(); ++c) {
                    vf.push_back(s2f(v_words[c]));
                }
            } else {
                valid = false;
            }
        } else {
            valid = false;
        }
        if(!valid) {
            log_error("Invalid move location \"" + v + "\"!", dn);
        }
        
        
    } else if(n == "play_sound") {
    
        type = MOB_ACTION_PLAY_SOUND;
        
        
    } else if(n == "randomize_var") {
    
        type = MOB_ACTION_RANDOMIZE_VAR;
        if(v_words.size() < 3) {
            log_error(
                "This \"randomize_var\" is badly formed! Format: "
                "\"randomize_var <variable> <minimum value> <maximum value>\".",
                dn
            );
            valid = false;
        } else {
            vs.push_back(v_words[0]);
            vi.push_back(s2i(v_words[1]));
            vi.push_back(s2i(v_words[2]));
        }
        
        
    } else if(n == "release") {
    
        type = MOB_ACTION_RELEASE;
        
        
    } else if(n == "set_animation") {
    
        type = MOB_ACTION_SET_ANIMATION;
        
        size_t f_pos = mt->anims.find_animation(v);
        if(f_pos == INVALID) {
            log_error("Unknown animation \"" + v + "\"!", dn);
            valid = false;
        } else {
            vi.push_back(f_pos);
        }
        
        
    } else if(n == "set_near_reach" || n == "set_far_reach") {
    
        if(n == "set_far_reach") {
            type = MOB_ACTION_SET_FAR_REACH;
        } else {
            type = MOB_ACTION_SET_NEAR_REACH;
        }
        
        if(v.empty()) {
            vi.push_back(INVALID);
            return;
        }
        
        for(size_t r = 0; r < mt->reaches.size(); ++r) {
            if(mt->reaches[r].name == v) {
                vi.push_back(r);
                return;
            }
        }
        
        log_error(
            "Reach-setting action refers to non-existent reach \"" +
            v + "\"!", dn
        );
        valid = false;
        
        
    } else if(n == "set_gravity") {
    
        type = MOB_ACTION_SET_GRAVITY;
        
        if(v.empty()) {
            valid = false;
            log_error(
                "The set_gravity action needs to know the "
                "new gravity multiplier!", dn
            );
        } else {
            vf.push_back(s2f(v));
        }
        
        
    } else if(n == "set_health") {
    
        type = MOB_ACTION_SET_HEALTH;
        
        if(v_words.size() < 2) {
            valid = false;
        } else {
            if(v_words[0] == "relative") {
                vi.push_back(MOB_ACTION_NUMERICAL_RELATIVE);
            } else if(v_words[0] == "absolute") {
                vi.push_back(MOB_ACTION_NUMERICAL_ABSOLUTE);
            } else {
                valid = false;
            }
            vf.push_back(s2f(v_words[1]));
        }
        
        if(!valid) {
            log_error("Invalid health data \"" + v + "\"!", dn);
        }
        
        
    } else if(n == "set_hiding") {
    
        type = MOB_ACTION_SET_HIDING;
        
        if(v.empty()) {
            log_error("The hide action needs a true or false value!", dn);
            valid = false;
        } else {
            vi.push_back(s2b(v));
        }
        
        
    } else if(n == "set_state" && states) {
    
        type = MOB_ACTION_SET_STATE;
        
        for(size_t s = 0; s < states->size(); ++s) {
            if(states->at(s)->name == v) {
                vi.push_back(s);
                break;
            }
        }
        if(vi.empty()) {
            log_error("Unknown state \"" + v + "\"!", dn);
        }
        
        
    } else if(n == "set_tangible") {
    
        type = MOB_ACTION_SET_TANGIBLE;
        
        if(v.empty()) {
            valid = false;
            log_error(
                "The set_tangible action needs a true or false value!", dn
            );
        } else {
            vi.push_back(s2b(v));
        }
        
        
    } else if(n == "set_timer") {
    
        type = MOB_ACTION_SET_TIMER;
        
        if(v_words.size() >= 1 && v_words[0] == "randomly") {
            vi.push_back(MOB_ACTION_SET_TIMER_RANDOM);
            if(v_words.size() >= 3) {
                vf.push_back(s2f(v_words[1]));
                vf.push_back(s2f(v_words[2]));
            } else {
                log_error(
                    "To set a timer randomly, you need to specify the "
                    "minimum and maximum time!", dn
                );
                valid = false;
            }
        } else if(v_words.size() >= 1) {
            vi.push_back(MOB_ACTION_SET_TIMER_NUMBER);
            vf.push_back(s2f(v_words[0]));
        } else {
            log_error("No timer amount specified!", dn);
            valid = false;
        }
        
        
    } else if(n == "set_var") {
    
        type = MOB_ACTION_SET_VAR;
        
        if(v_words.size() < 2) {
            log_error("\"set_var\" needs to know the variable and value!", dn);
            valid = false;
        } else {
            vs = v_words;
        }
        
        
    } else if(n == "spawn") {
    
        type = MOB_ACTION_SPAWN;
        
        if(v.empty()) {
            valid = false;
            log_error(
                "This \"spawn\" is badly formed! Format: \"spawn <object name> "
                "<relative/absolute> <x> <y> <z> <angle> "
                "[<script variables>]\".", dn
            );
            return;
        }
        
        size_t params_word = v_words.size();
        for(size_t w = 0; w < v_words.size(); ++w) {
            if(v_words[w].find("=") != string::npos) {
                //Found a word that has an equals in it,
                //so this is the spawned object's extra script data.
                if(v_words[w] == "=") {
                    //If it's the full word, that means the user split the
                    //parameter and the equals.
                    params_word = w - 1;
                } else {
                    params_word = w;
                }
                break;
            }
        }
        
        string object_name;
        bool relative = true;
        float x, y, z, a;
        for(size_t w = 0; w < params_word - 5; ++w) {
            object_name += v_words[w] + " ";
        }
        if(object_name.empty()) {
            valid = false;
        } else {
            object_name.erase(object_name.size() - 1);
            
            if(v_words[params_word - 5] == "relative") {
                relative = true;
            } else if(v_words[params_word - 5] == "absolute") {
                relative = false;
            } else {
                valid = false;
            }
            x = s2f(v_words[params_word - 4]);
            y = s2f(v_words[params_word - 3]);
            z = s2f(v_words[params_word - 2]);
            a = deg_to_rad(s2f(v_words[params_word - 1]));
            
            vs.push_back(object_name);
            vi.push_back(
                relative ?
                MOB_ACTION_NUMERICAL_RELATIVE : MOB_ACTION_NUMERICAL_ABSOLUTE
            );
            vf.push_back(x);
            vf.push_back(y);
            vf.push_back(z);
            vf.push_back(a);
            if(params_word < v_words.size()) {
                vs.push_back(
                    v.substr(v.find(v_words[params_word]), string::npos)
                );
            }
        }
        
        if(!valid) {
            log_error(
                "This \"spawn\" is badly formed! Format: \"spawn <object name> "
                "<relative/absolute> <x> <y> <z> <angle> "
                "[<script variables>]\".", dn
            );
        }
        
        
    } else if(n == "start_chomping") {
    
        type = MOB_ACTION_START_CHOMPING;
        
        if(v_words.empty()) {
            valid = false;
        } else {
            //The first word is the number of Pikmin it can chomp at most.
            vi.push_back(s2i(v_words[0]));
            
            for(size_t p = 1; p < v_words.size(); ++p) {
                size_t p_nr = mt->anims.find_body_part(v_words[p]);
                
                if(p_nr == INVALID) {
                    log_error("Unknown body part \"" + v_words[p] + "\"!", dn);
                    valid = false;
                } else {
                    vi.push_back(p_nr);
                }
            }
            
            if(vi.size() == 1) {
                valid = false;
            }
        }
        
        if(!valid) {
            log_error(
                "This \"start_chomping\" is badly formed! Format: "
                "\"start_chomping <maximum victims> <body parts>\".", dn
            );
        }
        
        
    } else if(n == "start_dying") {
    
        type = MOB_ACTION_START_DYING;
        
        
    } else if(n == "start_particles") {
    
        type = MOB_ACTION_START_PARTICLES;
        if(v_words.size() < 3) {
            log_error(
                "This \"start_particles\" is badly formed! Format: "
                "\"start_particles\" <generator name> "
                "<x offset> <y offset>\".", dn
            );
            valid = false;
        } else {
            if(
                custom_particle_generators.find(v_words[0]) ==
                custom_particle_generators.end()
            ) {
                log_error(
                    "Particle generator \"" + v_words[0] + "\" not found!", dn
                );
                valid = false;
            } else {
                vs.push_back(v_words[0]);
                vf.push_back(s2f(v_words[1]));
                vf.push_back(s2f(v_words[2]));
            }
        }
        
        
    } else if(n == "stop") {
    
        type = MOB_ACTION_STOP;
        
        if(v == "vertically") {
            vi.push_back(1);
        }
        
        
    } else if(n == "stop_chomping") {
    
        type = MOB_ACTION_STOP_CHOMPING;
        
        
    } else if(n == "stop_particles") {
    
        type = MOB_ACTION_STOP_PARTICLES;
        
        
    } else if(n == "swallow") {
    
        type = MOB_ACTION_SWALLOW;
        
        if(v == "all") {
            vi.push_back(MOB_ACTION_SWALLOW_ALL);
        } else {
            vi.push_back(MOB_ACTION_SWALLOW_NUMBER);
            vi.push_back(s2i(v));
        }
        
        
    } else if(n == "teleport") {
    
        type = MOB_ACTION_TELEPORT;
        
        if(v.empty()) {
            valid = false;
            log_error(
                "The \"teleport\" action needs to know the location!", dn
            );
        } else {
            if(v_words[0] == "relative") {
                vi.push_back(MOB_ACTION_NUMERICAL_RELATIVE);
                v_words.erase(v_words.begin());
            } else if(v_words[0] == "absolute") {
                vi.push_back(MOB_ACTION_NUMERICAL_ABSOLUTE);
                v_words.erase(v_words.begin());
            } else {
                valid = false;
            }
            if(v_words.size() < 3) {
                valid = false;
                log_error(
                    "The \"teleport\" action needs to know the "
                    "X, Y, and Z coordinates!", dn
                );
            } else {
                vf.push_back(s2f(v_words[0]));
                vf.push_back(s2f(v_words[1]));
                vf.push_back(s2f(v_words[2]));
            }
        }
        
        
    } else if(n == "turn") {
    
        type = MOB_ACTION_TURN;
        
        if(v_words.empty()) {
            valid = false;
            log_error("The \"turn\" action needs a direction!", dn);
        } else if(v_words[0] == "absolute") {
            vi.push_back(MOB_ACTION_TURN_ABSOLUTE);
            if(v_words.size() >= 2) {
                vf.push_back(deg_to_rad(s2f(v_words[1])));
            } else {
                valid = false;
                log_error(
                    "The \"turn\" action with an absolute angle needs "
                    "the angle!"
                );
            }
        } else if(v_words[0] == "relative") {
            vi.push_back(MOB_ACTION_TURN_RELATIVE);
            if(v_words.size() >= 2) {
                vf.push_back(deg_to_rad(s2f(v_words[1])));
            } else {
                valid = false;
                log_error(
                    "The \"turn\" action with a relative angle needs "
                    "the angle!"
                );
            }
        } else if(v == "focused_mob") {
            vi.push_back(MOB_ACTION_TURN_FOCUSED_MOB);
        } else if(v == "home") {
            vi.push_back(MOB_ACTION_TURN_HOME);
        } else if(v == "randomly") {
            vi.push_back(MOB_ACTION_TURN_RANDOMLY);
        } else {
            valid = false;
            log_error("Invalid turn target \"" + v + "\"!", dn);
        }
        
        
    } else {
    
        type = MOB_ACTION_UNKNOWN;
        log_error("Unknown script action name \"" + n + "\"!", dn);
        valid = false;
        
        
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty mob action.
 * type:     the action type.
 */
mob_action::mob_action(unsigned char type) :
    type(type),
    code(nullptr),
    valid(true) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action that runs custom code.
 * code: the function to run.
 */
mob_action::mob_action(custom_action_code code) :
    type(MOB_ACTION_UNKNOWN),
    code(code),
    valid(true) {
    
}


/* ----------------------------------------------------------------------------
 * Runs an action.
 * m:             the mob.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_2: custom argument #2 to pass to the code.
 * Return value is only used by the "if" actions, to indicate their
 *   evaluation result.
 */
bool mob_action::run(
    mob* m, void* custom_data_1, void* custom_data_2,
    const size_t parent_event
) {

    if(!valid) return false;
    
    //Custom code (i.e. instead of text-based script, use actual code).
    if(code) {
        code(m, custom_data_1, custom_data_2);
        return false;
    }
    
    if(type == MOB_ACTION_DELETE) {
    
        m->to_delete = true;
        
        
    } else if(type == MOB_ACTION_SWALLOW) {
    
        if(vi[0] == MOB_ACTION_SWALLOW_ALL) {
            m->swallow_chomped_pikmin(m->chomping_pikmin.size());
        } else {
            m->swallow_chomped_pikmin(vi[1]);
        }
        
        
    } else if(type == MOB_ACTION_FOCUS) {
    
        m->focused_mob = (mob*) custom_data_1;
        
        
    } else if(type == MOB_ACTION_FINISH_DYING) {
    
        m->finish_dying();
        
        
    } else if(type == MOB_ACTION_IF) {
    
        string lhs;
        string rhs;
        if(
            vi[0] == MOB_ACTION_IF_LHS_VAR &&
            vs.size() >= 2
        ) {
            lhs = m->vars[vs[0]];
            rhs = vs[1];
        } else if(
            vi[0] == MOB_ACTION_IF_LHS_INFO &&
            vi.size() >= 3 && !vs.empty()
        ) {
            if(vi[2] == MOB_ACTION_COMPARAND_CHOMPED_PIKMIN) {
                lhs = i2s(m->chomping_pikmin.size());
            } else if(vi[2] == MOB_ACTION_COMPARAND_DAY_MINUTES) {
                lhs = i2s(day_minutes);
            } else if(vi[2] == MOB_ACTION_COMPARAND_FRAME_SIGNAL) {
                if(parent_event == MOB_EVENT_FRAME_SIGNAL) {
                    lhs = i2s(*((size_t*) custom_data_1));
                }
            } else if(vi[2] == MOB_ACTION_COMPARAND_HEALTH) {
                lhs = i2s(m->health);
            } else if(vi[2] == MOB_ACTION_COMPARAND_LATCHED_PIKMIN) {
                lhs = i2s(m->get_latched_pikmin_amount());
            } else if(vi[2] == MOB_ACTION_COMPARAND_LATCHED_PIKMIN_WEIGHT) {
                lhs = i2s(m->get_latched_pikmin_weight());
            } else if(vi[2] == MOB_ACTION_COMPARAND_MOB_CATEGORY) {
                if(
                    parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                    parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
                    parent_event == MOB_EVENT_OBJECT_IN_REACH ||
                    parent_event == MOB_EVENT_OPPONENT_IN_REACH
                ) {
                    lhs = ((mob*) custom_data_1)->type->category->name;
                }
            } else if(vi[2] == MOB_ACTION_COMPARAND_MOB_TYPE) {
                if(
                    parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                    parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
                    parent_event == MOB_EVENT_OBJECT_IN_REACH ||
                    parent_event == MOB_EVENT_OPPONENT_IN_REACH
                ) {
                    lhs = ((mob*) custom_data_1)->type->name;
                }
            } else if(vi[2] == MOB_ACTION_COMPARAND_BODY_PART) {
                if(
                    parent_event == MOB_EVENT_HITBOX_TOUCH_N ||
                    parent_event == MOB_EVENT_HITBOX_TOUCH_N_A ||
                    parent_event == MOB_EVENT_DAMAGE
                ) {
                    lhs =
                        (
                            (hitbox_interaction*) custom_data_1
                        )->h1->body_part_name;
                } else if(
                    parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                    parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
                    parent_event == MOB_EVENT_PIKMIN_LANDED
                ) {
                    lhs =
                        m->get_closest_hitbox(
                            ((mob*) custom_data_1)->pos,
                            INVALID, NULL
                        )->body_part_name;
                }
            } else if(vi[2] == MOB_ACTION_COMPARAND_OTHER_BODY_PART) {
                if(
                    parent_event == MOB_EVENT_HITBOX_TOUCH_N ||
                    parent_event == MOB_EVENT_HITBOX_TOUCH_N_A ||
                    parent_event == MOB_EVENT_DAMAGE
                ) {
                    lhs =
                        (
                            (hitbox_interaction*) custom_data_1
                        )->h2->body_part_name;
                } else if(
                    parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                    parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
                    parent_event == MOB_EVENT_PIKMIN_LANDED
                ) {
                    lhs =
                        ((mob*) custom_data_1)->get_closest_hitbox(
                            m->pos, INVALID, NULL
                        )->body_part_name;
                }
            }
            rhs = vs[0];
        } else {
            return false;
        }
        
        if(vi[1] == MOB_ACTION_IF_OP_EQUAL) {
            return (lhs == rhs);
        } else if(vi[1] == MOB_ACTION_IF_OP_NOT) {
            return (lhs != rhs);
        } else if(vi[1] == MOB_ACTION_IF_OP_LESS) {
            return (s2i(lhs) < s2i(rhs));
        } else if(vi[1] == MOB_ACTION_IF_OP_MORE) {
            return (s2i(lhs) > s2i(rhs));
        } else if(vi[1] == MOB_ACTION_IF_OP_LESS_E) {
            return (s2i(lhs) <= s2i(rhs));
        } else if(vi[1] == MOB_ACTION_IF_OP_MORE_E) {
            return (s2i(lhs) >= s2i(rhs));
        }
        
    } else if(type == MOB_ACTION_INC_VAR) {
    
        int nr = s2i(m->vars[vs[0]]);
        m->set_var(vs[0], i2s(nr + 1));
        
        
    } else if(type == MOB_ACTION_MOVE) {
    
        if(vi[0] == MOB_ACTION_MOVE_FOCUSED_MOB) {
            if(m->focused_mob) {
                m->chase(point(), &m->focused_mob->pos, false);
            } else {
                m->stop_chasing();
            }
            
        } else if(vi[0] == MOB_ACTION_MOVE_FOCUSED_MOB_POS) {
            if(m->focused_mob) {
                m->chase(m->focused_mob->pos, NULL, false);
            } else {
                m->stop_chasing();
            }
            
        } else if(vi[0] == MOB_ACTION_MOVE_HOME) {
            m->chase(m->home, NULL, false);
            
        } else if(vi[0] == MOB_ACTION_MOVE_RANDOMLY) {
            m->chase(
                point(
                    m->pos.x + randomf(-1000, 1000),
                    m->pos.y + randomf(-1000, 1000)
                ),
                NULL, false
            );
            
        } else if(vi[0] == MOB_ACTION_MOVE_COORDS) {
            m->chase(point(vf[0], vf[1]), NULL, false);
            
        } else if(vi[0] == MOB_ACTION_MOVE_REL_COORDS) {
            point p = rotate_point(point(vf[0], vf[1]), m->angle);
            m->chase(m->pos + p, NULL, false);
            
        }
        
        
    } else if(type == MOB_ACTION_RANDOMIZE_VAR) {
    
        m->vars[vs[0]] = i2s(randomi(vi[0], vi[1]));
        
        
    } else if(type == MOB_ACTION_RELEASE) {
    
        m->release_chomped_pikmin();
        
        
    } else if(type == MOB_ACTION_SET_ANIMATION) {
    
        m->set_animation(vi[0], false);
        
        
    } else if(type == MOB_ACTION_START_CHOMPING) {
    
        m->chomp_max = vi[0];
        m->chomp_body_parts.clear();
        for(size_t p = 1; p < vi.size(); ++p) {
            m->chomp_body_parts.push_back(vi[p]);
        }
        
        
    } else if(type == MOB_ACTION_SET_FAR_REACH) {
    
        m->far_reach = vi[0];
        
    } else if(type == MOB_ACTION_SET_GRAVITY) {
    
        m->gravity_mult = vf[0];
        
        
    } else if(type == MOB_ACTION_SET_HEALTH) {
    
        m->set_health(
            vi[0] == MOB_ACTION_NUMERICAL_RELATIVE,
            false,
            vf[0]
        );
        
        
    } else if(type == MOB_ACTION_SET_HIDING) {
    
        m->hide = vi[0];
        
        
    } else if(type == MOB_ACTION_SET_NEAR_REACH) {
    
        m->near_reach = vi[0];
        
    } else if(type == MOB_ACTION_SET_STATE) {
    
        m->fsm.set_state(vi[0], custom_data_1, custom_data_2);
        
        
    } else if(type == MOB_ACTION_SET_TANGIBLE) {
    
        m->tangible = (bool) vi[0];
        
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        if(vi[0] == MOB_ACTION_SET_TIMER_RANDOM) {
            m->set_timer(randomf(vf[0], vf[1]));
        } else {
            m->set_timer(vf[0]);
        }
        
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        m->set_var(vs[0], vs[1]);
        
        
    } else if(type == MOB_ACTION_SPAWN) {
    
        //First, find the mob.
        mob_type* type_ptr = mob_categories.find_mob_type(vs[0]);
        if(!type_ptr) return false;
        if(
            type_ptr->category->id == MOB_CATEGORY_PIKMIN &&
            pikmin_list.size() >= max_pikmin_in_field
        ) {
            return false;
        }
        
        point xy;
        float z = 0;
        float angle = 0;
        
        if(vi[0] == MOB_ACTION_NUMERICAL_RELATIVE) {
            xy = m->pos + rotate_point(point(vf[0], vf[1]), m->angle);
            z = m->z + vf[2];
            angle = m->angle + vf[3];
        } else {
            xy = point(vf[0], vf[1]);
            z = vf[2];
            angle = vf[3];
        }
        
        if(!get_sector(xy, NULL, true)) {
            //Spawn out of bounds? No way!
            return false;
        }
        
        mob* new_mob =
            create_mob(
                type_ptr->category,
                xy,
                type_ptr,
                angle,
                vs.size() >= 2 ? vs[1] : ""
            );
            
        new_mob->z = z;
        
        if(type_ptr->category->id == MOB_CATEGORY_TREASURES) {
            //This way, treasures that fall into the abyss respawn at the
            //spawner mob's original spot.
            new_mob->home = m->home;
        } else {
            new_mob->home = xy;
        }
        
        
    } else if(type == MOB_ACTION_START_DYING) {
    
        m->start_dying();
        
        
    } else if(type == MOB_ACTION_START_PARTICLES) {
    
        if(vs.empty()) {
            m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
        } else {
            if(
                custom_particle_generators.find(vs[0]) !=
                custom_particle_generators.end()
            ) {
                particle_generator pg = custom_particle_generators[vs[0]];
                pg.id = MOB_PARTICLE_GENERATOR_SCRIPT;
                pg.follow_pos = &m->pos;
                pg.follow_angle = &m->angle;
                pg.follow_pos_offset = point(vf[0], vf[1]);
                pg.reset();
                m->particle_generators.push_back(pg);
            }
        }
        
        
    } else if(type == MOB_ACTION_STOP) {
    
        if(vi.empty()) {
            m->stop_chasing();
            m->intended_turn_angle = m->angle;
            m->intended_turn_pos = NULL;
        } else {
            m->speed_z = 0;
        }
        
        
    } else if(type == MOB_ACTION_STOP_CHOMPING) {
    
        m->chomp_max = 0;
        m->chomp_body_parts.clear();
        
        
    } else if(type == MOB_ACTION_STOP_PARTICLES) {
    
        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
        
        
    } else if(type == MOB_ACTION_TELEPORT) {
    
        m->stop_chasing();
        point xy;
        float z;
        if(vi[0] == MOB_ACTION_NUMERICAL_RELATIVE) {
            point p = rotate_point(point(vf[0], vf[1]), m->angle);
            xy = m->pos + p;
            z = m->z + vf[2];
        } else {
            xy = point(vf[0], vf[1]);
            z = vf[2];
        }
        m->chase(xy, NULL, true);
        m->z = z;
        
        
    } else if(type == MOB_ACTION_TURN) {
    
        if(vi[0] == MOB_ACTION_TURN_ABSOLUTE) {
            m->face(vf[0], NULL);
        } else if(vi[0] == MOB_ACTION_TURN_RELATIVE) {
            m->face(m->angle + vf[0], NULL);
        } else if(vi[0] == MOB_ACTION_TURN_FOCUSED_MOB && m->focused_mob) {
            m->face(0, &m->focused_mob->pos);
        } else if(vi[0] == MOB_ACTION_TURN_HOME) {
            m->face(get_angle(m->pos, m->home), NULL);
        } else if(vi[0] == MOB_ACTION_TURN_RANDOMLY) {
            m->face(randomf(0, M_PI * 2), NULL);
        }
        
        
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Runs a mob event. Basically runs all actions within.
 * m:             the mob.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_2: custom argument #2 to pass to the code.
 */
void mob_event::run(mob* m, void* custom_data_1, void* custom_data_2) {
    for(size_t a = 0; a < actions.size(); ++a) {
    
        if(actions[a]->type == MOB_ACTION_IF) {
            //If statement. Look out for its return value, and
            //change the flow accordingly.
            
            if(!actions[a]->run(m, custom_data_1, custom_data_2, type)) {
                //If it returned true, execution continues as normal, but
                //if it returned false, skip to the "else" or "end if" actions.
                size_t next_a = a + 1;
                size_t depth = 0;
                for(; next_a < actions.size(); ++next_a) {
                    if(actions[next_a]->type == MOB_ACTION_IF) {
                        depth++;
                    } else if(actions[next_a]->type == MOB_ACTION_ELSE) {
                        if(depth == 0) break;
                    } else if(actions[next_a]->type == MOB_ACTION_END_IF) {
                        if(depth == 0) break;
                        else depth--;
                    }
                }
                a = next_a;
                
            }
            
        } else if(actions[a]->type == MOB_ACTION_ELSE) {
            //If we actually managed to read an "else", that means we were
            //running through the normal execution of a "then" section.
            //Jump to the "end if".
            size_t next_a = a + 1;
            size_t depth = 0;
            for(; next_a < actions.size(); ++next_a) {
                if(actions[next_a]->type == MOB_ACTION_IF) {
                    depth++;
                } else if(actions[next_a]->type == MOB_ACTION_END_IF) {
                    if(depth == 0) break;
                    else depth--;
                }
            }
            a = next_a;
            
        } else if(actions[a]->type == MOB_ACTION_END_IF) {
            //Nothing to do.
            
        } else {
            //Normal action.
            actions[a]->run(m, custom_data_1, custom_data_2, type);
            if(actions[a]->type == MOB_ACTION_SET_STATE) break;
            
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns a pointer to an event of the given type in the state,
 * if it exists.
 * type: the event's type.
 */
mob_event* mob_state::get_event(const size_t type) {
    return events[type];
}


/* ----------------------------------------------------------------------------
 * Returns a pointer to an event of the given type in the current state,
 * if it exists.
 * type: the event's type.
 */
mob_event* mob_fsm::get_event(const size_t type) {
    if(!cur_state) return NULL;
    return cur_state->events[type];
}


/* ----------------------------------------------------------------------------
 * Runs an event in the current state, if it exists.
 * type:          the event's type.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_1: custom argument #1 to pass to the code.
 */
void mob_fsm::run_event(
    const size_t type, void* custom_data_1, void* custom_data_2
) {
    mob_event* e = get_event(type);
    if(e) {
        e->run(m, custom_data_1, custom_data_2);
    } else {
    
#ifdef DEBUG_FSM
        cout <<
             "Missing event on run_event() - Mob " <<
             m << ", event " << type << ", state " <<
             (this->cur_state ? this->cur_state->name : "[None]") <<
             endl;
#endif
             
        return;
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new event given a data node.
 * d: the data node.
 * a: its actions.
 */
mob_event::mob_event(data_node* d, const vector<mob_action*> &a) :
    actions(a) {
    
#define r(name, number) \
    else if(n == (name)) type = (number)
    
    string n = d->name;
    if(n == "on_enter")  type = MOB_EVENT_ON_ENTER;
    r("on_leave",               MOB_EVENT_ON_LEAVE);
    r("on_tick",                MOB_EVENT_ON_TICK);
    r("on_animation_end",       MOB_EVENT_ANIMATION_END);
    r("on_damage",              MOB_EVENT_DAMAGE);
    r("on_far_from_home",       MOB_EVENT_FAR_FROM_HOME);
    r("on_focus_off_reach",     MOB_EVENT_FOCUS_OFF_REACH);
    r("on_frame_signal",        MOB_EVENT_FRAME_SIGNAL);
    r("on_itch",                MOB_EVENT_ITCH);
    r("on_land",                MOB_EVENT_LANDED);
    r("on_object_in_reach",     MOB_EVENT_OBJECT_IN_REACH);
    r("on_opponent_in_reach",   MOB_EVENT_OPPONENT_IN_REACH);
    r("on_pikmin_land",         MOB_EVENT_PIKMIN_LANDED);
    r("on_reach_destination",   MOB_EVENT_REACHED_DESTINATION);
    r("on_touch_hazard",        MOB_EVENT_TOUCHED_HAZARD);
    r("on_touch_object",        MOB_EVENT_TOUCHED_OBJECT);
    r("on_touch_opponent",      MOB_EVENT_TOUCHED_OPPONENT);
    r("on_touch_wall",          MOB_EVENT_TOUCHED_WALL);
    r("on_timer",               MOB_EVENT_TIMER);
    else {
        type = MOB_EVENT_UNKNOWN;
        log_error("Unknown script event name \"" + n + "\"!", d);
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new event.
 * t: the event type.
 * a: its actions.
 */
mob_event::mob_event(const unsigned char t, const vector<mob_action*> &a) :
    type(t),
    actions(a) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new state.
 * name: the state's name.
 */
mob_state::mob_state(const string &name) :
    name(name),
    id(INVALID) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
        events[e] = nullptr;
    }
}

/* ----------------------------------------------------------------------------
 * Creates a new state.
 * name: the state's name.
 * e:    its events.
 */
mob_state::mob_state(const string &name, mob_event* evs[N_MOB_EVENTS]) :
    name(name),
    id(INVALID) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
        events[e] = evs[e];
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty state.
 * name: the state's name.
 * id:   its ID, for sorting on the vector of states.
 */
mob_state::mob_state(const string &name, const size_t id) :
    name(name),
    id(id) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
        events[e] = nullptr;
    }
}


/* ----------------------------------------------------------------------------
 * Changes the fsm to use a different state.
 * info*: data to pass on to the code after the state change.
 *   This data comes from the event that started all of this.
 */
void mob_fsm::set_state(const size_t new_state, void* info1, void* info2) {

    //Run the code to leave the current state.
    if(cur_state) {
        for(unsigned char p = STATE_HISTORY_SIZE - 1; p > 0; --p) {
            prev_state_names[p] = prev_state_names[p - 1];
        }
        prev_state_names[0] = cur_state->name;
        run_event(MOB_EVENT_ON_LEAVE, info1, info2);
    }
    
    //Uncomment this to be notified about state changes on stdout.
    /*if(cur_state) {
        cout << "State " << cur_state->name << " -> "
        << m->type->states[new_state]->name << "\n";
    }*/
    
    if(new_state < m->type->states.size() && new_state != INVALID) {
        //Switch states.
        cur_state = m->type->states[new_state];
        
        //Run the code to enter the new state.
        run_event(MOB_EVENT_ON_ENTER, info1, info2);
    }
    
}


/* ----------------------------------------------------------------------------
 * Creates a new mob FSM.
 */
mob_fsm::mob_fsm(mob* m) :
    cur_state(nullptr) {
    
    if(!m) return;
    this->m = m;
}


/* ----------------------------------------------------------------------------
 * Confirms if the "if", "else", and "end_if" actions in a given vector of
 * actions are all okay, and there are no mismatches, like for instance,
 * an "else" without an "if".
 * If everything is okay, returns true. If not, throws errors to the
 * error log and returns false.
 * actions: The vector of actions to check.
 * dn:      Data node from where these actions came.
 */
bool assert_if_actions(const vector<mob_action*> &actions, data_node* dn) {
    int level = 0;
    for(size_t a = 0; a < actions.size(); ++a) {
        if(actions[a]->type == MOB_ACTION_IF) {
            level++;
        } else if(actions[a]->type == MOB_ACTION_ELSE) {
            if(level == 0) {
                log_error(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
        } else if(actions[a]->type == MOB_ACTION_END_IF) {
            if(level == 0) {
                log_error(
                    "Found an \"end_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            level--;
        }
    }
    if(level > 0) {
        log_error(
            "Some \"if\" actions don't have a matching \"end_if\" action!",
            dn
        );
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Fixes some things in the list of states.
 * For instance, state-switching actions that use
 * a name instead of a number.
 * states:         the vector of states.
 * starting_state: name of the starting state for the mob.
 * Returns the number of the starting state.
 */
size_t fix_states(vector<mob_state*> &states, const string &starting_state) {
    size_t starting_state_nr = INVALID;
    //Fix actions that change the state that are using a string.
    for(size_t s = 0; s < states.size(); ++s) {
        mob_state* state = states[s];
        if(state->name == starting_state) starting_state_nr = s;
        
        for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
            mob_event* ev = state->events[e];
            if(!ev) continue;
            
            for(size_t a = 0; a < ev->actions.size(); ++a) {
                mob_action* action = ev->actions[a];
                
                if(
                    action->type == MOB_ACTION_SET_STATE &&
                    !action->vs.empty()
                ) {
                    string state_name = action->vs[0];
                    size_t state_nr = 0;
                    bool found_state = false;
                    
                    for(; state_nr < states.size(); ++state_nr) {
                        if(states[state_nr]->name == state_name) {
                            found_state = true;
                            break;
                        }
                    }
                    
                    if(!found_state) {
                        state_nr = INVALID;
                        log_error(
                            "State \"" + state->name +
                            "\" has an action to switch "
                            "to an unknown state: \"" + state_name + "\"!",
                            nullptr
                        );
                    }
                    
                    action->vs.clear();
                    action->vi.clear();
                    action->vi.push_back(state_nr);
                    
                }
            }
        }
    }
    return starting_state_nr;
}


/* ----------------------------------------------------------------------------
 * Loads the actions to be run when the mob initializes.
 * mt:      The type of mob the actions are going to.
 * node:    The data node.
 * actions: Vector of actions to be filled.
 */
void load_init_actions(
    mob_type* mt, data_node* node, vector<mob_action*>* actions
) {
    for(size_t a = 0; a < node->get_nr_of_children(); ++a) {
        actions->push_back(new mob_action(node->get_child(a), NULL, mt));
    }
    assert_if_actions(*actions, node);
}


/* ----------------------------------------------------------------------------
 * Loads the states off of a data node.
 * mt:     The type of mob the states are going to.
 * node:   The data node.
 * states: Vector of states to place the new states on.
 */
void load_script(mob_type* mt, data_node* node, vector<mob_state*>* states) {
    size_t n_new_states = node->get_nr_of_children();
    size_t old_n_states = states->size();
    
    for(size_t s = 0; s < n_new_states; ++s) {
    
        data_node* state_node = node->get_child(s);
        //Let's save the state now, so that the state switching events
        //can now what numbers the events they need correspond to.
        states->push_back(new mob_state(state_node->name));
    }
    
    for(size_t s = 0; s < n_new_states; ++s) {
        data_node* state_node = node->get_child(s);
        vector<mob_event*> events;
        size_t n_events = state_node->get_nr_of_children();
        
        for(size_t e = 0; e < n_events; ++e) {
        
            data_node* event_node = state_node->get_child(e);
            vector<mob_action*> actions;
            
            for(size_t a = 0; a < event_node->get_nr_of_children(); ++a) {
                data_node* action_node = event_node->get_child(a);
                actions.push_back(new mob_action(action_node, states, mt));
            }
            
            events.push_back(new mob_event(event_node, actions));
            
            assert_if_actions(actions, event_node);
            
        }
        
        //Inject a damage event.
        vector<mob_action*> da_actions;
        da_actions.push_back(new mob_action(gen_mob_fsm::be_attacked));
        events.push_back(new mob_event(MOB_EVENT_HITBOX_TOUCH_N_A, da_actions));
        
        //Inject a death event.
        if(
            state_node->name != mt->death_state_name &&
            find(
                mt->states_ignoring_death.begin(),
                mt->states_ignoring_death.end(),
                state_node->name
            ) == mt->states_ignoring_death.end() &&
            !mt->death_state_name.empty()
        ) {
            vector<mob_action*> de_actions;
            de_actions.push_back(new mob_action(gen_mob_fsm::die));
            events.push_back(new mob_event(MOB_EVENT_DEATH, de_actions));
        }
        
        //Inject a bottomless pit event.
        vector<mob_action*> bp_actions;
        bp_actions.push_back(new mob_action(gen_mob_fsm::fall_down_pit));
        events.push_back(new mob_event(MOB_EVENT_BOTTOMLESS_PIT, bp_actions));
        
        //Inject a spray event.
        if(
            find(
                mt->states_ignoring_spray.begin(),
                mt->states_ignoring_spray.end(),
                state_node->name
            ) == mt->states_ignoring_spray.end()
        ) {
            vector<mob_action*> s_actions;
            s_actions.push_back(new mob_action(gen_mob_fsm::touch_spray));
            events.push_back(new mob_event(MOB_EVENT_TOUCHED_SPRAY, s_actions));
        }
        
        //Connect the events to the state.
        for(size_t e = 0; e < events.size(); ++e) {
            size_t ev_type = events[e]->type;
            states->at(s + old_n_states)->events[ev_type] = events[e];
        }
        
        states->at(s + old_n_states)->id = s + old_n_states;
        
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the states from memory.
 * mt: the type of mob.
 */
void unload_script(mob_type* mt) {
    for(size_t s = 0; s < mt->states.size(); ++s) {
        mob_state* s_ptr = mt->states[s];
        
        for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
            mob_event* e_ptr = s_ptr->events[e];
            if(!e_ptr) continue;
            
            for(size_t a = 0; a < e_ptr->actions.size(); ++a) {
                delete e_ptr->actions[a];
            }
            
            e_ptr->actions.clear();
            delete e_ptr;
            
        }
        
        delete s_ptr;
        
    }
    mt->states.clear();
}


/* ----------------------------------------------------------------------------
 * Creates the easy fsm creator.
 */
easy_fsm_creator::easy_fsm_creator() :
    cur_state(nullptr),
    cur_event(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Finishes the state that is currently under construction, if any.
 */
void easy_fsm_creator::commit_state() {
    if(!cur_state) return;
    commit_event();
    cur_state = NULL;
}


/* ----------------------------------------------------------------------------
 * Finishes the event that is currently under construction, if any.
 */
void easy_fsm_creator::commit_event() {
    if(!cur_event) return;
    cur_event = NULL;
}


/* ----------------------------------------------------------------------------
 * Finishes the previous state, if any, creates a new state,
 * and starts tracking for the creation of its events.
 */
void easy_fsm_creator::new_state(const string &name, const size_t id) {
    commit_state();
    cur_state = new mob_state(name, id);
    states.push_back(cur_state);
}


/* ----------------------------------------------------------------------------
 * Finishes the previous event, if any, creates a new event for the
 * current state, and starts tracking for the creation of its actions.
 */
void easy_fsm_creator::new_event(const unsigned char type) {
    commit_event();
    cur_event = new mob_event(type);
    cur_state->events[type] = cur_event;
}


/* ----------------------------------------------------------------------------
 * Creates a new action for the current event, one that changes
 * the mob's state to something else.
 */
void easy_fsm_creator::change_state(const string &new_state) {
    cur_event->actions.push_back(new mob_action(MOB_ACTION_SET_STATE));
    cur_event->actions.back()->vs.push_back(new_state);
}


/* ----------------------------------------------------------------------------
 * Creates a new action for the current event, one that
 * runs some custom code.
 */
void easy_fsm_creator::run(custom_action_code code) {
    cur_event->actions.push_back(new mob_action(code));
}


/* ----------------------------------------------------------------------------
 * Finishes any event or state under construction and returns the
 * final vector of states.
 */
vector<mob_state*> easy_fsm_creator::finish() {
    commit_event();
    commit_state();
    sort(
        states.begin(), states.end(),
    [] (mob_state * ms1, mob_state * ms2) -> bool {
        return ms1->id < ms2->id;
    }
    );
    return states;
}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about an event where two hitboxes touch.
 * mob2: the other mob.
 * h1:   the current mob's hitbox.
 * h2:   the other mob's hitbox.
 */
hitbox_interaction::hitbox_interaction(
    mob* mob2, hitbox* h1, hitbox* h2
) {
    this->mob2 = mob2;
    this->h1   = h1;
    this->h2   = h2;
}
