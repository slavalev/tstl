/*****************************************************************************************************//**
 *
 *  Module Name:	\file win32_mutex.hpp
 *
 *  Abstract:		\brief Win32 mutex envelop.
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

#ifndef __WIN32_MUTEX_HPP__
#define __WIN32_MUTEX_HPP__

#include <windows.h>

namespace tstl {

class mutex
{
  CRITICAL_SECTION locker;

public:

  void init ()
  { InitializeCriticalSection (& locker); }

   mutex () { init (); }

  ~mutex ()
  { DeleteCriticalSection (& locker); }

  void lock ()
  { EnterCriticalSection  (& locker); }

  void unlock ()
  { LeaveCriticalSection  (& locker); }
};

}; /* end of tstl namespace */

#endif /* __WIN32_MUTEX_HPP__ */
