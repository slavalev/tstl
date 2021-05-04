/*****************************************************************************************************//**
 *
 *  Module Name:	\file pthread_mutex.hpp
 *
 *  Abstract:		\brief Posix Theards (pthreads) mutex envelop.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Intenal: mutex
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __PTHREAD_MUTEX_HPP__
#define __PTHREAD_MUTEX_HPP__

#include <pthread.h>

namespace tstl {

class mutex
{
  pthread_mutex_t locker;

public:

  void init ()
  { if (pthread_mutex_init (& locker, 0) ) { brk (); } }

   mutex () { init (); }

  ~mutex ()
  { pthread_mutex_destroy (& locker); }

  void lock ()
  { pthread_mutex_lock    (& locker); }

  void unlock ()
  { pthread_mutex_unlock  (& locker); }
};

}; /* end of tstl namespace */

#endif /* __PTHREAD_MUTEX_HPP__ */
