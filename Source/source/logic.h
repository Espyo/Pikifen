/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the main game loop logic.
 */

#ifndef LOGIC_INCLUDED
#define LOGIC_INCLUDED

void do_aesthetic_logic();
void do_gameplay_logic();
void process_mob(mob* m_ptr, size_t m);

#endif //ifndef LOGIC_INCLUDED
