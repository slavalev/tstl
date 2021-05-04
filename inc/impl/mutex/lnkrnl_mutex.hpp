/*****************************************************************************************************//**
 *
 *  Module Name:	\file lnkrnl_mutex.hpp
 *
 *  Abstract:		\brief Linux OS native kernel mutex envelop.
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

#ifndef __LNKRNL_MUTEX_HPP__
#define __LNKRNL_MUTEX_HPP__

#include <asm/semaphore.h>

namespace tstl {

class mutex
{
  struct semaphore locker;

public:

  void init ()
  { init_MUTEX (& locker); }

  mutex () { init (); }

  void lock ()
  { down (& locker); }

  void unlock ()
  { up   (& locker); }
};

}; /* end of tstl namespace */

#endif /* __LNKRNL_MUTEX_HPP__ */
