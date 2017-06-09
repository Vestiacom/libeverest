#ifndef EVEREST_INTERNALS_COMMON_HPP_
#define EVEREST_INTERNALS_COMMON_HPP_

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define EVEREST_HELPER_DLL_IMPORT __declspec(dllimport)
  #define EVEREST_HELPER_DLL_EXPORT __declspec(dllexport)
  #define EVEREST_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define EVEREST_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define EVEREST_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define EVEREST_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define EVEREST_HELPER_DLL_IMPORT
    #define EVEREST_HELPER_DLL_EXPORT
    #define EVEREST_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define EVEREST_API and EVEREST_LOCAL.
// EVEREST_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// EVEREST_LOCAL is used for non-api symbols.

#ifdef EVEREST_DLL // defined if EVEREST is compiled as a DLL
  #ifdef EVEREST_DLL_EXPORTS // defined if we are building the EVEREST DLL (instead of using it)
    #define EVEREST_API EVEREST_HELPER_DLL_EXPORT
  #else
    #define EVEREST_API EVEREST_HELPER_DLL_IMPORT
  #endif // EVEREST_DLL_EXPORTS
  #define EVEREST_LOCAL EVEREST_HELPER_DLL_LOCAL
#else // EVEREST_DLL is not defined: this means EVEREST is a static lib.
  #define EVEREST_API
  #define EVEREST_LOCAL
#endif // EVEREST_DLL

#endif // EVEREST_INTERNALS_COMMON_HPP_

