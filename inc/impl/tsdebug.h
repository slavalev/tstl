/*****************************************************************************************************//**
 *
 *  Module Name:	\file tsdebug.h
 *
 *  Abstract:		\brief Debuging macroses
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 20.10.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSDEBUG_H__
#define __TSDEBUG_H__

/// break point platform depended macros definition
#if (defined (DEBUG) || defined (DBG) ) && !defined (brk)
#  ifndef __GNUC__
#    if !defined (_AMD64_)
#      define __debugbreak() { _asm int 3 }
#    endif
#    if defined (_NTDDK_)
#      ifdef __cplusplus
extern "C" {
#      endif
extern volatile unsigned char brk_enable;
#      ifdef __cplusplus
}
#      endif
#      define brk() { if (brk_enable) { __debugbreak (); } }
#    else
#      define brk() { __debugbreak (); }
#    endif
#  else
#    define brk() { asm ("int $3"); } ///< __GNUC__
#  endif
#elif !defined (brk)
#  define brk() {;}
#endif

#if (TOTALDEBUG)
# define tbrk()	brk()
#else
# define tbrk() {;}
#endif

#endif /* __TSDEBUG_H__ */
