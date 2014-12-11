/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the loader thread class and loader thread-related functions.
 */

#ifndef LOADER_THREAD_H
#define LOADER_THREAD_H

/*
 * This thread makes it easy to create threads that load content.
 * Just give it a function that loads data, and you're ready to go.
 * The loading function should take a pointer to the Allegro thread (unused)
 * as well as a pointer to its loader_thread object, used to update the
 * loading percentage. The loader thread should update the percentage as
 * smoothly as it decides. Meanwhile, the main thread is in charge of
 * waiting for it, and drawing a number on-screen, or something.
 */

class loader_thread{
public:
    ALLEGRO_THREAD* thread;
    unsigned char percentage_done; //0 to 100.
    //The current step in the loading process.
    //The loader and main threads should decide what this means.
    //It gets initialized as 0.
    unsigned int step;

    loader_thread(void* (*t)(ALLEGRO_THREAD *thread, void *arg) = NULL);
    void start();
    void wait();
    void destroy();
};

#endif //ifndef LOADER_THREAD_H
