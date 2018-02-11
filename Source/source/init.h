/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for program initializer and deinitializer functions.
 */

#ifndef INIT_INCLUDED
#define INIT_INCLUDED

#include "animation.h"
#include "data_file.h"

void init_allegro();
void init_asset_file_names();
void init_controls();
void init_error_bitmap();
void init_essentials();
void init_event_things(ALLEGRO_TIMER* &timer, ALLEGRO_EVENT_QUEUE* &queue);
void init_game_states();
void init_hud_items();
void init_misc();
void init_mob_categories();
void init_sector_types();
void init_single_animation(
    data_node* anim_def_file, const string &name,
    single_animation_suite &anim
);

void destroy_allegro();
void destroy_event_things(ALLEGRO_TIMER* &timer, ALLEGRO_EVENT_QUEUE* &queue);
void destroy_game_states();
void destroy_misc();
void destroy_mob_categories();

#endif //ifndef INIT_INCLUDED
