#pragma once

#if defined(__linux__) && !defined(LINUX)
#define LINUX
#elif defined(__APPLE__) && !defined(OSX)
#define OSX
#endif

#undef C_DLLEXPORT
#undef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#define NOINLINE __declspec(noinline)
#define snprintf _snprintf
#define C_DLLEXPORT	extern "C" DLLEXPORT
#else
#define DLLEXPORT __attribute__((visibility("default")))
#define NOINLINE __attribute__((noinline))
#define WINAPI		/* */
#define C_DLLEXPORT	extern "C" DLLEXPORT
#endif


#ifndef _MSC_VER
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#else
#define likely(x)      (x)
#define unlikely(x)    (x)
#endif

// Our GiveFnptrsToDll, called by engine.
typedef void (WINAPI* GIVE_ENGINE_FUNCTIONS_FN)(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals);
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals);
