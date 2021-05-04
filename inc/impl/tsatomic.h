/*****************************************************************************************************//**
 *
 *  Module Name:	\file tsatomic.h
 *
 *  Abstract:		\brief Atomic primitives definitions.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Internal: ts_resource_*, ts_spin_*, tstl :: atomic_*
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSATOMIC_H__
#define __TSATOMIC_H__

#include "impl/tsdebug.h"
#include "impl/tssleep.h"

/// Common locker definition
#define ts_lock_define(locker) volatile long locker;

/// Init fast mutex
#define ts_resource_lock_init(status)   { status = TS_FREE_SIGN; }
#define ts_resource_lock_define(status) ts_lock_define (status)

/// Enter to fast mutex
#define ts_resource_lock(status)  \
{ while (TS_FREE_SIGN != tstl :: interlocked_compare_exchange ( (long*) & status, TS_BUSY_SIGN, TS_FREE_SIGN) ) \
  { if (ts_processors_number > 1) \
    { long counter = TS_SPINLOCK_COUNTER << 1; \
      while (TS_FREE_SIGN != status && --counter > 0) { ts_yield_processor (); } \
      if (TS_FREE_SIGN == status) continue; } /* if end */ \
      ts_sleep (TS_SPINLOCK_SLEEP_TIME); } }

/// Leave fast mutex
#define ts_resource_unlock(status) \
{ if (TS_BUSY_SIGN != tstl :: interlocked_compare_exchange ( (long*) & status, TS_FREE_SIGN, TS_BUSY_SIGN) ) { brk (); } }

/// Init spin locker
#define ts_spin_lock_init(spin_lock)   { spin_lock = 0; }
#define ts_spin_lock_define(spin_lock) ts_lock_define (spin_lock)

/// Enter generic spinlocker
#define ts_spin_lock(spin_lock) \
{ while (0 != tstl :: interlocked_compare_exchange ( (long*) & spin_lock, 1, 0) ) \
  { while (0 != spin_lock) { ts_yield_processor (); } } }

/// Leave generic spinlocker
#define ts_spin_unlock(spin_lock) \
{ tstl :: interlocked_exchange ( (long*) & spin_lock, 0); }

/// Inline macroses
#if defined (_MSC_VER)

#  if defined (_WIN64)

#    if !(defined (_NTDDK_) || defined (_WINBASE_) || defined (_WINBASE_H) || defined (_WINNT_) )
#      define InterlockedIncrement   _InterlockedIncrement
#      define InterlockedDecrement   _InterlockedDecrement
#      define InterlockedExchangeAdd _InterlockedExchangeAdd

#      define InterlockedExchangePointer        _InterlockedExchangePointer
#      define InterlockedCompareExchange        _InterlockedCompareExchange
#      define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

extern "C" long  InterlockedIncrement	(long* addend);
extern "C" long  InterlockedDecrement	(long* addend);
extern "C" long  InterlockedExchangeAdd	(long* addend, long value);

extern "C" void* InterlockedExchangePointer	   (void** destination, void* exchange);
extern "C" long  InterlockedCompareExchange	   (long*  destination, long  exchange, long  comperand);
extern "C" void* InterlockedCompareExchangePointer (void** destination, void* exchange, void* comperand);

#      pragma intrinsic(_InterlockedIncrement)
#      pragma intrinsic(_InterlockedDecrement)
#      pragma intrinsic(_InterlockedExchangeAdd)

#      pragma intrinsic(_InterlockedExchangePointer)
#      pragma intrinsic(_InterlockedCompareExchange)
#      pragma intrinsic(_InterlockedCompareExchangePointer)
#    endif

namespace tstl {

static inline long interlocked_exchange (volatile long* destination, long exchange)
{ return InterlockedExchange (destination, exchange); }

static inline void* interlocked_exchange_pointer (void** destination, void* exchange)
{ return InterlockedExchangePointer (destination, exchange); }

static inline long interlocked_compare_exchange (volatile long* destination, long exchange, long comperand)
{ return InterlockedCompareExchange (destination, exchange, comperand); }

static inline void* interlocked_compare_exchange_pointer (void** destination, void* exchange, void* comperand)
{ return InterlockedCompareExchangePointer (destination, exchange, comperand); }

}; ///< end of tstl namespace

#  else ///< 32

#    if !(defined (_NTDDK_) || defined (_WINBASE_) || defined (_WINBASE_H) || defined (_WINNT_) )
#      define InterlockedIncrement   _InterlockedIncrement
#      define InterlockedDecrement   _InterlockedDecrement
#      define InterlockedExchangeAdd _InterlockedExchangeAdd

extern "C" long InterlockedIncrement	(long* addend);
extern "C" long InterlockedDecrement	(long* addend);
extern "C" long InterlockedExchangeAdd	(long* addend, long value);

#      pragma intrinsic(_InterlockedIncrement)
#      pragma intrinsic(_InterlockedDecrement)
#      pragma intrinsic(_InterlockedExchangeAdd)
#    endif

/// Intel(tm) 80486+ byte code

namespace tstl {

static inline long interlocked_compare_exchange (volatile long* destination, long exchange, long comperand)
{
  __asm {
    mov     ecx, destination
    mov     edx, exchange
    mov     eax, comperand

    lock cmpxchg [ecx], edx
   }
}

static inline long interlocked_exchange (volatile long* destination, long exchange)
{
  __asm {
    mov     ecx, destination
    mov     edx, exchange
    mov     eax, [ecx]

next_cmpxchg: 
    lock cmpxchg [ecx], edx
    jnz     short next_cmpxchg
   }
}

static inline void* interlocked_compare_exchange_pointer (void** destination, void* exchange, void* comperand)
{ return (void*) interlocked_compare_exchange ( (long*) destination, (long) exchange, (long) comperand); }

static inline void* interlocked_exchange_pointer (void** destination, void* exchange)
{ return (void*) interlocked_exchange ((long*) destination, (long) exchange); }

}; ///< end of tstl namespace

#  endif ///< _WIN64

#elif defined (__GNUC__)

namespace tstl {

#  include "impl/gasm.h"

static inline void* interlocked_compare_exchange_pointer (void** destination, void* exchange, void* comperand)
{ return (void*) interlocked_compare_exchange ( (long*) destination, (long) exchange, (long) comperand); }

static inline void* interlocked_exchange_pointer (void** destination, void* exchange)
{ return (void*) interlocked_exchange ( (long*) destination, (long) exchange); }

}; ///< end of tstl namespace

#else
#  error "There's place for porting code against your platform."
#endif

namespace tstl {

static inline void atomic_inc (long* addend)
{ ::InterlockedIncrement (addend); }

static inline void atomic_dec (long* addend)
{ ::InterlockedDecrement (addend); }

static inline long atomic_inc_return (long* addend)
{ return ::InterlockedIncrement (addend); }

static inline long atomic_dec_return (long* addend)
{ return ::InterlockedDecrement (addend); }

static inline long atomic_add_return (long* addend, long value)
{ return ::InterlockedExchangeAdd (addend, value); }

static inline long atomic_exchange (long* destination, long exchange)
{ return interlocked_exchange (destination, exchange); }

static inline void* atomic_exchange (void** destination, void* exchange)
{ return interlocked_exchange_pointer (destination, exchange); }

static inline long atomic_compare_exchange (long* destination, long exchange, long comperand)
{ return interlocked_compare_exchange (destination, exchange, comperand); }

static inline void* atomic_compare_exchange (void** destination, void* exchange, void* comperand)
{ return interlocked_compare_exchange_pointer (destination, exchange, comperand); }

}; ///< end of tstl namespace

#endif /* __TSATOMIC_H__ */
