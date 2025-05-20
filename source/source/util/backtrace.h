/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the backtrace functions.
 */

#pragma once

#include <string>
#include <vector>


using std::size_t;
using std::string;
using std::vector;


namespace BACKTRACE {
const size_t MAX_FRAMES = 30;
const size_t MAX_SYMBOL_LENGTH = 512;
}


#if defined(__linux__) || (__APPLE__)
//Linux and Mac OS.

#include <execinfo.h>
#include <cxxabi.h>


/**
 * @brief Demangles a mangled debugging symbol.
 *
 * @param symbol The symbol to demangle.
 * @return The demangled symbol.
 */
string demangeSymbol(const string& symbol) {
    //Special thanks: https://oroboro.com/stack-trace-on-crash/
    size_t moduleSize = 0;
    size_t nameSize = 0;
    size_t offsetStart = string::npos;
    size_t offsetSize = 0;
    string ret;
    
#ifdef __APPLE__ //Mac OS.
    size_t nameStart = symbol.find(" _");
    if(nameStart != string::npos) {
        moduleSize = nameStart;
        nameStart = nameStart + 1;
        size_t spacePos = symbol.find(" ", nameStart + 1);
        if(spacePos != string::npos) {
            nameSize = spacePos - nameStart;
            offsetStart = nameStart + nameSize + 1;
            if(offsetStart != string::npos) {
                offsetSize = symbol.size();
            }
        }
    }
#else //Linux.
    size_t nameStart = symbol.find("(");
    if(nameStart != string::npos) {
        moduleSize = nameStart;
        nameStart = nameStart + 1;
        size_t plusPos = symbol.find("+", nameStart + 1);
        if(plusPos != string::npos) {
            nameSize = plusPos - nameStart;
            offsetStart = nameStart + nameSize + 1;
            if(offsetStart != string::npos) {
                offsetSize = symbol.find(")", offsetStart) - offsetStart;
            }
        }
    }
#endif
    
    if(nameStart != string::npos && offsetStart != string::npos) {
        string moduleStr = symbol.substr(0, moduleSize);
        string mangledName = symbol.substr(nameStart, nameSize);
        string offsetStr = symbol.substr(offsetStart, offsetSize);
        
        int demangleStatus;
        char* demangledName =
            abi::__cxa_demangle(
                mangledName.c_str(), nullptr, nullptr, &demangleStatus
            );
            
        if(demangleStatus == 0) {
            ret =
                moduleStr + " " + demangledName + " + " + offsetStr;
        } else {
            ret =
                moduleStr + " " + mangledName + " + " + offsetStr;
        }
    } else {
        ret = symbol;
    }
    return ret;
}


/**
 * @brief Returns the backtrace of the current stack.
 *
 * @return The backtrace.
 */
vector<string> getBacktrace() {
    vector<string> result;
    void* stack[BACKTRACE::MAX_FRAMES];
    
    size_t nSymbols = backtrace(stack, BACKTRACE::MAX_FRAMES);
    char** symbols = backtrace_symbols(stack, nSymbols);
    
    for(size_t s = 0; s < nSymbols; s++) {
        result.push_back(demangeSymbol(symbols[s]));
    }
    
    free(symbols);
    
    if(result.empty()) {
        result.push_back("(Could not obtain)");
    }
    return result;
}



#elif defined(_WIN32)
//Windows.

#include <Windows.h>
#include <DbgHelp.h>
#include <sstream>
#include <WinBase.h>

/**
 * @brief Returns the backtrace of the current stack.
 *
 * @return The backtrace.
 */
vector<string> getBacktrace() {
    vector<string> result;
    void* stack[BACKTRACE::MAX_FRAMES];
    
    HANDLE process = GetCurrentProcess();
    SYMBOL_INFO* symbol =
        (SYMBOL_INFO*) malloc(
            sizeof(SYMBOL_INFO) +
            (BACKTRACE::MAX_SYMBOL_LENGTH - 1) * sizeof(TCHAR)
        );
    symbol->MaxNameLen = BACKTRACE::MAX_SYMBOL_LENGTH;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    DWORD dummyDisplacement;
    
    IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*) malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    
    SymInitialize(process, nullptr, TRUE);
    size_t nSymbols =
        CaptureStackBackTrace(0, BACKTRACE::MAX_FRAMES, stack, nullptr);
        
    for(size_t s = 0; s < nSymbols; s++) {
        SymFromAddr(process, (DWORD64) stack[s], nullptr, symbol);
        
        std::stringstream str;
        if(
            SymGetLineFromAddr64(
                process, (DWORD64) stack[s],
                &dummyDisplacement, line
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

/**
 * @brief Returns the backtrace of the current stack.
 *
 * @return The backtrace.
 */
vector<string> getBacktrace() {
    vector<string> v;
    v.push_back("(Not supported)");
    return v;
}


#endif
