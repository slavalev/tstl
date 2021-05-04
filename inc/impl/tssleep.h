/*****************************************************************************************************//**
 *
 *  Module Name:	\file tssleep.h
 *
 *  Abstract:		\brief Sleeping routines definitions for different platforms.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Internal: ts_sleep, ts_sleep_intr, ts_yield_processor, ts_processors_number,
 *            TS_ONE_SECOND, TS_SPINLOCK_SLEEP_TIME
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/
#ifndef __TSSLEEP_H__
#define __TSSLEEP_H__

/// definition of ts_yield_processor
#if defined (_MSC_VER)

#  if (_MSC_VER >= 1310)
extern "C" void _mm_pause ();
#    pragma intrinsic(_mm_pause)
#    define ts_yield_processor() { _mm_pause (); }

#  else

#    if !defined (_WIN64)
#      define ts_yield_processor() { __asm { rep nop } }
#    else
#      error "Unsupported target system!!!"
#    endif

#  endif

#elif defined(__GNUC__)

#  define ts_yield_processor() { __asm__ __volatile__( "rep; nop" : : : "memory" ); }

#endif ///< end of define _MSC_VER >= 1310

/// definitions of ts_sleep/ts_sleep_intr, ts_processors_number, TS_ONE_SECOND, TS_SPINLOCK_SLEEP_TIME
#if defined (_NTDDK_)

#  define TS_ONE_SECOND		10000000	///< 10000000 tick per second
#  define TS_HZ(x)			(TS_ONE_SECOND / (x))
#  define TS_RELATIVE_HZ_TIMEOUT(x)	(-(TS_HZ(x)))
#  define TS_SPINLOCK_SLEEP_TIME (2 * TS_RELATIVE_HZ_TIMEOUT(TS_SPINLOCK_COUNTER))

#  define ts_sleep(time)\
{ LARGE_INTEGER timeout;	\
  timeout.QuadPart = time;	\
  KeDelayExecutionThread (KernelMode, TRUE, &timeout); }

#  define ts_sleep_intr(time, status)\
{ LARGE_INTEGER timeout;	\
  timeout.QuadPart = time;	\
  status = STATUS_SUCCESS == KeDelayExecutionThread (KernelMode, TRUE, & timeout); }

#  define ts_processors_number ( (int) KeNumberProcessors )

#elif defined (WIN32)

#  define TS_ONE_SECOND	1000 ///< 1000 tick per second
#  define TS_SPINLOCK_SLEEP_TIME (TS_ONE_SECOND / TS_SPINLOCK_COUNTER)

#  define ts_sleep(time) { Sleep (time); }
#  define ts_sleep_intr(time, status) { SleepEx (time, TRUE); }

#  define ts_processors_number 2

#elif defined (__GNUC__)

#  if defined (__DJGPP__)

#    define TS_ONE_SECOND	1000 ///< 1000 tick per second
#    define TS_SPINLOCK_SLEEP_TIME (TS_ONE_SECOND / TS_SPINLOCK_COUNTER)

#    define ts_sleep(time) { msleep (time); }
#    define ts_sleep_intr(time, status) ts_sleep (time)

#    define ts_processors_number (sysconf (_SC_NPROCESSORS_CONF) )

#  elif defined (__linux__) && (__KERNEL__)

#    define TS_ONE_SECOND	HZ ///< 1 Hertz
#    define TS_SPINLOCK_SLEEP_TIME (TS_ONE_SECOND / TS_SPINLOCK_COUNTER)

#    define ts_sleep(time) { interruptible_sleep_on_timeout (&wait, time); }
#    define ts_sleep_intr(time, status) { status = 0 == interruptible_sleep_on_timeout (&wait, time); }

#    define ts_processors_number NR_CPUS

#  elif defined (__FreeBSD__) && (__KERNEL__)

#    define TS_ONE_SECOND	hz ///< 1 Hertz
#    define TS_SPINLOCK_SLEEP_TIME (TS_ONE_SECOND / TS_SPINLOCK_COUNTER)

#    define ts_sleep(time) { msleep (0, &timer_mtx, 0, "TSTL", time); }
#    define ts_sleep_intr(time, status) ts_sleep (time)

#    define ts_processors_number NCPU

#  else

#    define TS_ONE_SECOND	1000 ///< 1000 tick per second
#    define TS_SPINLOCK_SLEEP_TIME (TS_ONE_SECOND / TS_SPINLOCK_COUNTER)

#    define ts_sleep(time) { msleep (time); }
#    define ts_sleep_intr(time, status) ts_sleep (time)

#    define ts_processors_number (sysconf (_SC_NPROCESSORS_CONF) )

#  endif

#else
#  error "Undefied target system!!!"
#endif

#endif /* __TSSLEEP_H__ */
