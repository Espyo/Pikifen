/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the parallel code utilities.
 */

#ifndef PARALLEL_INCLUDED
#define PARALLEL_INCLUDED

#include <functional>
#include <vector>

using namespace std;

namespace parallel {


/* ----------------------------------------------------------------------------
 * This structure holds information necessary to run a
 * parallel for task.
 */
struct parallel_for_task_info {
    size_t begin, end;
    function<void(size_t)> code;
    parallel_for_task_info(const size_t b, const size_t e, const function<void(size_t)> c);
};


/* ----------------------------------------------------------------------------
 * Holds information on a thread; particularly, what it should do next.
 */
class task_thread {
private:
    ALLEGRO_THREAD* thread;
    ALLEGRO_MUTEX* mutex;       // Main mutex.
    ALLEGRO_MUTEX* done_mutex;  // Mutex to be used for signaling that the task is done.
    ALLEGRO_COND* start_signal; // Signal the main thread uses to warn this thread that it must start working.
    ALLEGRO_COND* done_signal;  // Signal this thread uses to warn the main thread that it finished working what it was told to.
    bool task_is_done;          // If the main thread doesn't catch the "done" signal, it can check this.
    bool task_is_waiting;       // If this thread doesn't catch the "start" signal, it can check this.
    void* extra_task_info;      // Extra info, JUST for the task's code.
    function<void(void*)> code;
    
    static void* thread_code(ALLEGRO_THREAD*, void* c);
    
public:
    task_thread();
    void start_task(function<void(void*)> code, void* extra_task_info = NULL);
    void wait_until_done();
    void join_and_destroy();
};


/* ----------------------------------------------------------------------------
 * A class that manages the task threads.
 * Use this to parallelize tasks.
 * Don't use this for really short tasks,
 * as it will likely take longer to parallelize them
 * than not.
 */
class task_thread_manager {
private:
    vector<task_thread*> task_threads;
    
    static void parallel_for_task_code(void* extra_task_info);
    
public:
    task_thread_manager(size_t n_threads = 0);
    void destroy();
    
    void run_task_group(
        function<void(void*)> task1,
        function<void(void*)> task2,
        function<void(void*)> task3  = NULL,
        function<void(void*)> task4  = NULL,
        function<void(void*)> task5  = NULL,
        function<void(void*)> task6  = NULL,
        function<void(void*)> task7  = NULL,
        function<void(void*)> task8  = NULL,
        function<void(void*)> task9  = NULL,
        function<void(void*)> task10 = NULL
    );
    void parallel_for(size_t begin, size_t end, const function<void(size_t)> iteration_code);
};


/* ----------------------------------------------------------------------------
 * This thread makes it easy to create threads that load content.
 * Just give it a function that loads data, and you're ready to go.
 * The loading function should take a pointer to the Allegro thread (unused)
 * as well as a pointer to its loader_thread object, used to update the
 * loading percentage. The loader thread should update the percentage as
 * smoothly as it decides. Meanwhile, the main thread is in charge of
 * waiting for it, and drawing a number on-screen, or something.
 */
class loader_thread {
public:
    ALLEGRO_THREAD* thread;
    function<void(loader_thread*)> task;
    unsigned char percentage_done; // 0 to 100.
    // The current step in the loading process.
    // The loader and main threads should decide what this means.
    // It gets initialized as 0.
    unsigned int step;
    
    static void* loader_thread_code(ALLEGRO_THREAD* t, void* arg);
    
    loader_thread();
    void set_task(function<void(loader_thread*)> t = NULL);
    void start();
    void wait();
    void destroy();
};

}

#endif // ifndef PARALLEL_INCLUDED

