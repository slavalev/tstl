/*********************************************************************************************************
 *
 *  Module Name:	tslist.hpp
 *
 *  Abstract:		Simple doubly linked lists porting against C++.
 *
 *  Author:		Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	05.08.2003	started
 *
 *  Classes, methods and structures:
 *
 *  TODO:
 *
 *********************************************************************************************************/

#ifndef __TSLIST_HPP__
#define __TSLIST_HPP__

extern "C" {

#define new newx

#ifdef _WIN32
#  define __inline__ __inline
#endif ///< _WIN32

#include "impl/list.h"

typedef struct list_head list_head, *plh;

#undef new
}

#endif /* __TSLIST_HPP__ */
