#include <allegro5/allegro_font.h>

#include "../functions.h"
#include "info_spot.h"
#include "../vars.h"

info_spot::info_spot(float x, float y, sector* sec, string text, bool fullscreen, ALLEGRO_FONT* font)
    : mob(x, y, sec->floors[0].z, info_spot_mob_type, sec) {
    
    this->text = text;
    this->fullscreen = fullscreen;
    text_w = 0;
    
    vector<string> lines = split(text, "\n");
    size_t n_lines = lines.size();
    for(size_t l = 0; l < n_lines; l++) {
        unsigned int line_w = al_get_text_width(font, lines[l].c_str());
        if(line_w > text_w) text_w = line_w;
    }
    
    //ToDo remove
    //timer_interval = timer = 2;
    /*vector<mob_action*> see_actions = vector<mob_action*>(1, new mob_action(MOB_ACTION_MOVE, "pikmin"));
    vector<mob_action*> lose_actions = vector<mob_action*>(1, new mob_action(MOB_ACTION_MOVE, "home"));
    lose_actions.push_back(new mob_action(MOB_ACTION_WAIT, "2"));
    lose_actions.push_back(new mob_action(MOB_ACTION_SET_HEALTH, "20"));
    events.push_back(new mob_event(MOB_EVENT_SEE_PIKMIN, see_actions));
    events.push_back(new mob_event(MOB_EVENT_LOSE_PIKMIN, lose_actions));
    events.push_back(new mob_event(MOB_EVENT_NEAR_PIKMIN, see_actions));
    events.push_back(new mob_event(MOB_EVENT_TIMER, see_actions));*/
}