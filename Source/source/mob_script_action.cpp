/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script action classes and
 * related functions.
 */

#include "mob_script_action.h"

#include "utils/string_utils.h"
#include "functions.h"


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
    
    if(n == "arachnorb_plan_logic") {
    
        type = MOB_ACTION_ARACHNORB_PLAN_LOGIC;
        if(v == "home") {
            vi.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME);
        } else if(v == "forward") {
            vi.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD);
        } else if(v == "cw_turn") {
            vi.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN);
        } else if(v == "ccw_turn") {
            vi.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN);
        } else {
            valid = false;
            log_error(
                "The \"arachnorb_plan_logic\" action needs to know what "
                "the mob wants to do!", dn
            );
        }
        
    } else if(n == "delete") {
    
        type = MOB_ACTION_DELETE;
        
    } else if(n == "else") {
    
        type = MOB_ACTION_ELSE;
        if(!v.empty()) {
            log_error(
                "The \"else\" action shouldn't have anything after it!", dn
            );
        }
        
        
    } else if(n == "end_if") {
    
        type = MOB_ACTION_END_IF;
        if(!v.empty()) {
            log_error(
                "The \"end_if\" action shouldn't have anything after it!",
                dn
            );
        }
        
        
    } else if(n == "finish_dying") {
    
        type = MOB_ACTION_FINISH_DYING;
        
        
    } else if(n == "focus") {
    
        type = MOB_ACTION_FOCUS;
        
        if(v.empty()) {
            log_error("The \"focus\" action needs to know the target!", dn);
            valid = false;
            
        } else if(v == "parent") {
            vi.push_back(MOB_ACTION_FOCUS_PARENT);
            
        } else if(v == "trigger") {
            vi.push_back(MOB_ACTION_FOCUS_TRIGGER);
            
        } else {
            log_error("Unknown focus target \"" + v + "\"!", dn);
            valid = false;
            
        }
        
        
    } else if(n == "get_chomped") {
    
        type = MOB_ACTION_GET_CHOMPED;
        
        
    } else if(n == "if") {
    
        //TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF;
        
        /* vi[0] contains the operator.
         * vi[1] contains the type of LHS comparand.
         * vi[2] contains the type of RHS comparand.
         * vs[0] contains the LHS comparand.
         * vs[1] contains the RHS comparand.
         */
        
        //Find the operator first, since that's the easiest part.
        size_t operator_pos = string::npos;
        unsigned char operator_size = 0;
        if((operator_pos = v.find(" = ")) != string::npos) {
            vi.push_back(MOB_ACTION_IF_OP_EQUAL);
            operator_size = 3;
            
        } else if((operator_pos = v.find(" != ")) != string::npos) {
            vi.push_back(MOB_ACTION_IF_OP_NOT);
            operator_size = 4;
            
        } else if((operator_pos = v.find(" < ")) != string::npos) {
            vi.push_back(MOB_ACTION_IF_OP_LESS);
            operator_size = 3;
            
        } else if((operator_pos = v.find(" > ")) != string::npos) {
            vi.push_back(MOB_ACTION_IF_OP_MORE);
            operator_size = 3;
            
        } else if((operator_pos = v.find(" <= ")) != string::npos) {
            vi.push_back(MOB_ACTION_IF_OP_LESS_E);
            operator_size = 4;
            
        } else if((operator_pos = v.find(" >= ")) != string::npos) {
            vi.push_back(MOB_ACTION_IF_OP_MORE_E);
            operator_size = 4;
            
        } else {
            log_error(
                "This \"if\" is badly formed! Format: \"if <comparand> "
                "<operator> <value>\".",
                dn
            );
            valid = false;
            return;
            
        }
        
        //Now, we can gather the left-hand side, and the right-hand side.
        string lhs = v.substr(0, operator_pos);
        string rhs = v.substr(operator_pos + operator_size, string::npos);
        
        lhs = trim_spaces(lhs);
        rhs = trim_spaces(rhs);
        
        //Examine the left-hand side.
        if(lhs.size() >= 4 && lhs.substr(0, 4) == "var ") {
            vi.push_back(MOB_ACTION_IF_LHS_VAR);
            lhs = trim_spaces(lhs.substr(4, string::npos), true);
        } else if(lhs == "body_part") {
            vi.push_back(MOB_ACTION_IF_LHS_BODY_PART);
        } else if(lhs == "chomped_pikmin") {
            vi.push_back(MOB_ACTION_IF_LHS_CHOMPED_PIKMIN);
        } else if(lhs == "day_minutes") {
            vi.push_back(MOB_ACTION_IF_LHS_DAY_MINUTES);
        } else if(lhs == "field_pikmin") {
            vi.push_back(MOB_ACTION_IF_LHS_FIELD_PIKMIN);
        } else if(lhs == "frame_signal") {
            vi.push_back(MOB_ACTION_IF_LHS_FRAME_SIGNAL);
        } else if(lhs == "health") {
            vi.push_back(MOB_ACTION_IF_LHS_HEALTH);
        } else if(lhs == "latched_pikmin") {
            vi.push_back(MOB_ACTION_IF_LHS_LATCHED_PIKMIN);
        } else if(lhs == "latched_pikmin_weight") {
            vi.push_back(MOB_ACTION_IF_LHS_LATCHED_PIKMIN_WEIGHT);
        } else if(lhs == "message") {
            vi.push_back(MOB_ACTION_IF_LHS_MESSAGE);
        } else if(lhs == "mob_category") {
            vi.push_back(MOB_ACTION_IF_LHS_MOB_CATEGORY);
        } else if(lhs == "mob_type") {
            vi.push_back(MOB_ACTION_IF_LHS_MOB_TYPE);
        } else if(lhs == "other_body_part") {
            vi.push_back(MOB_ACTION_IF_LHS_OTHER_BODY_PART);
        } else if(lhs == "sender") {
            vi.push_back(MOB_ACTION_IF_LHS_MESSAGE_SENDER);
        } else {
            log_error(
                "Unknown comparand \"" + lhs + "\"!",
                dn
            );
            valid = false;
            return;
        }
        
        //Now, examine the right-hand side.
        if(rhs.size() >= 4 && rhs.substr(0, 4) == "var ") {
            vi.push_back(MOB_ACTION_IF_RHS_VAR);
            rhs = trim_spaces(rhs.substr(4, string::npos), true);
        } else {
            vi.push_back(MOB_ACTION_IF_RHS_CONST);
        }
        
        //Finally, save both.
        vs.push_back(lhs);
        vs.push_back(rhs);
        
        
    } else if(n == "move_to_absolute") {
    
        type = MOB_ACTION_MOVE_TO_ABSOLUTE;
        
        if(v_words.size() >= 2) {
            for(size_t c = 0; c < v_words.size(); ++c) {
                vf.push_back(s2f(v_words[c]));
            }
        } else {
            valid = false;
        }
        
        
    } else if(n == "move_to_relative") {
    
        type = MOB_ACTION_MOVE_TO_RELATIVE;
        
        if(v_words.size() >= 2) {
            for(size_t c = 0; c < v_words.size(); ++c) {
                vf.push_back(s2f(v_words[c]));
            }
        } else {
            valid = false;
        }
        
        
    } else if(n == "move_to_target") {
    
        type = MOB_ACTION_MOVE_TO_TARGET;
        
        if(v.empty()) {
            valid = false;
            log_error("The move action has no location specified!", dn);
        } else if(v == "arachnorb_foot_logic") {
            vi.push_back(MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC);
        } else if(v == "away_from_focused_mob") {
            vi.push_back(MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB);
        } else if(v == "focused_mob") {
            vi.push_back(MOB_ACTION_MOVE_FOCUSED_MOB);
        } else if(v == "focused_mob_position") {
            vi.push_back(MOB_ACTION_MOVE_FOCUSED_MOB_POS);
        } else if(v == "home") {
            vi.push_back(MOB_ACTION_MOVE_HOME);
        } else if(v == "linked_mob_average") {
            vi.push_back(MOB_ACTION_MOVE_LINKED_MOB_AVERAGE);
        } else if(v == "randomly") {
            vi.push_back(MOB_ACTION_MOVE_RANDOMLY);
        } else {
            valid = false;
        }
        if(!valid) {
            log_error("Invalid move target \"" + v + "\"!", dn);
        }
        
        
    } else if(n == "order_release") {
    
        type = MOB_ACTION_ORDER_RELEASE;
        
        
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
        
        
    } else if(n == "randomize_timer") {
    
        type = MOB_ACTION_RANDOMIZE_TIMER;
        
        if(v_words.size() >= 2) {
            vf.push_back(s2f(v_words[0]));
            vf.push_back(s2f(v_words[1]));
        } else {
            log_error(
                "To set a timer randomly, you need to specify the "
                "minimum and maximum time!", dn
            );
            valid = false;
        }
        
        
    } else if(n == "receive_status") {
    
        type = MOB_ACTION_RECEIVE_STATUS;
        if(status_types.find(v) == status_types.end()) {
            log_error("Unknown status effect \"" + v + "\"!", dn);
            valid = false;
        } else {
            vs.push_back(v);
        }
        
        
    } else if(n == "release") {
    
        type = MOB_ACTION_RELEASE;
        
        
    } else if(n == "remove_status") {
    
        type = MOB_ACTION_REMOVE_STATUS;
        if(!v.empty()) {
            bool exists = false;
            for(auto s = status_types.begin(); s != status_types.end(); s++) {
                if(s->first == v) {
                    exists = true;
                    break;
                }
            }
            if(exists) {
                vs.push_back(v);
            } else {
                log_error("Unknown status effect \"" + v + "\"!", dn);
                valid = false;
            }
        } else {
            log_error(
                "The \"remove_status\" action needs to know the status "
                "effect's name!", dn
            );
            valid = false;
        }
        
        
    } else if(n == "send_message_to_links") {
    
        type = MOB_ACTION_SEND_MESSAGE_TO_LINKS;
        if(v_words.size() >= 1) {
        
            string msg;
            for(size_t w = 0; w < v_words.size(); ++w) {
                msg += v_words[w] + " ";
            }
            vs.push_back(trim_spaces(msg));
            
        } else {
            log_error(
                "The message sending action needs to know the message!", dn
            );
            valid = false;
        }
        
        
    } else if(n == "send_message_to_nearby") {
    
        type = MOB_ACTION_SEND_MESSAGE_TO_NEARBY;
        if(v_words.size() >= 2) {
        
            vf.push_back(s2f(v_words[0]));
            string msg;
            for(size_t w = 1; w < v_words.size(); ++w) {
                msg += v_words[w] + " ";
            }
            vs.push_back(trim_spaces(msg));
            
        } else {
            log_error(
                "The message sending action needs to know the distance "
                "and the message!", dn
            );
            valid = false;
        }
        
        
    } else if(n == "set_animation") {
    
        type = MOB_ACTION_SET_ANIMATION;
        
        if(v.empty()) {
            log_error(
                "The animation setting action needs to know the animation "
                "name!", dn
            );
            valid = false;
        } else {
            size_t f_pos = mt->anims.find_animation(v_words[0]);
            if(f_pos == INVALID) {
                log_error("Unknown animation \"" + v_words[0] + "\"!", dn);
                valid = false;
            } else {
                vi.push_back(f_pos);
                vi.push_back(
                    v_words.size() > 1 && v_words[1] == "no_restart"
                );
            }
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
        
        
    } else if(n == "set_holdable") {
    
        type = MOB_ACTION_SET_HOLDABLE;
        
        unsigned char final_flags = 0;
        for(size_t word_nr = 0; word_nr < v_words.size(); ++word_nr) {
        
            if(v_words[word_nr] == "unholdable") {
                final_flags = 0;
                break;
                
            } else if(v_words[word_nr] == "pikmin") {
                final_flags |= HOLDABLE_BY_PIKMIN;
                
            } else if(v_words[word_nr] == "enemies") {
                final_flags |= HOLDABLE_BY_ENEMIES;
            }
        }
        
        vi.push_back(final_flags);
        
        
    } else if(n == "set_limb_animation") {
    
        type = MOB_ACTION_SET_LIMB_ANIMATION;
        
        if(v.empty()) {
            log_error(
                "The \"set_limb_animation\" action needs to know the "
                "name of the animation!", dn
            );
            valid = false;
        } else {
            vs.push_back(v);
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
        
        
    } else if(n == "set_team") {
    
        type = MOB_ACTION_SET_TEAM;
        if(!v.empty()) {
            size_t team_nr = string_to_team_nr(v);
            if(team_nr != INVALID) {
                vi.push_back(team_nr);
            } else {
                log_error("Unknown team name \"" + v + "\"!", dn);
                valid = false;
            }
        } else {
            log_error(
                "The \"set_team\" action needs to know the team name!", dn
            );
            valid = false;
        }
        
        
    } else if(n == "set_timer") {
    
        type = MOB_ACTION_SET_TIMER;
        
        if(v_words.size() >= 1) {
            vf.push_back(s2f(v_words[0]));
        } else {
            log_error("No timer amount specified!", dn);
            valid = false;
        }
        
        
    } else if(n == "set_var") {
    
        type = MOB_ACTION_SET_VAR;
        
        bool set_to_value = false;
        
        if(v_words.size() < 2) {
            log_error(
                "The \"set_var\" action needs to know the variable name and "
                "what to set it to!", dn
            );
            valid = false;
            return;
        }
        
        if(v_words.size() < 4) {
            set_to_value = true;
            
        } else {
            if(v_words[2] == "+") {
                vi.push_back(MOB_ACTION_SET_VAR_SUM);
            } else if(v_words[2] == "-") {
                vi.push_back(MOB_ACTION_SET_VAR_SUBTRACT);
            } else if(v_words[2] == "*") {
                vi.push_back(MOB_ACTION_SET_VAR_MULTIPLY);
            } else if(v_words[2] == "/") {
                vi.push_back(MOB_ACTION_SET_VAR_DIVIDE);
            } else if(v_words[2] == "%") {
                vi.push_back(MOB_ACTION_SET_VAR_MODULO);
            } else {
                set_to_value = true;
            }
        }
        
        if(set_to_value) {
            vs = v_words;
            vi.push_back(MOB_ACTION_SET_VAR_VALUE);
            
        } else {
            vs.push_back(v_words[0]);
            
            if(is_number(v_words[1])) {
                vf.push_back(s2f(v_words[1]));
                vs.push_back("");
            } else {
                vf.push_back(0);
                vs.push_back(v_words[1]);
            }
            
            if(is_number(v_words[3])) {
                vf.push_back(s2f(v_words[3]));
                vs.push_back("");
            } else {
                vf.push_back(0);
                vs.push_back(v_words[3]);
            }
        }
        
        
    } else if(n == "show_message_from_var") {
    
        type = MOB_ACTION_SHOW_MESSAGE_FROM_VAR;
        
        if(v.empty()) {
            log_error(
                "\"show_message_from_var\" needs to know the "
                "name of the variable that holds the text!", dn
            );
            valid = false;
        } else {
            vs.push_back(v);
        }
        
        
    } else if(n == "spawn") {
    
        type = MOB_ACTION_SPAWN;
        
        for(size_t s = 0; s < mt->spawns.size(); ++s) {
            if(mt->spawns[s].name == v) {
                vi.push_back(s);
                return;
            }
        }
        
        log_error("Spawn info block \"" + v + "\" not found!", dn);
        valid = false;
        
        
    } else if(n == "stabilize_z") {
    
        type = MOB_ACTION_STABILIZE_Z;
        
        if(v_words.size() >= 2) {
            if(v_words[0] == "highest") {
                vi.push_back(MOB_ACTION_STABILIZE_Z_HIGHEST);
            } else if(v_words[0] == "lowest") {
                vi.push_back(MOB_ACTION_STABILIZE_Z_LOWEST);
            } else {
                log_error(
                    "Unknown reference in the \"stabilize_z\" action: \"" +
                    v_words[0] + "\"!", dn
                );
                valid = false;
            }
            
            vf.push_back(s2f(v_words[1]));
        } else {
            log_error(
                "The \"stabilize_z\" action needs to know if the stabilization "
                "is regarding the higest or lowest linked object, and needs "
                "to know a Z offset!", dn
            );
            valid = false;
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
        
        
    } else if(n == "start_height_effect") {
    
        type = MOB_ACTION_START_HEIGHT_EFFECT;
        
        
    } else if(n == "start_particles") {
    
        type = MOB_ACTION_START_PARTICLES;
        if(v_words.empty()) {
            log_error(
                "The \"start_particles\" action needs to know the "
                "particle generator name!", dn
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
                
                if(v_words.size() >= 2) {
                    vf.push_back(s2f(v_words[1]));
                } else {
                    vf.push_back(0.0f);
                }
                if(v_words.size() >= 3) {
                    vf.push_back(s2f(v_words[2]));
                } else {
                    vf.push_back(0.0f);
                }
                if(v_words.size() >= 4) {
                    vf.push_back(s2f(v_words[3]));
                } else {
                    vf.push_back(0.0f);
                }
            }
        }
        
        
    } else if(n == "stop") {
    
        type = MOB_ACTION_STOP;
        
        
    } else if(n == "stop_chomping") {
    
        type = MOB_ACTION_STOP_CHOMPING;
        
        
    } else if(n == "stop_height_effect") {
    
        type = MOB_ACTION_STOP_HEIGHT_EFFECT;
        
        
    } else if(n == "stop_particles") {
    
        type = MOB_ACTION_STOP_PARTICLES;
        
        
    } else if(n == "stop_vertically") {
    
        type = MOB_ACTION_STOP_VERTICALLY;
        
        
    } else if(n == "swallow") {
    
        type = MOB_ACTION_SWALLOW;
        
        vi.push_back(s2i(v));
        
        
    } else if(n == "swallow_all") {
    
        type = MOB_ACTION_SWALLOW_ALL;
        
        
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
        } else if(v == "arachnorb_head_logic") {
            vi.push_back(MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC);
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
    
    switch(type) {
    case MOB_ACTION_ARACHNORB_PLAN_LOGIC: {

        m->arachnorb_plan_logic(vi[0]);
        
        break;
        
        
    } case MOB_ACTION_DELETE: {

        m->to_delete = true;
        
        break;
        
        
    } case MOB_ACTION_FOCUS: {

        if(vi[0] == MOB_ACTION_FOCUS_PARENT && m->parent) {
            m->focus_on_mob(m->parent->m);
            
        } else if(vi[0] == MOB_ACTION_FOCUS_TRIGGER) {
            if(
                parent_event == MOB_EVENT_OBJECT_IN_REACH ||
                parent_event == MOB_EVENT_OPPONENT_IN_REACH ||
                parent_event == MOB_EVENT_PIKMIN_LANDED ||
                parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                parent_event == MOB_EVENT_TOUCHED_OPPONENT
            ) {
                m->focus_on_mob((mob*) custom_data_1);
                
            } else if(
                parent_event == MOB_EVENT_RECEIVE_MESSAGE
            ) {
                m->focus_on_mob((mob*) custom_data_2);
            }
        }
        
        break;
        
        
    } case MOB_ACTION_FINISH_DYING: {

        m->finish_dying();
        
        break;
        
        
    } case MOB_ACTION_GET_CHOMPED: {

        if(parent_event == MOB_EVENT_HITBOX_TOUCH_EAT) {
            ((mob*) custom_data_1)->chomp(m, (hitbox*) custom_data_2);
        }
        
        break;
        
        
    } case MOB_ACTION_IF: {

        string lhs;
        string rhs;
        
        if(vi[1] == MOB_ACTION_IF_LHS_VAR) {
            lhs = m->vars[vs[0]];
        } else if(vi[1] == MOB_ACTION_IF_LHS_CHOMPED_PIKMIN) {
            lhs = i2s(m->chomping_mobs.size());
        } else if(vi[1] == MOB_ACTION_IF_LHS_DAY_MINUTES) {
            lhs = i2s(day_minutes);
        } else if(vi[1] == MOB_ACTION_IF_LHS_FIELD_PIKMIN) {
            lhs = i2s(pikmin_list.size());
        } else if(vi[1] == MOB_ACTION_IF_LHS_FRAME_SIGNAL) {
            if(parent_event == MOB_EVENT_FRAME_SIGNAL) {
                lhs = i2s(*((size_t*) custom_data_1));
            }
        } else if(vi[1] == MOB_ACTION_IF_LHS_HEALTH) {
            lhs = i2s(m->health);
        } else if(vi[1] == MOB_ACTION_IF_LHS_LATCHED_PIKMIN) {
            lhs = i2s(m->get_latched_pikmin_amount());
        } else if(vi[1] == MOB_ACTION_IF_LHS_LATCHED_PIKMIN_WEIGHT) {
            lhs = i2s(m->get_latched_pikmin_weight());
        } else if(vi[1] == MOB_ACTION_IF_LHS_MESSAGE) {
            if(parent_event == MOB_EVENT_RECEIVE_MESSAGE) {
                lhs = *((string*) custom_data_1);
            }
        } else if(vi[1] == MOB_ACTION_IF_LHS_MESSAGE_SENDER) {
            if(parent_event == MOB_EVENT_RECEIVE_MESSAGE) {
                lhs = ((mob*) custom_data_2)->type->name;
            }
        } else if(vi[1] == MOB_ACTION_IF_LHS_MOB_CATEGORY) {
            if(
                parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
                parent_event == MOB_EVENT_OBJECT_IN_REACH ||
                parent_event == MOB_EVENT_OPPONENT_IN_REACH
            ) {
                lhs = ((mob*) custom_data_1)->type->category->name;
            }
        } else if(vi[1] == MOB_ACTION_IF_LHS_MOB_TYPE) {
            if(
                parent_event == MOB_EVENT_TOUCHED_OBJECT ||
                parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
                parent_event == MOB_EVENT_OBJECT_IN_REACH ||
                parent_event == MOB_EVENT_OPPONENT_IN_REACH ||
                parent_event == MOB_EVENT_PIKMIN_LANDED
            ) {
                lhs = ((mob*) custom_data_1)->type->name;
            }
        } else if(vi[1] == MOB_ACTION_IF_LHS_BODY_PART) {
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
        } else if(vi[1] == MOB_ACTION_IF_LHS_OTHER_BODY_PART) {
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
        } else {
            return false;
        }
        
        if(vi[2] == MOB_ACTION_IF_RHS_VAR) {
            rhs = m->vars[vs[1]];
        } else {
            rhs = vs[1];
        }
        
        if(vi[0] == MOB_ACTION_IF_OP_EQUAL) {
            if(is_number(lhs)) {
                return (s2f(lhs) == s2f(rhs));
            } else {
                return (lhs == rhs);
            }
        } else if(vi[0] == MOB_ACTION_IF_OP_NOT) {
            if(is_number(lhs)) {
                return (s2f(lhs) != s2f(rhs));
            } else {
                return (lhs != rhs);
            }
        } else if(vi[0] == MOB_ACTION_IF_OP_LESS) {
            return (s2f(lhs) < s2f(rhs));
        } else if(vi[0] == MOB_ACTION_IF_OP_MORE) {
            return (s2f(lhs) > s2f(rhs));
        } else if(vi[0] == MOB_ACTION_IF_OP_LESS_E) {
            return (s2f(lhs) <= s2f(rhs));
        } else if(vi[0] == MOB_ACTION_IF_OP_MORE_E) {
            return (s2f(lhs) >= s2f(rhs));
        }
        
        break;
        
        
    } case MOB_ACTION_MOVE_TO_ABSOLUTE: {

        m->chase(point(vf[0], vf[1]), NULL, false);
        
        break;
        
    } case MOB_ACTION_MOVE_TO_RELATIVE: {

        point p = rotate_point(point(vf[0], vf[1]), m->angle);
        m->chase(m->pos + p, NULL, false);
        
        break;
        
    } case MOB_ACTION_MOVE_TO_TARGET: {

        if(vi[0] == MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB) {
            if(m->focused_mob) {
                float a = get_angle(m->pos, m->focused_mob->pos);
                point offset = point(2000, 0);
                offset = rotate_point(offset, a + TAU / 2.0);
                m->chase(m->pos + offset, NULL, false);
            } else {
                m->stop_chasing();
            }
            
        } else if(vi[0] == MOB_ACTION_MOVE_FOCUSED_MOB) {
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
            
        } else if(vi[0] == MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC) {
            m->arachnorb_foot_move_logic();
            
        } else if(vi[0] == MOB_ACTION_MOVE_LINKED_MOB_AVERAGE) {
            if(m->links.empty()) return false;
            
            point des;
            for(size_t l = 0; l < m->links.size(); ++l) {
                des += m->links[l]->pos;
            }
            des = des / m->links.size();
            
            m->chase(des, NULL, false);
            
        } else if(vi[0] == MOB_ACTION_MOVE_RANDOMLY) {
            m->chase(
                point(
                    m->pos.x + randomf(-1000, 1000),
                    m->pos.y + randomf(-1000, 1000)
                ),
                NULL, false
            );
            
        }
        
        break;
        
        
    } case MOB_ACTION_ORDER_RELEASE: {

        if(m->holder.m) {
            m->holder.m->fsm.run_event(MOB_EVENT_RELEASE_ORDER, NULL, NULL);
        }
        
        break;
        
        
    } case MOB_ACTION_RANDOMIZE_VAR: {

        m->vars[vs[0]] = i2s(randomi(vi[0], vi[1]));
        
        break;
        
        
    } case MOB_ACTION_RANDOMIZE_TIMER: {

        m->set_timer(randomf(vf[0], vf[1]));
        
        break;
        
        
    } case MOB_ACTION_RECEIVE_STATUS: {

        m->apply_status_effect(&status_types[vs[0]], true, false);
        
        break;
        
        
    } case MOB_ACTION_RELEASE: {

        m->release_chomped_pikmin();
        
        break;
        
        
    } case MOB_ACTION_REMOVE_STATUS: {

        for(size_t s = 0; s < m->statuses.size(); ++s) {
            if(m->statuses[s].type->name == vs[0]) {
                m->statuses[s].to_delete = true;
            }
        }
        
        break;
        
        
    } case MOB_ACTION_SEND_MESSAGE_TO_LINKS: {

        for(size_t l = 0; l < m->links.size(); ++l) {
            if(m->links[l] == m) continue;
            m->send_message(m->links[l], vs[0]);
        }
        
        break;
        
        
    } case MOB_ACTION_SEND_MESSAGE_TO_NEARBY: {

        for(size_t m2 = 0; m2 < mobs.size(); ++m2) {
            if(mobs[m2] == m) continue;
            if(dist(m->pos, mobs[m2]->pos) > vf[0]) continue;
            m->send_message(mobs[m2], vs[0]);
        }
        
        break;
        
        
    } case MOB_ACTION_SET_ANIMATION: {

        m->set_animation(vi[0], false, vi[1] == 0);
        
        break;
        
        
    } case MOB_ACTION_SET_FAR_REACH: {

        m->far_reach = vi[0];
        
        break;
        
        
    } case MOB_ACTION_SET_GRAVITY: {

        m->gravity_mult = vf[0];
        
        break;
        
        
    } case MOB_ACTION_SET_HEALTH: {

        m->set_health(
            vi[0] == MOB_ACTION_NUMERICAL_RELATIVE,
            false,
            vf[0]
        );
        
        break;
        
        
    } case MOB_ACTION_SET_HIDING: {

        m->hide = vi[0];
        
        break;
        
        
    } case MOB_ACTION_SET_HOLDABLE: {

        if(typeid(*m) == typeid(tool)) {
            ((tool*) m)->holdability_flags = vi[0];
        }
        
        break;
        
        
    } case MOB_ACTION_SET_NEAR_REACH: {

        m->near_reach = vi[0];
        
        break;
        
        
    } case MOB_ACTION_SET_LIMB_ANIMATION: {

        if(!m->parent) return false;
        if(!m->parent->limb_anim.anim_db) return false;
        size_t a = m->parent->limb_anim.anim_db->find_animation(vs[0]);
        if(a == INVALID) return false;
        m->parent->limb_anim.cur_anim =
            m->parent->limb_anim.anim_db->animations[a];
        m->parent->limb_anim.start();
        
        break;
        
        
    } case MOB_ACTION_SET_STATE: {

        m->fsm.set_state(vi[0], custom_data_1, custom_data_2);
        
        break;
        
        
    } case MOB_ACTION_SET_TANGIBLE: {

        m->tangible = (bool) vi[0];
        
        break;
        
        
    } case MOB_ACTION_SET_TEAM: {

        m->team = vi[0];
        
        break;
        
        
    } case MOB_ACTION_SET_TIMER: {

        m->set_timer(vf[0]);
        
        break;
        
        
    } case MOB_ACTION_SET_VAR: {

        if(vi[0] == MOB_ACTION_SET_VAR_VALUE) {
            m->set_var(vs[0], vs[1]);
            
        } else {
            float lhs, rhs, result;
            if(vs[1].empty()) {
                lhs = vf[0];
            } else {
                lhs = s2f(m->vars[vs[1]]);
            }
            
            if(vs[2].empty()) {
                rhs = vf[1];
            } else {
                rhs = s2f(m->vars[vs[2]]);
            }
            
            if(vi[0] == MOB_ACTION_SET_VAR_SUM) {
                result = lhs + rhs;
            } else if(vi[0] == MOB_ACTION_SET_VAR_SUBTRACT) {
                result = lhs - rhs;
            } else if(vi[0] == MOB_ACTION_SET_VAR_MULTIPLY) {
                result = lhs * rhs;
            } else if(vi[0] == MOB_ACTION_SET_VAR_DIVIDE) {
                if(rhs == 0) {
                    result = 0;
                } else {
                    result = lhs / rhs;
                }
            } else {
                if(rhs == 0) {
                    result = 0;
                } else {
                    result = fmod(lhs, rhs);
                }
            }
            
            m->vars[vs[0]] = f2s(result);
        }
        
        break;
        
        
    } case MOB_ACTION_SHOW_MESSAGE_FROM_VAR: {

        start_message(m->vars[vs[0]], NULL);
        
        break;
        
        
    } case MOB_ACTION_SPAWN: {

        return m->spawn(&m->type->spawns[vi[0]]);
        
        break;
        
        
    } case MOB_ACTION_STABILIZE_Z: {

        if(m->links.empty()) return false;
        float best_match_z = m->links[0]->z;
        for(size_t l = 1; l < m->links.size(); ++l) {
            if(
                vi[0] == MOB_ACTION_STABILIZE_Z_HIGHEST &&
                m->links[l]->z > best_match_z
            ) {
                best_match_z = m->links[l]->z;
            } else if(
                vi[0] == MOB_ACTION_STABILIZE_Z_LOWEST &&
                m->links[l]->z < best_match_z
            ) {
                best_match_z = m->links[l]->z;
            }
        }
        
        m->z = best_match_z + vf[0];
        
        break;
        
        
    } case MOB_ACTION_START_DYING: {

        m->start_dying();
        
        break;
        
        
    } case MOB_ACTION_START_CHOMPING: {

        m->chomp_max = vi[0];
        m->chomp_body_parts.clear();
        for(size_t p = 1; p < vi.size(); ++p) {
            m->chomp_body_parts.push_back(vi[p]);
        }
        
        break;
        
        
    } case MOB_ACTION_START_HEIGHT_EFFECT: {

        m->start_height_effect();
        
        break;
        
        
    } case MOB_ACTION_START_PARTICLES: {

        if(vs.empty()) {
            m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
        } else {
            if(
                custom_particle_generators.find(vs[0]) !=
                custom_particle_generators.end()
            ) {
                particle_generator pg = custom_particle_generators[vs[0]];
                pg.id = MOB_PARTICLE_GENERATOR_SCRIPT;
                pg.follow_mob = m;
                pg.follow_angle = &m->angle;
                pg.follow_pos_offset = point(vf[0], vf[1]);
                pg.follow_z_offset = vf[2];
                pg.reset();
                m->particle_generators.push_back(pg);
            }
        }
        
        break;
        
        
    } case MOB_ACTION_STOP: {

        m->stop_chasing();
        m->stop_turning();
        
        break;
        
        
    } case MOB_ACTION_STOP_CHOMPING: {

        m->chomp_max = 0;
        m->chomp_body_parts.clear();
        
        break;
        
        
    } case MOB_ACTION_STOP_HEIGHT_EFFECT: {

        m->stop_height_effect();
        
        break;
        
        
    } case MOB_ACTION_STOP_PARTICLES: {

        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
        
        break;
        
        
    } case MOB_ACTION_STOP_VERTICALLY: {

        m->speed_z = 0;
        
        break;
        
        
    } case MOB_ACTION_SWALLOW: {

        m->swallow_chomped_pikmin(vi[1]);
        
        break;
        
        
    } case MOB_ACTION_SWALLOW_ALL: {

        m->swallow_chomped_pikmin(m->chomping_mobs.size());
        
        break;
        
        
    } case MOB_ACTION_TELEPORT: {

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
        
        break;
        
        
    } case MOB_ACTION_TURN: {

        if(vi[0] == MOB_ACTION_TURN_ABSOLUTE) {
            m->face(vf[0], NULL);
        } else if(vi[0] == MOB_ACTION_TURN_RELATIVE) {
            m->face(m->angle + vf[0], NULL);
        } else if(vi[0] == MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC) {
            m->arachnorb_head_turn_logic();
        } else if(vi[0] == MOB_ACTION_TURN_FOCUSED_MOB && m->focused_mob) {
            m->face(0, &m->focused_mob->pos);
        } else if(vi[0] == MOB_ACTION_TURN_HOME) {
            m->face(get_angle(m->pos, m->home), NULL);
        } else if(vi[0] == MOB_ACTION_TURN_RANDOMLY) {
            m->face(randomf(0, TAU), NULL);
        }
        
        break;
        
        
    }
    }
    
    return false;
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
