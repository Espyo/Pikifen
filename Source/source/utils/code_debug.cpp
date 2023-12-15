/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Code debugging tools. See the header file for more information.
 */

#include <allegro5/allegro.h>

#include "code_debug.h"


double code_debug_benchmark_measure_start;
double code_debug_benchmark_sum;
unsigned int code_debug_benchmark_iterations;


#ifdef CODE_DEBUG_NEW

map<void*, string> code_debug_new_allocs;
bool code_debug_new_recording;

/* ----------------------------------------------------------------------------
 * Overrides operator delete.
 * ptr:
 *   Pointer to memory to deallocate.
 */
void operator delete(void* ptr) noexcept {
    if(code_debug_new_recording) {
        map<void*, string>::iterator it = code_debug_new_allocs.find(ptr);
        if(it != code_debug_new_allocs.end()) {
            code_debug_new_allocs.erase(it);
        }
    }
    return free(ptr);
}


/* ----------------------------------------------------------------------------
 * Overrides operator delete[].
 * ptr:
 *   Pointer to memory to deallocate.
 */
void operator delete[](void* ptr) noexcept {
    if(code_debug_new_recording) {
        map<void*, string>::iterator it = code_debug_new_allocs.find(ptr);
        if(it != code_debug_new_allocs.end()) {
            code_debug_new_allocs.erase(it);
        }
    }
    return free(ptr);
}


/* ----------------------------------------------------------------------------
 * Overrides operator new.
 * size:
 *   Size of memory to allocate.
 * file:
 *   Name of the code file that requested this allocation.
 * line:
 *   Line in the code file that requested this allocation.
 */
void* operator new(size_t size, char* file, int line) {
    void* ptr = malloc(size);
    if(code_debug_new_recording) {
        code_debug_new_allocs[ptr] =
            string(file) + ":" + to_string((long long) line);
    }
    return ptr;
}


/* ----------------------------------------------------------------------------
 * Overrides operator new[].
 * size:
 *   Size of memory to allocate.
 * file:
 *   Name of the code file that requested this allocation.
 * line:
 *   Line in the code file that requested this allocation.
 */
void* operator new[](size_t size, char* file, int line) {
    void* ptr = malloc(size);
    if(code_debug_new_recording) {
        code_debug_new_allocs[ptr] =
            string(file) + ":" + to_string((long long) line);
    }
    return ptr;
}

#endif //ifndef CODE_DEBUG_NEW



/* ----------------------------------------------------------------------------
 * Starts a time measurement for benchmarking.
 */
void code_debug_benchmark_start_measuring() {
    code_debug_benchmark_measure_start = al_get_time();
}


/* ----------------------------------------------------------------------------
 * Finishes a time measurement for benchmarking. Stores and returns the
 * time difference.
 */
double code_debug_benchmark_end_measuring() {
    double duration = al_get_time() - code_debug_benchmark_measure_start;
    code_debug_benchmark_sum += duration;
    code_debug_benchmark_iterations++;
    return duration;
}


/* ----------------------------------------------------------------------------
 * Returns the average duration of all measurements taken so far.
 */
double code_debug_benchmark_get_avg_duration() {
    if(code_debug_benchmark_iterations == 0) return 0.0f;
    return
        code_debug_benchmark_sum / (double) code_debug_benchmark_iterations;
}
