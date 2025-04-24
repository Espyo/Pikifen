/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the code debugging tools.
 */

#pragma once

/**
 * @brief Memory allocation and memory leak debug.
 *
 * To activate:
 *   1. Define CODE_DEBUG_NEW.
 *   2. Include this header file in all cpp files.
 *   3. Do a global project replace of "new" to "codeDebugNew",
 *     and "delete" to "codeDebugDelete". Enable case-sensitivity and
 *     "whole word" matching, and don't replace anything in this
 *     header file or the cpp file.
 *   4. At some point in your code, start recording allocations by doing
 *     "codeDebugNewRecording = true;"
 *   5. Place a breakpoint when you want to stop recording and examine
 *     memory leaks, by watching the codeDebugNewAllocs vector's contents.
 * With this tool on, all memory allocations and freeings will be recorded.
 * This only applies to operations in the project's code
 * (hence the find-and-replace). This helps in debugging memory leaks in
 * two instances of the same state of execution.
 * For instance, you enter the title screen, start recording, enter a different
 * menu, and return to the title screen. Everything that got allocated since
 * recording should've been freed by now since the program is in the same
 * state it was before (the title screen). With this tool, it is possible to see
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


extern map<void*, string> codeDebugNewAllocs;
extern bool codeDebugNewRecording;

void* operator new(size_t size, char* file, int line);
void* operator new[](size_t size, char* file, int line);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr, size_t size) noexcept;

#define codeDebugNew new(__FILE__, __LINE__)
#define codeDebugDelete delete

#endif //ifdef CODE_DEBUG_NEW


//Timestamp for the start of the current benchmark measurement.
extern double codeDebugBenchmarkMeasureStart;
//Sum of the durations of all code benchmarking iterations.
extern double codeDebugBenchmarkSum;
//Number of code benchmarking iterations so far.
extern unsigned int codeDebugBenchmarkIterations;

void codeDebugBenchmarkStartMeasuring();
double codeDebugBenchmarkEndMeasuring();
double codeDebugBenchmarkGetAvgDuration();
