/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Loader thread class and loader thread-related functions.
 */

#include <allegro5/allegro.h>

#include "loader_thread.h"

/* ----------------------------------------------------------------------------
 * Creates a loading thread.
 * t: pointer to a function, that takes an Allegro thread pointer (unused),
   * and a void* that points to the loading thread instance.
 */
loader_thread::loader_thread(void* (*t)(ALLEGRO_THREAD* thread, void* arg)) {
    if(!t) return;
    thread = al_create_thread(t, (void*) this);
    percentage_done = 0;
    step = 0;
}


/* ----------------------------------------------------------------------------
 * Starts the thread.
 */
void loader_thread::start() {
    al_start_thread(thread);
}


/* ----------------------------------------------------------------------------
 * Waits until the thread is done. Should be used by
 * the main thread, when it wants to join threads.
 */
void loader_thread::wait() {
    al_join_thread(thread, NULL);
}


/* ----------------------------------------------------------------------------
 * Destroys the thread. Remember to call this when
 * you don't need it any more.
 */
void loader_thread::destroy() {
    al_destroy_thread(thread);
}
