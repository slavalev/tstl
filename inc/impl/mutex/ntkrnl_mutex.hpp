/*****************************************************************************************************//**
 *
 *  Module Name:	\file ntkrnl_mutex.hpp
 *
 *  Abstract:		\brief NT OS kernel mutexes.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Internal: mutex
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __NTKRNL_MUTEX_HPP__
#define __NTKRNL_MUTEX_HPP__

#if defined (USE_PUSHLOCK)

#include <fltKernel.h>

namespace tstl {

class mutex
{
  EX_PUSH_LOCK locker;

public:

  void init ()
  { FltInitializePushLock (& locker); }

   mutex () { init (); }

  ~mutex ()
  { FltDeletePushLock (& locker); }

  void lock ()
  { FltAcquirePushLockExclusive (& locker); }

  void unlock ()
  { FltReleasePushLock (& locker); }
};

}; /* end of tstl namespace */

#else

#include <ntddk.h>

namespace tstl {

class mutex
{
  KMUTEX locker;

public:

  void init ()
  { KeInitializeMutex (& locker, 0); }

   mutex () { init (); }

  ~mutex () {}

  void lock ()
  { KeWaitForMutexObject (& locker, Executive, KernelMode, TRUE, 0); }

  void unlock ()
  { KeReleaseMutex (& locker, FALSE); }
};

}; /* end of tstl namespace */

#endif

#endif /* __NTKRNL_MUTEX_HPP__ */
