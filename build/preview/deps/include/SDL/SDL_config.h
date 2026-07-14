/* MSVC smoke-test shim: replaces the mingw-configure-generated SDL_config.h
   (which #errors under _MSC_VER). Based on SDL 1.2.14 SDL_config_win32.h. */
#ifndef _SDL_config_h
#define _SDL_config_h

#include "SDL_platform.h"

#define HAVE_STDINT_H 1
#define HAVE_LIBC 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDARG_H 1
#define HAVE_STRING_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MATH_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_MALLOC 1
#define HAVE_CALLOC 1
#define HAVE_REALLOC 1
#define HAVE_FREE 1
#define HAVE_QSORT 1
#define HAVE_ABS 1
#define HAVE_MEMSET 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMCMP 1
#define HAVE_STRLEN 1
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
#define HAVE_STRTOD 1
#define HAVE_ATOI 1
#define HAVE_ATOF 1
#define HAVE_STRCMP 1
#define HAVE_STRNCMP 1
#define HAVE__STRICMP 1
#define HAVE__STRNICMP 1
#define HAVE_SSCANF 1

#define SDL_HAS_64BIT_TYPE 1

#define SDL_AUDIO_DRIVER_DSOUND 1
#define SDL_JOYSTICK_WINMM 1
#define SDL_LOADSO_WIN32 1
#define SDL_THREAD_WIN32 1
#define SDL_TIMER_WIN32 1
#define SDL_VIDEO_DRIVER_WINDIB 1
#define SDL_VIDEO_OPENGL 1

#endif /* _SDL_config_h */
