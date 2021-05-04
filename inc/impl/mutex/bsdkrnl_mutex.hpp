/*****************************************************************************************************//**
 *
 *  Module Name:	\file bsdkrnl_mutex.hpp
 *
 *  Abstract:		\brief BSD OS native kernel mutex envelop.
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

#ifndef __BSDKRNL_MUTEX_HPP__
#define __BSDKRNL_MUTEX_HPP__

#include <sys/mutex.h>

namespace tstl {

class mutex
{
  struct mtx locker;

public:

  void init ()
  { mtx_init (& locker, "mutex", "tstl", MTX_DEF); }

  mutex () { init (); }

  ~mutex ()
  { mtx_destroy (& locker); }

  void lock ()
  { mtx_lock   (& locker); }

  void unlock ()
  { mtx_unlock (& locker); }
};

}; /* end of tstl namespace */

#endif /* __BSDKRNL_MUTEX_HPP__ */
