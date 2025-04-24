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


double codeDebugBenchmarkMeasureStart;
double codeDebugBenchmarkSum;
unsigned int codeDebugBenchmarkIterations;


#ifdef CODE_DEBUG_NEW

map<void*, string> codeDebugNewAllocs;
bool codeDebugNewRecording;

/**
 * @brief Overrides operator delete.
 *
 * @param ptr Pointer to memory to deallocate.
 */
void operator delete(void* ptr) noexcept {
    if(codeDebugNewRecording) {
        map<void*, string>::iterator it = codeDebugNewAllocs.find(ptr);
        if(it != codeDebugNewAllocs.end()) {
            codeDebugNewAllocs.erase(it);
        }
    }
    return free(ptr);
}


/**
 * @brief Overrides operator delete[].
 *
 * @param ptr Pointer to memory to deallocate.
 */
void operator delete[](void* ptr) noexcept {
    if(codeDebugNewRecording) {
        map<void*, string>::iterator it = codeDebugNewAllocs.find(ptr);
        if(it != codeDebugNewAllocs.end()) {
            codeDebugNewAllocs.erase(it);
        }
    }
    return free(ptr);
}


/**
 * @brief Overrides operator new.
 *
 * @param size Size of memory to allocate.
 * @param file Name of the code file that requested this allocation.
 * @param line Line in the code file that requested this allocation.
 */
void* operator new(size_t size, char* file, int line) {
    void* ptr = malloc(size);
    if(codeDebugNewRecording) {
        codeDebugNewAllocs[ptr] =
            string(file) + ":" + to_string((long long) line);
    }
    return ptr;
}


/**
 * @brief Overrides operator new[].
 *
 * @param size Size of memory to allocate.
 * @param file Name of the code file that requested this allocation.
 * @param line Line in the code file that requested this allocation.
 */
void* operator new[](size_t size, char* file, int line) {
    void* ptr = malloc(size);
    if(codeDebugNewRecording) {
        codeDebugNewAllocs[ptr] =
            string(file) + ":" + to_string((long long) line);
    }
    return ptr;
}


#endif //ifndef CODE_DEBUG_NEW



/**
 * @brief Starts a time measurement for benchmarking.
 */
void codeDebugBenchmarkStartMeasuring() {
    codeDebugBenchmarkMeasureStart = al_get_time();
}


/**
 * @brief Finishes a time measurement for benchmarking. Stores and returns the
 * time difference.
 *
 * @return The time difference.
 */
double codeDebugBenchmarkEndMeasuring() {
    double duration = al_get_time() - codeDebugBenchmarkMeasureStart;
    codeDebugBenchmarkSum += duration;
    codeDebugBenchmarkIterations++;
    return duration;
}


/**
 * @brief Returns the average duration of all measurements taken so far.
 *
 * @return The average duration.
 */
double codeDebugBenchmarkGetAvgDuration() {
    if(codeDebugBenchmarkIterations == 0) return 0.0f;
    return
        codeDebugBenchmarkSum / (double) codeDebugBenchmarkIterations;
}
