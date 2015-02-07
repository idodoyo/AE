
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

// USEDLL defined for PAC or driver library users who want to use the DLL
// Must be defined in the project settings for a project which uses the DLL
// version

#if !defined(_DLL_INC)
#define _DLL_INC

#if defined(_USRDLL) || defined(USEDLL)
  #ifdef DLL_EXPORTS
    #define DLL_API __declspec(dllexport)
  #else
    #define DLL_API __declspec(dllimport)
  #endif
#else
  #define DLL_API   // expands to nothing if were not building a DLL
#endif

#endif  // _DLL_INC
