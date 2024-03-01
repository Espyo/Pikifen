/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the code debugging tools.
 */

#ifndef CODE_DEBUG_INCLUDED
#define CODE_DEBUG_INCLUDED

/**
 * @brief Memory allocation and memory leak debug.
 *
 * To activate:
 *   1. Define CODE_DEBUG_NEW.
 *   2. Include this header file in all cpp files.
 *   3. Do a global project replace of "new" to "code_debug_new",
 *     and "delete" to "code_debug_delete". Enable case-sensitivity and
 *     "whole word" matching, and don't replace anything in this
 *     header file or the cpp file.
 *   4. At some point in your code, start recording allocations by doing
 *     "code_debug_new_recording = true;"
 *   5. Place a breakpoint when you want to stop recording and examine
 *     memory leaks, by watching the code_debug_new_allocs vector's contents.
 * With this tool on, all memory allocations and freeings will be recorded.
 * This only applies to operations in the project's code
 * (hence the find-and-replace). This helps in debugging memory leaks in
 * two instances of the same state of execution.
 * For instance, you enter the main menu, start recording, enter a different
 * menu, and return to the main menu. Everything that got allocated since
 * recording should've been freed by now since the program is in the same
 * state it was before (the main menu). With this tool, it is possible to see
 * what addresses were allocated, but not freed.
 * The only information that is given is the memory address (whose content can
 * be inspected if you paused execution with a breakpoint), and the file + line
 * the allocation was made in.
 * Compared to the likes of memcheck, this is faster, and useful in that it
 * reports lines from the project's code exclusively,
 * making it easier to understand.
 * Based on http://wyw.dcweb.cn/leakage.htm
 */
#ifdef CODE_DEBUG_NEW

#include <cstdlib>
#include <map>
#include <new>
#include <string>

using std::map;
using std::size_t;
using std::string;


extern map<void*, string> code_debug_new_allocs;
extern bool code_debug_new_recording;

void* operator new(size_t size, char* file, int line);
void* operator new[](size_t size, char* file, int line);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr, size_t size) noexcept;

#define code_debug_new new(__FILE__, __LINE__)
#define code_debug_delete delete

#endif //ifdef CODE_DEBUG_NEW


//Timestamp for the start of the current benchmark measurement.
extern double code_debug_benchmark_measure_start;
//Sum of the durations of all code benchmarking iterations.
extern double code_debug_benchmark_sum;
//Number of code benchmarking iterations so far.
extern unsigned int code_debug_benchmark_iterations;

void code_debug_benchmark_start_measuring();
double code_debug_benchmark_end_measuring();
double code_debug_benchmark_get_avg_duration();

#endif //ifndef CODE_DEBUG_INCLUDED
