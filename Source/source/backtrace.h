/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the backtrace functions.
 */

#ifndef BACKTRACE_INCLUDED
#define BACKTRACE_INCLUDED

#include <string>
#include <vector>

using namespace std;

const size_t BACKTRACE_MAX_FRAMES = 30;
const size_t BACKTRACE_MAX_SYMBOL_LENGTH = 512;

#if defined(__linux__) || (__APPLE__ && __MACH__)
//Linux and Mac OS.

#include <execinfo.h>
vector<string> get_backtrace() {
    vector<string> result;
    void* stack[BACKTRACE_MAX_FRAMES];
    
    size_t n_symbols = backtrace(stack, BACKTRACE_MAX_FRAMES);
    char** symbols = backtrace_symbols(stack, n_symbols);
    for(int s = 0; s < n_symbols; ++s) {
        result.push_back(symbols[s]);
    }
    
    free(symbols);
    
    if(result.empty()) {
        result.push_back("(Could not obtain)");
    }
    return result;
}



#elif defined(_WIN32)
//Windows.

#include <DbgHelp.h>
#include <sstream>
#include <WinBase.h>
vector<string> get_backtrace() {
    vector<string> result;
    void* stack[BACKTRACE_MAX_FRAMES];

    HANDLE process = GetCurrentProcess();
    SYMBOL_INFO* symbol =
        (SYMBOL_INFO*) malloc(
            sizeof(SYMBOL_INFO) +
            (BACKTRACE_MAX_SYMBOL_LENGTH - 1) * sizeof(TCHAR)
        );
    symbol->MaxNameLen = BACKTRACE_MAX_SYMBOL_LENGTH;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    DWORD dummy_displacement;

    IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*) malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    SymInitialize(process, NULL, TRUE);
    size_t n_symbols =
        CaptureStackBackTrace(0, BACKTRACE_MAX_FRAMES, stack, NULL);

    for(size_t s = 0; s < n_symbols; ++s) {
        SymFromAddr(process, (DWORD64) stack[s], NULL, symbol);

        stringstream str;
        if(
            SymGetLineFromAddr64(
                process, (DWORD64) stack[s],
                &dummy_displacement, line
            )
        ) {
            str <<
                symbol->Name << " in " <<
                line->FileName << ":" << line->LineNumber <<
                " [" << symbol->Address << "]";
        } else {
            str << symbol->Name << " [" << symbol->Address << "]";
        }

        result.push_back(str.str());
    }

    free(line);
    free(symbol);

    if(result.empty()) {
        result.push_back("(Could not obtain)");
    }
    return result;
}



#else
//Not supported.

vector<string> get_backtrace() {
    vector<string> v;
    v.push_back("(Not supported)");
    return v;
}

#endif

#endif //ifndef BACKTRACE_INCLUDED
