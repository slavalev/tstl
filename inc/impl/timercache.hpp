/*****************************************************************************************************//**
 *
 *  Module Name:	\file timercache.hpp
 *
 *  Abstract:		\brief Cache manager with cleaning policy by timer.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External:	hash_key, ts_sleep, set_timeout, check_timeout, allocator
 *  Internal:	timer_cache
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TIMERCACHE_HPP__
#define __TIMERCACHE_HPP__

#include "tstl.hpp"

#include "impl/tshash.hpp"

namespace tstl {

#define SET_AT_TRY_COUNTER 1

#if defined (__GNUC__)
typedef unsigned long long ulonglong;
#elif defined (_MSC_VER)
typedef unsigned __int64   ulonglong;
#endif

/// Object status cyclo graph

/** +---------------------LOOPBACK----------------------+
  * +-> FREE -> BUSY -> LIVE -> KILL +--------+-> ERAS -+
  *                                  +-> DEAD +        */
template <class Tkey, class Tvalue, class Thash = size_t, class Tallocator = allocator>

class timer_cache
{
  typedef struct timer_cache_elem
  {
    /// redanted service information
    long status; ///< "FREE" || "BUSY" || "LIVE" || "KILL" || "DEAD" || "ERAS"
    long ref;
    ulonglong lastus;

    /// usefull payload
    Tvalue* pval;
    Thash hash;
    Tkey  key;   ///< for example key == char*
  } tce, *ptce;

  ptce storage, top_storage;
  unsigned short* buble_booster;
  ulonglong cache_timeout;
  long use_counter, max_elem;

  Tallocator allocator;

  hash_key<Tkey, Thash> hk;

  /// Private cache array methods

  /// Lock reference on element
  long lock (ptce& pelem)
  { return atomic_inc_return (& pelem->ref); }

  /// Free reference on element
  long release (ptce& pelem)
  { return atomic_dec_return (& pelem->ref); }

  /// Synoname of Release
  long unlock (ptce& pelem)
  { return release (pelem); }

  /// Lock reference counter removed element
  long lock_remove (ptce& pelem)
  { return (atomic_add_return (& pelem->ref, TS_MINUS_NULL) + TS_MINUS_NULL); }

  /// Free reference counter removed element
  void release_remove (ptce& pelem)
  { atomic_add_return (& pelem->ref, TS_MINUS_MEDIAN); }

  /// Synonym of release_remove
  void unlock_remove (ptce& pelem)
  { release_remove (pelem); }

  /// Change status of cache element from XXX -> (to) YYY
  long change_status (ptce& pelem, long new_status, long previos_status)
  { return atomic_compare_exchange (& pelem->status, new_status, previos_status); }

  /// Set cache element timer
  void set_timeout (ptce& pelem) const
  { ::set_timeout ((unsigned long*) & pelem->lastus, (unsigned long*) & cache_timeout); }

  /// Check cache element timer
  bool check_timeout (ptce& pelem) const
  { return ::check_timeout ((unsigned long*) & pelem->lastus); }

  /// Buble up
  bool buble_up (const long buble_pos);

  /// CleanUp element
  bool remove_dead (ptce pelem);

  /// ERAS -> FREE. Enable only in ERASE status. If successfull than setup FREE status
  bool erase (ptce& pelem);

  /// DEAD -> ERAS -> FREE. Return true if erase dead element
  bool erase_dead (ptce& pelem);

  /// KILL -> ERAS -> FREE. Return true if erase element
  bool erase_killed (ptce& pelem);

  /// LIVE -> KILL (-> ERAS -> FREE) || LIVE -> KILL -> DEAD
  /** Set status FREE or DEAD, begin with LIVE status going over KILL and ERAS
    * Return false if pelem biger of top_storage */
  bool remove (ptce pelem);

  /// Search array element by key & lock it
  bool search_by_key  (long& pos, ptce& pelem, Tkey key);

  /// Search array element by hash & lock it
  bool search_by_hash (long& pos, ptce& pelem, Thash hash);

  /// Doesn't thread safe method, it called from destructor
  void remove_all_unsafe ();

public:
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  timer_cache (const ulonglong timeout, const long num_elem = 32);

  ~timer_cache ();

  /// Doesn't thread safe method
  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about cache using
  long get_stat () const
  { return use_counter; }

  /// Get hash by key
  Thash hash (Tkey key) const
  { return hk.hash (key); }

  /// Insert element in cache & if successfull than lock element
  bool set_at (long& pos, Tkey key, Thash hash, const Tvalue* pvalue);

  /// Insert element in cache & if successfull than lock element
  bool set_at (long& pos, Tkey key, const Tvalue* pvalue)
  { return set_at (pos, key, hk.hash (key), pvalue); }

  /// Look for element in cache by key & if successfull search than lock element
  bool lookup_by_key (long& pos, Tkey key, Tvalue*& pvalue);

  /// Look for element in cache by keys hash & if successfull search than lock element
  bool lookup_by_key_hash (long& pos, Tkey key, Tvalue*& pvalue);

  /// Look for element in cache by hash & if successfull search than lock element
  bool lookup_by_hash (long& pos, Thash hash, Tvalue*& pvalue);

  /// Look for element in cache by pointer on pval
  Tvalue* lookup (long& pos)
  {
    if (!storage || pos >= max_elem) { brk (); return 0; }
    return storage [pos].pval;
  }

  /// Free reference on element
  long release (long& pos)
  {
    if (!storage || pos >= max_elem) { brk (); return false; }
    ptce pelem = & storage [pos];
    return release (pelem);
  }

  /// Synonym of release
  long unlock (long& pos)
  { return release (pos); }

  /// Remove element from cache & unlock element if successfull
  bool remove (long& pos)
  {
    if (!storage || pos >= max_elem) { brk (); return false; }
    return remove (& storage [pos]);
  }

  /// Remove element from cache on cleanup
  bool remove_dead (long& pos)
  {
    if (!storage || pos >= max_elem) { brk (); return false; }
    return remove_dead (& storage [pos]);
  }

  /// Remove element from cache by key
  bool remove_by_key (Tkey key);

  /// Remove element from cache by keys hash
  bool remove_by_key_hash (Tkey key);

  /// Remove element from cache by hash
  bool remove_by_hash (Thash hash);

  void remove_all ();

  /// Next cache enumerating. If it returned fail than last element was unlocked
  bool start (long& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Begin cache enumerating. If it returned success than element was locked
  bool next  (long& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
  {
    ptce pelem = & storage [pos++];
    unlock (pelem);
    return start (pos, key, hash, pvalue);
  }

  /// Next cache enumerating. If it returned fail than last element was unlocked
  bool start (long& pos, Thash hash, Tvalue*& pvalue);

  /// Begin cache enumerating. If it returned success than element was locked
  bool next  (long& pos, Thash hash, Tvalue*& pvalue)
  {
    ptce pelem = & storage [pos++];
    unlock (pelem);
    return start (pos, hash, pvalue);
  }
};

/// Buble up
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: buble_up (const long buble_pos)
{
  if (!buble_pos) return false; ///< topmost element

  if (buble_pos >= max_elem)
  { brk (); return false; } ///< out of array

  long* prev = (long*)(& buble_booster [buble_pos - 1] ); ///< exchange with previous element
  long comperand = *prev;

  if (comperand != atomic_compare_exchange (prev, (comperand << 16) | (comperand >> 16), comperand) )
  { tbrk (); return false; }

  return true;
}

/// CleanUp element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove_dead (ptce pelem)
{
  if (!pelem->pval) ///< BUSY && ERAS may be
  {
    pelem->ref    = 0;
    pelem->lastus = 0;
    pelem->hash   = 0;
    pelem->key    = 0;
    pelem->status = TS_FREE_SIGN;
    return false;
  }

  if (pelem->pval)
  {
    pelem->pval-> ~Tvalue ();

    allocator.deallocate (pelem->pval), pelem->pval = 0;
  }

  pelem->ref    = 0;
  pelem->lastus = 0;
  pelem->hash   = 0;
  pelem->key    = 0;
  pelem->status = TS_FREE_SIGN;

  atomic_dec (& use_counter);
  return true;
}

/// ERAS -> FREE. Enable only in ERASE status. If successfull than setup FREE status
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: erase (ptce& pelem)
{
  Tvalue *locp = pelem->pval;
  Tvalue *prev = (Tvalue*) atomic_compare_exchange ( (void**) & pelem->pval, 0, locp);

  if (locp
   && locp == prev)
  {
    pelem->hash = 0;
    pelem->key  = 0;

    atomic_dec (& use_counter);

    if (locp)
    {
      locp-> ~Tvalue ();

      allocator.deallocate (locp), locp = 0;
    }

    /// Set FREE status
    change_status (pelem, TS_FREE_SIGN, TS_ERAS_SIGN);
    return true;
  }
  else
  {
    /// Cache corupted, very bad break point !!!
    brk ();
    /// status don't touch
    return false;
  }
}

/// DEAD -> ERAS -> FREE. Return true if erase dead element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: erase_dead (ptce& pelem)
{
  /// Change status on ERASE
  long status = change_status (pelem, TS_ERAS_SIGN, TS_DEAD_SIGN);

  if (TS_DEAD_SIGN == status)
  {
    /// Setup ERASE status successfull
    if (pelem->ref == TS_MINUS_NULL)
    {
      /// Reference counter locked successfull
      /** Begin termination dead element of cache */
      return erase (pelem);
    }
    else
    {
      /// Element doesn't ready for removing
      /** Try to restore status */
      change_status (pelem, TS_DEAD_SIGN, TS_ERAS_SIGN);
      return false;
    }
  } ///< end TS_ERAS_SIGN
  else
  {
    /// ERASE status doesn't setuped
    /** Already doesn't dead %-)) may be already erased %-)) */
    return false;
  }
}

/// KILL -> ERAS -> FREE. Return true if erase element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: erase_killed (ptce& pelem)
{
  /// Change status on ERASE
  long status = change_status (pelem, TS_ERAS_SIGN, TS_KILL_SIGN);

  if (TS_KILL_SIGN == status)
  {
    /// Setup ERASE status successfull
    return erase (pelem);
  }
  else
  {
    /// ERASE status doesn't setuped
    /** Already doesn't in killing mode %-)) may be already erased %-)) */
    brk (); ///< lock counter succefully locked, but nobody do it... :-((
    return false;
  }
}

/// LIVE -> KILL (-> ERAS -> FREE) || LIVE -> KILL -> DEAD
/** Set status FREE or DEAD, begin with LIVE status going over KILL and ERAS
  * Return false if pelem biger of top_storage */
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove (ptce pelem)
{
  if (pelem >= top_storage) { brk (); return false; }

  /// If LIVE status passed, than entry to killing status
  long status = change_status (pelem, TS_KILL_SIGN, TS_LIVE_SIGN);

  /// Release element
  release (pelem); ///< == 0

  /// Cache element status control
  if (TS_LIVE_SIGN != status)
  {
    if (TS_KILL_SIGN == status)
    {
      /// Already in killing mode
      tbrk ();
    }
    else
    if (TS_DEAD_SIGN == status)
    {
      /// Element marked as dead
      brk ();

      if (erase_dead (pelem))
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
	return true;
      }
    } ///< end TS_DEAD_SIGN
    else
    if (TS_ERAS_SIGN == status)
    {
      /// Already in erasing mode
      brk ();
    }
    else
    if (TS_BUSY_SIGN == status)
    {
      /// strange mode try to remove... :-((
      brk ();
    }
    else
    if (TS_FREE_SIGN == status)
    {
      /// Final cleanup %-))
      tbrk ();
    }
    else
    {
      /// Element status corrupted, very bad break point !!!
      brk ();
    }

    return false;
  } ///< end TS_LIVE_SIGN != status

  /// set KILL status successfull

  /// Try to lock reference counter for using
  if (lock_remove (pelem) == TS_MINUS_NULL) ///< Reference counter locked successfull
  {
    /// Begin termination element of cache
    if (erase_killed (pelem))
    {
      /// Unlock reference counter from unchanged state
      release_remove (pelem);
      return true;
    }
  }

  /// Cache element's in using, try to wait releasing 
  long retry = TS_SPINLOCK_COUNTER;

  for (; retry > 0; retry--)
  {
    if (pelem->ref == TS_MINUS_NULL)
      break;

    ts_sleep (TS_SPINLOCK_SLEEP_TIME);
  }

  if (!retry)
  {
    brk ();

    /// Mark cache element for removing
    status = change_status (pelem, TS_DEAD_SIGN, TS_KILL_SIGN);
    return false;
  }

  /// Begin termination element of cache
  if (erase_killed (pelem))
  {
    /// Unlock reference counter from unchanged state
    release_remove (pelem);
    return true;
  }

  brk ();
  return false;
}

/// Search array element by key & lock it
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: search_by_key (long& pos, ptce& pelem, Tkey key)
{
  if (!storage || !max_elem) { brk (); return false; }

  long i = 0;

  /// Move to ahead of cache array
  for (i = 0, pos = buble_booster [i], pelem = & storage [pos];
       i < max_elem;
              pos = buble_booster [i], pelem = & storage [pos])
  {
    if (TS_LIVE_SIGN != pelem->status)
    {
      if (erase_dead (pelem))
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);

	if (++i >= max_elem)
	  return false;

	continue;
      }
    }

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected 
      unlock (pelem);

      if (++i >= max_elem)
	return false;

      continue;
    }

    if (pelem->key == key)
    {
      buble_up (i);
      set_timeout (pelem);
      return true;
    }
    else
    if (check_timeout (pelem)
     && change_status (pelem, TS_DEAD_SIGN, TS_LIVE_SIGN) == TS_LIVE_SIGN)
    {
      /// Lock reference counter to unchanged state
      lock_remove (pelem);
      unlock (pelem);

      if (erase_dead (pelem))
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
      }

      if (++i >= max_elem)
	return false;

      continue;
    } ///< end else key

    unlock (pelem);

    if (++i >= max_elem)
      return false;
  } ///< end for

  return false;
}

/// Search array element by hash & lock it
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: search_by_hash (long& pos, ptce& pelem, Thash hash)
{
  if (!storage || !max_elem) { brk (); return false; }

  long i = 0;

  /// Move to ahead of cache array
  for (i = 0, pos = buble_booster [i], pelem = & storage [pos];
       i < max_elem;
              pos = buble_booster [i], pelem = & storage [pos])
  {
    if (TS_LIVE_SIGN != pelem->status)
    {
      if (erase_dead (pelem))
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);

	if (++i >= max_elem)
	  return false;

	continue;
      }
    }

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);

      if (++i >= max_elem)
	return false;

      continue;
    }

    if (pelem->hash == hash)
    {
      buble_up (i);
      set_timeout (pelem);
      return true;
    }
    else
    if (check_timeout (pelem)
     && change_status (pelem, TS_DEAD_SIGN, TS_LIVE_SIGN) == TS_LIVE_SIGN)
    {
      /// Lock reference counter to unchanged state
      lock_remove (pelem);
      unlock (pelem);

      if (erase_dead (pelem))
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
      }

      if (++i >= max_elem)
	return false;

      continue;
    } ///< end else hash

    unlock (pelem);

    if (++i >= max_elem)
      return false;
  } ///< end for pos

  return false;
}

template <class Tkey, class Tvalue, class Thash, class Tallocator>
timer_cache    <Tkey,       Tvalue,       Thash,       Tallocator>

:: timer_cache (const ulonglong timeout, long num_elem = 32)
 : cache_timeout (0), storage (0), top_storage (0), use_counter (0), max_elem (0)
{
  if (num_elem > 0xFFFF) { brk (); num_elem = 0xFFFF; } ///< Elements are avaibled to processing maximum 0xFFFF (65535). Limited by buble booster.

  storage = allocator.allocate (sizeof (*storage) * num_elem);
  if (!storage) { brk (); return; }

  buble_booster = allocator.allocate (sizeof (*buble_booster) * num_elem);
  if (!buble_booster) { brk (); allocator.deallocate (storage), storage = 0; return; }

  memset (storage,       0, sizeof (tce)   * num_elem);
  memset (buble_booster, 0, sizeof (unsigned short) * num_elem);

  ptce p = storage;

  for (long i = 0; i < num_elem; i++, p++)
  {
    p->status = TS_FREE_SIGN;
    buble_booster [i] = (unsigned short) i;
  }

  top_storage = storage + num_elem;
  cache_timeout = timeout;
  max_elem  = num_elem;
}

template <class Tkey, class Tvalue, class Thash, class Tallocator>
timer_cache    <Tkey,       Tvalue,       Thash,       Tallocator>

:: ~timer_cache ()
{
  remove_all_unsafe ();

  max_elem = 0;

  if (storage)       allocator.deallocate (storage),            storage  = top_storage = 0;
  if (buble_booster) allocator.deallocate (buble_booster) buble_booster, buble_booster = 0;
}

/// Insert element in cache & if successfull than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: set_at (long& pos, Tkey key, Thash hash, const Tvalue* pvalue)
{
  if (!storage || !max_elem) { brk (); return false; }

  ptce pelem = 0;

  if (search_by_hash (pos = 0, pelem, hash))
  {
    if (TS_LIVE_SIGN == pelem->status)
    {
      release (pelem);
      return false;
    }

    brk ();
    release (pelem);
  }

  long status = TS_BUSY_SIGN, retry = SET_AT_TRY_COUNTER;

  for (; retry > 0; retry--)
  {
    pelem = storage;

    /// Move to ahead of cache array
    for (pos = 0; pos < max_elem; pos++, pelem++)
    {
      if (TS_LIVE_SIGN == pelem->status
      && check_timeout (pelem)
      && change_status (pelem, TS_DEAD_SIGN, TS_LIVE_SIGN) == TS_LIVE_SIGN)
      {
	/// Lock reference counter to unchanged state
	lock_remove (pelem);
      }

      if (TS_FREE_SIGN != pelem->status)
      {
	if (erase_dead (pelem))
	{
	  /// Try to lock free cache element
	  status = change_status (pelem, TS_BUSY_SIGN, TS_FREE_SIGN);

	  if (TS_FREE_SIGN != status)
	  {
	    brk (); ///< Bingo!!! - concurent SetAt in progress!!!
	    release_remove (pelem);

	    ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	    continue;
	  }

	  /// Lock element
	  lock (pelem);

	  /// Unlock reference counter from unchanged state
	  release_remove (pelem);

	  break;
	}
      }

      /// Lock element
      if (lock (pelem) <= 0)
      {
	/// Locked counter detected
	unlock (pelem);
	continue;
      }

      /// Try to lock free cache element
      status = change_status (pelem, TS_BUSY_SIGN, TS_FREE_SIGN);

      if (TS_FREE_SIGN == status)
	break;

      unlock (pelem);
    } ///< end for pos

    /// Initial FREE status
    if (TS_FREE_SIGN == status)
      break;
  } ///< end for retry

  if (!retry)
  {
    brk ();

    /// Try to remove bottom element
    pos = buble_booster [max_elem - 1];
    pelem = & storage [pos];

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);
      return false;
    }

    /// Remove last element
    if (!remove (pelem))
      return false;

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);
      return false;
    }

    /// Try to lock free cache element
    if (TS_FREE_SIGN != change_status (pelem, TS_BUSY_SIGN, TS_FREE_SIGN))
    {
      unlock (pelem);
      return false;
    }

    buble_up (max_elem - 1);
  }

  Tvalue* pval = allocator.allocate (sizeof (*pval) )

  /// Allocate new cache element
  if (!pval)
  {
    brk ();

    change_status (pelem, TS_FREE_SIGN, TS_BUSY_SIGN);
    unlock (pelem);

    return false;
  }

  tstl :: allocator a;
  :: new ( (void*) pval, a) Tvalue (*pvalue);

  pelem->key  = key;
  pelem->hash = hash;

  /// Activate record
  pelem->pval = pval;

  set_timeout (pelem);

  atomic_inc (& use_counter);

  /// Activate record
  change_status (pelem, TS_LIVE_SIGN, TS_BUSY_SIGN);
  return true;
}

/// Look for element in cache by key & if successfull search than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: lookup_by_key (long& pos, Tkey key, Tvalue*& pvalue)
{
  ptce pelem;

  if (!search_by_key (pos = 0, pelem, key) )
    return false;

  pvalue = pelem->pval;
  return true;
}

/// Look for element in cache by keys hash & if successfull search than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: lookup_by_key_hash (long& pos, Tkey key, Tvalue*& pvalue)
{
  ptce pelem;
  Thash hash = hk.hash (key);

  if (!search_by_hash (pos = 0, pelem, hash) )
    return false;

  pvalue = pelem->pval;
  return true;
}

/// Look for element in cache by hash & if successfull search than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: lookup_by_hash (long& pos, Thash hash, Tvalue*& pvalue)
{
  ptce pelem;

  if (!search_by_hash (pos = 0, pelem, hash) )
    return false;

  pvalue = pelem->pval;
  return true;
}

/// Remove element from cache by key
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove_by_key (Tkey key)
{
  Tvalue* pvalue = 0;
  long pos = 0;

  if (!lookup_by_key (pos, key, pvalue) )
    return false;

  return remove (pos);
}

/// Remove element from cache by keys hash
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove_by_key_hash (Tkey key)
{
  Tvalue* pvalue = 0;
  long pos = 0;

  if (!lookup_by_key_hash (pos, key, pvalue) )
    return false;

  return remove (pos);
}

/// Remove element from cache by hash
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove_by_hash (Thash hash)
{
  Tvalue* pvalue = 0;
  long pos = 0;

  if (!lookup_by_hash (pos, hash, pvalue) )
    return false;

  return remove (pos);
}

template <class Tkey, class Tvalue, class Thash, class Tallocator>
void timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove_all ()
{
  Tkey key;
  Tvalue* pv;
  size_t pos = 0, hash;

  if (!start (pos, key, hash, pv))
    return;

  do { remove (pos); }
  while (next (pos, key, hash, pv));
}

/// Doesn't thread safe method, it called from destructor
template <class Tkey, class Tvalue, class Thash, class Tallocator>
void timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: remove_all_unsafe ()
{
  /// Move to ahead of cache array
  for (ptce pelem = storage; pelem < top_storage; pelem++)
  {
    if (TS_FREE_SIGN == pelem->status)
      continue;

    if (!pelem->pval) /* BUSY && ERAS may be */
      continue;

    pelem->pval-> ~Tvalue ();

    allocator.deallocate (pelem->pval), pelem->pval = 0;

    atomic_dec (&use_counter);
  }
}

/// Next caches enumerating & if failure than unlock last element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: start (long& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  ptce pelem = & storage [pos];

  /// Move to ahead of cache array
  for (; pos < max_elem; pos++, pelem++)
  {
    if (TS_LIVE_SIGN != pelem->status)
    {
      if (erase_dead (pelem) )
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
	continue;
      }
    }

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);
      continue;
    }

    key    = pelem->key;
    hash   = pelem->hash;
    pvalue = pelem->pval;

    return true;
  }

  return false;
}

/// Next caches enumerating & if failure than unlock last element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool timer_cache <Tkey,     Tvalue,       Thash,       Tallocator>

:: start (long& pos, Thash hash, Tvalue*& pvalue)
{
  ptce pelem = & storage [pos];

  /// Move to ahead of cache array
  for (; pos < max_elem; pos++, pelem++)
  {
    if (TS_LIVE_SIGN != pelem->status)
    {
      if (erase_dead (pelem))
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
	continue;
      }
    }

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);
      continue;
    }

    if (hash != pelem->hash)
    {
      unlock (pelem);
      continue;
    }

    pvalue = pelem->pval;

    return true;
  }

  return false;
}

template <class Tcache>
static bool init_tc (Tcache*& pc, int num_elem, ulonglong cache_timeout)
{
  pc = new Tcache (cache_timeout, num_elem);
  return 0 != pc;
};

}; /* end of tstl namespace */

#endif /* __TIMERCACHE_HPP__ */
