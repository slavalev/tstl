/*****************************************************************************************************//**
 *
 *  Module Name:	\file djgpp_mutex.hpp
 *
 *  Abstract:		\brief DOS GCC native mutex envelop.
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

#ifndef __DJGPP_MUTEX_HPP__
#define __DJGPP_MUTEX_HPP__

#include <lwp.h>

namespace tstl {

class mutex
{
  unsigned long locker;

public:

  void init ()
  { lwpCreateMutex (& locker); }

  mutex () : locker (0)
  { init (); }

  ~mutex ()
  { lwpDeleteMutex  (& locker); }

  void lock ()
  { lwpLockMutex    (& locker); }

  void unlock ()
  { lwpReleaseMutex (& locker); }
};

}; /* end of tstl namespace */

#endif /* __DJGPP_MUTEX_HPP__ */
