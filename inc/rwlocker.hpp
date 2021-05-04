/*****************************************************************************************************//**
 *
 *  Module Name:	\file rwlocker.hpp
 *
 *  Abstract:		\brief A shared locker allows a thread to lock shared data either for shared
 *                         read access or exclusive write access.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *			\date 12.11.2008 reimplemented
 *
 *  Classes, methods and structures: \details
 *
 *  External: melocker (relocker), ts_sleep
 *  Internal: rwlocker, emptylock :: rwlocker
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __RWLOCKER_HPP__
#define __RWLOCKER_HPP__

#include "impl/relocker.hpp"

namespace tstl {

/// Writer multiply reader guard
template <class Tlocker = melocker<> > ///< 'relocker' is posible locking policy

class rwlocker
{
  long readers, writers;
  Tlocker resource_locker;
  Tlocker readers_locker;

public:
  rwlocker () : readers (0), writers (0) {}

  ~rwlocker () { if (readers) brk (); }

  /// Call this to gain shared read access
  void read_lock ()
  {
    while (writers)
      ts_sleep (TS_SPINLOCK_SLEEP_TIME); ///< Lock any readers if write operation detected

    readers_locker.lock ();

    if (1 == atomic_inc_return (& readers))
      resource_locker.lock ();

    readers_locker.unlock ();
  }

  /// Call this when done accessing the resource
  void read_unlock ()
  {
    readers_locker.lock ();

    if (!atomic_dec_return (& readers))
      resource_locker.unlock ();

    readers_locker.unlock ();
  }

  /// Call this to gain exclusive write access
  void write_lock ()
  {
    atomic_inc (& writers);
    resource_locker.lock ();
  }

  /// Call this when done accessing the resource
  void write_unlock ()
  {
    resource_locker.unlock ();
    atomic_dec (& writers);
  }
};

/// Needs for turn off synchronization where used rwlocker
namespace emptylock {

template <class Tlocker = mutex<> > ///< emptylock :: mutex

struct rwlocker
{
  void read_lock ()   {}

  void read_unlock () {}

  void write_lock ()  {}

  void write_unlock () {}
};

}; /* end of emptylock namespace */

}; /* end of tstl namespace */

#endif /* __RWLOCKER_HPP__ */
