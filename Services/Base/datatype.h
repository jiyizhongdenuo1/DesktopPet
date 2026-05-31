#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef SHORT
typedef short SHORT;
#endif

#ifndef VOID
#define VOID                 void
#endif

#ifndef CHAR
typedef char                CHAR;
#endif

#ifndef UCHAR
typedef unsigned char       UCHAR;
#endif

#ifndef INT
typedef int                 INT;
#endif

#ifndef INT8U
typedef unsigned char	INT8U;                    /* Unsigned  8 bit quantity */
#endif

#ifndef UINT8
typedef unsigned char	UINT8;                    /* Unsigned  8 bit quantity */
#endif

#ifndef INT8
typedef signed   char	INT8;                     /* Signed    8 bit quantity */
#endif

#ifndef INT8S
typedef signed   char 	INT8S;                    /* Signed    8 bit quantity */
#endif

#ifndef INT16U
typedef unsigned short	INT16U;                   /* Unsigned 16 bit quantity */
#endif

#ifndef UINT16
typedef unsigned short	UINT16;                   /* Unsigned 16 bit quantity */
#endif

#ifndef INT16
typedef signed   short	INT16;                    /* Signed   16 bit quantity */
#endif

#ifndef INT16S
typedef signed   short	INT16S;                   /* Signed   16 bit quantity */
#endif

#ifndef INT32U
typedef unsigned int INT32U;                   /* Unsigned 32 bit quantity */
#endif

#ifndef UINT32
typedef unsigned int	UINT32;                   /* Unsigned 32 bit quantity */
#endif

//#if !defined(WIN32)
#ifndef INT32
typedef signed   int	INT32;                    /* Signed   32 bit quantity */
#endif
//#endif

#ifndef INT32S
typedef signed   int	INT32S;                   /* Signed   32 bit quantity */
#endif


typedef unsigned long long  INT64U;               /* Unsigned   64 bit quantity */
typedef unsigned long long  UINT64;               /* Unsigned   64 bit quantity */
typedef signed   long long  INT64;               /* Signed   64 bit quantity */

#ifndef TRUE
#define TRUE		1
#endif

#ifndef FALSE
#define FALSE 		0
#endif




#ifndef INT64S
//#if !defined(WIN32)
typedef signed long long     INT64S;              /* Signed   64 bit quantity */
//#else
////typedef __int64  INT64S;                          /* Signed   64 bit quantity */
//#endif
#endif

#ifndef FLOAT32
typedef float				 FLOAT32;             /* Signed   float quantity */
#endif

#ifndef FLOAT64
typedef double 			     FLOAT64;             /* Signed   double quantity */
#endif

//#ifndef bool
//typedef unsigned long 	bool;					/* Signed   bool quantity */
//#endif

#ifndef STATUS
typedef  int 	       STATUS;					    /* Signed   bool quantity */
#endif

//#ifdef __linux__
//#ifndef LRESULT
//typedef int           LRESULT;
//#endif
//#endif


#ifndef BOOL
//typedef unsigned int 	BOOL;                       /* Signed   bool quantity */
#define BOOL           int
#endif

#if !defined(WIN32)
    /* ISO C Standard:  7.16  Boolean type and values  <stdbool.h> */
    #if (defined __digital__ && defined __unix__)
    /* old system without (proper) C99 support */
    #define bool unsigned char
    #else
    /* modern system, so use it */
    #include <stdbool.h>
    #endif
#else
#endif

#ifndef RELEASEIF
#define RELEASEIF(pointer)      do \
                                { \
                                    if (NULL != (pointer)) \
                                    { \
                                        delete (pointer); \
                                        (pointer) = NULL; \
                                    } \
                                } while(0)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif

#ifndef FLOAT
typedef float               FLOAT;
#endif

//#ifndef _MINIGUI_GUI_

//#ifdef __linux__
//#ifndef HWND
//typedef void* HWND;
//#endif
//#endif

#ifndef WORD
typedef unsigned short  WORD;
#endif


#ifdef _WIN32
//windows x86 or x64
#ifdef _WIN64
    typedef long long PARAM;
#else
    typedef int PARAM;
#endif
#else //linux
#ifdef __LP64__
    typedef long int PARAM;
#else
    typedef int PARAM;
#endif  //__LP64__
#endif  //_WIN32


//#ifndef WPARAM
//typedef unsigned int    WPARAM;
//#endif

//#ifdef __linux__
//#ifndef LPARAM
//typedef unsigned long   LPARAM;
//#endif
//#endif

//#ifndef WDATA
//typedef unsigned int    WDATA;
//#endif

//#ifndef LDATA
//typedef unsigned int    LDATA;
//#endif

//#ifndef DWORD
//typedef unsigned long    DWORD;
//#endif



#ifndef UINT
typedef unsigned int    UINT;
#endif

#ifndef Uint8
typedef unsigned char    Uint8;
#endif

#ifndef gal_uint8
typedef Uint8    gal_uint8;
#endif

#ifndef BYTE
typedef unsigned char    BYTE;
#endif

// #endif


//#ifndef _MINIGUI_GUI_

//#ifdef __linux__
//typedef struct _RECT
//{
//    /**
//     * The x coordinate of the upper-left corner of the rectangle.
//     */
//    int left;
//    /**
//     * The y coordinate of the upper-left corner of the rectangle.
//     */
//    int top;
//    /**
//     * The x coordinate of the lower-right corner of the rectangle.
//     */
//    int right;
//    /**
//     * The y coordinate of the lower-right corner of the rectangle.
//     */
//    int bottom;
//} RECT;
//#endif





/**
 * \var typedef RECT* PRECT
 * \brief Data type of the pointer to a RECT.
 *
 * \sa RECT
 */
// #ifdef __linux__
// typedef RECT* PRECT;
// #endif

#ifndef PATH_MAX
    #define PATH_MAX    256
#endif

#ifndef MAX_PATH
    #define MAX_PATH        PATH_MAX
#endif

#ifdef __linux__
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif


//typedef enum _color_index
//{
//    COLOR_BLACK,        // black         --0
//    COLOR_DARKBLUE,     // dark blue     --1
//    COLOR_DARKGREEN,    // dark green    --2
//    COLOR_DARKCYAN,     // dark cyan     --3
//    COLOR_DARKRED,      // dark red      --4
//    COLOR_DARKMAGENTA,  // dark magenta  --5
//    COLOR_DARKYELLOW,   // dark yellow   --6
//    COLOR_DARKGRAY,     // dark gray     --7
//    COLOR_LIGHTGRAY,    // light gray    --8
//    COLOR_BLUE,         // blue          --9
//    COLOR_GREEN,        // green         --10
//    COLOR_CYAN,         // cyan          --11
//    COLOR_RED,
//    COLOR_MAGENTA,
//    COLOR_YELLOW,
//    COLOR_WHITE,
//    COLOR_BROWN,
//    COLOR_ORANGE,
//    COLOR_BLUE_LIGHT,
//    COLOR_GRAY110,

//    COLOR_MAX

//}COLOR_INDEX;


/**
 * RGB triple structure.
 * \sa PRGB, GAL_Color
 */
typedef struct _RGB
{
    /**
     * The red component of a RGB triple.
     */
    BYTE r;
    /**
     * The green component of a RGB triple.
     */
    BYTE g;
    /**
     * The blue component of a RGB triple.
     */
    BYTE b;
    /**
     * Reserved for alignment, maybe used for the alpha component of a RGB triple.
     */
    BYTE a;
} RGB;



// #endif  //_MINIGUI_GUI_


/**
 * \def NULL
 * \brief A value indicates null pointer.
 */
#ifndef NULL
#   ifdef __cplusplus
#       define NULL            (0)
#   else
#       define NULL            ((void *)0)
#   endif
#endif


typedef INT32U	GAL_PIXEL;

//#ifndef __UCOSII__
//#define __UCOSII__ 0
//#endif
#ifdef __cplusplus
}
#endif/*__cplusplus*/
