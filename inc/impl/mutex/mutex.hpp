/*****************************************************************************************************//**
 *
 *  Module Name:	\file mutex.hpp
 *
 *  Abstract:		\brief Main mutexes definition file.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Internal: mutex, fastlock :: mutex, spinlock :: mutex, emptylock :: mutex
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__

/// Platform specific includes
#if defined (_NTDDK_)
#  include "impl/mutex/ntkrnl_mutex.hpp"
#elif defined (WIN32)
#  include "impl/mutex/win32_mutex.hpp"
#elif defined (__GNUC__)
#  if defined (__DJGPP__)
#    include "impl/mutex/djgpp_mutex.hpp"
#  elif defined (__linux__) && (__KERNEL__)
#    include "impl/mutex/lnkrnl_mutex.hpp"
#  elif defined (__FreeBSD__) && (__KERNEL__)
#    include "impl/mutex/bsdkrnl_mutex.hpp"
#  else
#    include "impl/mutex/pthread_mutex.hpp"
#  endif
#else
#  error "Undefied target system!!!"
#endif

#include "impl/tsatomic.h"

namespace tstl {

/// crossplatform mutex. It bases on inerlocked CAS.
namespace fastlock {

class mutex
{
  ts_resource_lock_define (locker);

public:

  void init ()
  { ts_resource_lock_init (locker); }

  mutex () { init (); }

  void lock ()
  { ts_resource_lock   (locker); }

  void unlock ()
  { ts_resource_unlock (locker); }
};

}; /* end of fastlock namespace */

/// Spin lock based mutex
namespace spinlock {

class mutex
{
  ts_spin_lock_define (locker);

public:

  void init ()
  { ts_spin_lock_init (locker); }

  mutex () { init (); }

  void lock ()
  { ts_spin_lock   (locker) }

  void unlock ()
  { ts_spin_unlock (locker) }
};

}; /* end of spinlock namespace */

/// Needs for turn off synchronization where used melocker, relocker, rwlocker
namespace emptylock {

struct mutex
{
  void init () {}

  void lock () {}

  void unlock () {}
};

}; /* end of emptylock namespace */

}; /* end of tstl namespace */

#endif /* __MUTEX_HPP__ */
