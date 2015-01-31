/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Parallel code utilities.
 */

#include <algorithm>
#include <vector>

#include <allegro5/allegro.h>

#include "parallel.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Creates a parallel for task info structure.
 */
parallel::parallel_for_task_info::parallel_for_task_info(const size_t b, const size_t e, const function<void(size_t)> c) {
    begin = b; end = e; code = c;
}



/* ----------------------------------------------------------------------------
 * The code for the task threads.
 * They just wait until the main thread tells them they have a task.
 * When they complete it, they signal the main thread about their completion.
 * Then, they go back to waiting.
 */
void* parallel::task_thread::thread_code(ALLEGRO_THREAD* t, void* i) {

    task_thread* info = (task_thread*) i;
    
    while(!al_get_thread_should_stop(t)) {
        al_lock_mutex(info->mutex); {
        
            // Wait until the main thread tells this thread that there is work to do.
            if(!info->task_is_waiting) {
                al_wait_cond(info->start_signal, info->mutex);
            }
            
            info->task_is_waiting = false;
            
            // If the current "task" is to quit, then do so.
            if(al_get_thread_should_stop(t)) return NULL;
            
            // Run the task.
            info->code(info->extra_task_info);
            
            // We're done here. Signal the main thread that the task is done.
            // In case the main thread misses the signal, we'll also set
            // a flag, so it can check that.
            info->task_is_done = true;
            al_broadcast_cond(info->done_signal);
            
        } al_unlock_mutex(info->mutex);
        
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Creates a task thread.
 */
parallel::task_thread::task_thread() {
    thread          = al_create_thread(thread_code, (void*) this);
    mutex           = al_create_mutex();
    done_mutex      = al_create_mutex();
    start_signal    = al_create_cond();
    done_signal     = al_create_cond();
    task_is_waiting = false;
    task_is_done    = false;
    
    al_start_thread(thread);
}


/* ----------------------------------------------------------------------------
 * Joins this thread with the main thread, and then destroys this thread.
 */
void parallel::task_thread::join_and_destroy() {
    al_set_thread_should_stop(thread);
    // Let's send a signal that it has to start working,
    // even though we actually just want to wake it up,
    // so it can quit.
    task_is_waiting = true;
    al_broadcast_cond(start_signal);
    
    al_join_thread(thread, NULL);
    al_destroy_thread(thread);
}


/* ----------------------------------------------------------------------------
 * Sets the task the thread should do now, and makes it start progress on it.
 */
void parallel::task_thread::start_task(function<void(void*)> code, void* extra_task_info) {
    al_lock_mutex(mutex); {
        this->code = code;
        this->extra_task_info = extra_task_info;
        task_is_done = false;
        
        // We'll set a flag as well as the signal, in case the signal
        // is sent before the task is ready to catch it.
        task_is_waiting = true;
        al_broadcast_cond(start_signal);
        
    } al_unlock_mutex(mutex);
}


/* ----------------------------------------------------------------------------
 * Returns only when the thread's task is complete.
 */
void parallel::task_thread::wait_until_done() {
    al_lock_mutex(done_mutex); {
    
        // First, check if the task was completed before
        // the code got here. If not, then we wait for the signal.
        if(!task_is_done) {
            al_wait_cond(done_signal, done_mutex);
        }
        
    } al_unlock_mutex(done_mutex);
}



/* ----------------------------------------------------------------------------
 * The task code that runs the iterations requested, for
 * the parallel for function.
 */
void parallel::task_thread_manager::parallel_for_task_code(void* extra_task_info) {
    parallel_for_task_info* info = (parallel_for_task_info*) extra_task_info;
    
    for(size_t i = info->begin; i < info->end; i++) {
        info->code(i);
    }
}


/* ----------------------------------------------------------------------------
 * Sets up the system so that it allocates n threads that will be
 * used for tasks in the future.
 * If n_threads is 0, nothing gets allocated.
 */
parallel::task_thread_manager::task_thread_manager(size_t n_threads) {
    for(size_t t = 0; t < n_threads; t++) {
        task_threads.push_back(new task_thread());
    }
}


/* ----------------------------------------------------------------------------
 * Waits for the task threads and destroys them.
 */
void parallel::task_thread_manager::destroy() {
    for(size_t t = 0; t < task_threads.size(); t++) {
        task_threads[t]->join_and_destroy();
    }
    task_threads.clear();
}


/* ----------------------------------------------------------------------------
 * Runs all of the supplied tasks at once. The tasks run
 * simultaneously, each in their own thread.
 * This is meant for simple tasks, so don't go crazy.
 * The function returns when all threads are done.
 * Tasks 3 to 10 are optional.
 */
void parallel::task_thread_manager::run_task_group(
    function<void(void*)> task1,
    function<void(void*)> task2,
    function<void(void*)> task3,
    function<void(void*)> task4,
    function<void(void*)> task5,
    function<void(void*)> task6,
    function<void(void*)> task7,
    function<void(void*)> task8,
    function<void(void*)> task9,
    function<void(void*)> task10
) {
    function<void(void*)> functions[10] = {
        task1, task2, task3, task4, task5,
        task6, task7, task8, task9, task10
    };
    
    size_t n_tasks = 0;
    size_t first_task = 0;
    for(; n_tasks < 10; n_tasks++) if(!functions[n_tasks]) break;
    
    while(first_task < n_tasks) {
    
        size_t last_task = (first_task + min(n_tasks - first_task, task_threads.size())) - 1;
        
        for(size_t t = first_task; t <= last_task; t++) {
            task_threads[t % task_threads.size()]->start_task(functions[t]);
        }
        
        for(size_t t = first_task; t <= last_task; t++) {
            task_threads[t % task_threads.size()]->wait_until_done();
        }
        
        first_task = last_task + 1;
        
    }
}



/* ----------------------------------------------------------------------------
 * The parallel for runs a for loop (linearly increasing), except it splits
 * the iterations through several threads. As such, do not run this
 * on things that may lead to race conditions, amongst other nasty things.
 * The function returns when all threads are done iterating.
 * begin, end: start (inclusive) and end (exclusive) values of the iteration.
 * code:       code to run for each iteration.
 */
void parallel::task_thread_manager::parallel_for(size_t begin, size_t end, const function<void(size_t)> code) {

    vector<parallel_for_task_info> thread_infos;
    
    size_t n_threads = task_threads.size();
    
    size_t total = end - begin;
    n_threads = min(n_threads, total);
    size_t n_iterations = ceil(total / (float) n_threads);
    
    for(size_t t = 0; t < n_threads; t++) {
    
        thread_infos.push_back(parallel_for_task_info(
                                   t * n_iterations,
                                   min(total, (t + 1) * n_iterations),
                                   code
                               ));
    }
    
    // This cannot be on the same loop as the previous one
    // because as the thread infos are added, they're moved in
    // memory. This would break the pointers.
    for(size_t t = 0; t < n_threads; t++) {
        task_threads[t]->start_task(parallel_for_task_code, (void*) &thread_infos[t]);
    }
    
    for(size_t t = 0; t < n_threads; t++) {
        task_threads[t]->wait_until_done();
    }
    
}



/* ----------------------------------------------------------------------------
 * Code for the Allegro thread, to run the provided loading task.
 */
void* parallel::loader_thread::loader_thread_code(ALLEGRO_THREAD*, void* a) {
    loader_thread* t = (loader_thread*) a;
    
    t->task(t);
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Creates the loader thread.
 */
parallel::loader_thread::loader_thread() {
    task = NULL;
    thread = NULL;
    percentage_done = 0;
    step = 0;
}


/* ----------------------------------------------------------------------------
 * Sets what the loading thread should do.
 * t: pointer to a function, with the loading code.
 */
void parallel::loader_thread::set_task(function<void(loader_thread*)> t) {
    if(!t) return;
    task = t;
    thread = al_create_thread(loader_thread_code, (void*) this);
    percentage_done = 0;
    step = 0;
}


/* ----------------------------------------------------------------------------
 * Starts the thread.
 */
void parallel::loader_thread::start() {
    al_start_thread(thread);
}


/* ----------------------------------------------------------------------------
 * Waits until the thread is done. Should be used by
 * the main thread, when it wants to join threads.
 */
void parallel::loader_thread::wait() {
    al_join_thread(thread, NULL);
}


/* ----------------------------------------------------------------------------
 * Destroys the thread. Remember to call this when
 * you don't need it any more.
 */
void parallel::loader_thread::destroy() {
    al_destroy_thread(thread);
}
