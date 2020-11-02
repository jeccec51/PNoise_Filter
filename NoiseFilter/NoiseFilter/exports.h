#ifndef __NEXPORTS
#define __NEXPORTS
#ifndef __unix
#if (defined(NFILTER_EXPORTS))
#define NFILTER_DLL __declspec( dllexport )
#else
#define NFILTER_DLL __declspec( dllimport )
#endif
#else
#define NFILTER_DLL
#endif
#endif 