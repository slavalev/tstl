/*****************************************************************************************************//**
 *
 *  Module Name:	\file melocker.hpp
 *
 *  Abstract:		\brief Non reenterable mutual exclusion locker.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 12.11.2008 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: mutex
 *  Internal: melocker
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __MELOCKER_HPP__
#define __MELOCKER_HPP__

#include "tstl.hpp"

#include "impl/mutex/mutex.hpp"

namespace tstl {

/// Universal locker
#if defined (USE_SPINLOCK)
template <class Tlocker = spinlock :: mutex>
#elif defined (USE_FAST_MUTEX)
template <class Tlocker = fastlock :: mutex>
#else
template <class Tlocker = mutex>
#endif

struct melocker : Tlocker
{
  void init ()
  { Tlocker :: init ();   }

  void lock ()
  { Tlocker :: lock ();   }

  void unlock ()
  { Tlocker :: unlock (); }
};

}; /* end of tstl namespace */

#endif /* __MELOCKER_HPP__ */
