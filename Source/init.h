/*
 * Copyright (c) Andre 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for functions that initialize the game data and content.
 */

#ifndef INIT_INCLUDED
#define INIT_INCLUDED

void init_allegro();
void init_controls();
void init_dev_tools();
void init_error_bitmap();
void init_event_things(ALLEGRO_TIMER* &timer, ALLEGRO_EVENT_QUEUE* &queue);
void init_fonts();
void init_game_states();
void init_misc();
void init_misc_graphics();
void init_misc_sounds();
void init_mob_categories();
void init_sector_types();
void init_special_mob_types();

#endif //ifndef INIT_INCLUDED
