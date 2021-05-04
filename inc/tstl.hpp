/*****************************************************************************************************//**
 *
 *  Module Name:	\file tstl.hpp
 *
 *  Abstract:		\brief Thread Safe Templates library main include file.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com). 
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: new, delete
 *  Internal: allocator
 *
 *  TODO: 		\todo
 *
 *********************************************************************************************************/

#ifndef __TSTL_HPP__
#define __TSTL_HPP__

/// TSTL version defines
#define TSTL_VERSION    1
#define TSTL_PATCHLEVEL 3
#define TSTL_SUBLEVEL   5

/// Common defines
#define TS_SPINLOCK_COUNTER 500
#define TS_MINUS_MEDIAN	 0x1000
#define TS_MINUS_NULL	 ( (long)(0 - TS_MINUS_MEDIAN) )

/// Make signature from chars
#define TS_LONG_SIGNATURE(A, B, C, D) ( ( (unsigned long) (D) << 24) \
				      | ( (unsigned long) (C) << 16) \
				      | ( (unsigned long) (B) << 8)  \
				      |   (unsigned long) (A) )

/// Objects Status Definition Signatures (OSDS)
#define TS_FREE_SIGN TS_LONG_SIGNATURE ('F','R','E','E') ///< ready for using
#define TS_BUSY_SIGN TS_LONG_SIGNATURE ('B','U','S','Y') ///< busy, in change time
#define TS_LIVE_SIGN TS_LONG_SIGNATURE ('L','I','V','E') ///< has usefull payload
#define TS_KILL_SIGN TS_LONG_SIGNATURE ('K','I','L','L') ///< killing time status
#define TS_DEAD_SIGN TS_LONG_SIGNATURE ('D','E','A','D') ///< dead but partialy undestroyed
#define TS_ERAS_SIGN TS_LONG_SIGNATURE ('E','R','A','S') ///< fully erased

#include "impl/tsatomic.h"

namespace tstl {

struct allocator
{
  void*  allocate (size_t size) { return new char [size]; }
  void deallocate (void* p)     { delete [] p; }
};

}; /* end of tstl namespace */

static inline void* _cdecl operator new (size_t, void *_P, const tstl :: allocator& allocator)
{ tstl :: allocator unused_allocator = allocator; return (_P); }

/// Include all library templates if was not specified before one of them
#if !(defined (__TSMAP_HPP__)	\
   || defined (__TSCACHE_HPP__)	\
   || defined (__TIMERCACHE_HPP__)\
   || defined (__LIMITCACHE_HPP__)\
   || defined (__ALLOCCACHE_HPP__)\
   || defined (__TSQUEUE_HPP__)	\
   || defined (__TSPREQUEUE_HPP__)\
   || defined (__TSPIPE_HPP__)	\
   || defined (__RWLOCKER_HPP__)\
   || defined (__RELOCKER_HPP__)\
   || defined (__MELOCKER_HPP__) )

#  include "tspipe.hpp"		///< includes relocker.hpp
#  include "tscache.hpp"	///< includes limitcache.hpp -> tsmap.hpp, timercache.hpp
#  include "tsqueue.hpp"	///< includes relocker.hpp, alloccache.hpp
#  include "rwlocker.hpp"	///< includes relocker.hpp -> melocker.hpp
#  include "tsprequeue.hpp"	///< includes tsmap.hpp, tsqueue.hpp

#endif ///< Include all library templates if was not specified before one of them

#endif /* __TSTL_HPP__ */
