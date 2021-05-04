/*****************************************************************************************************//**
 *
 *  Module Name:	\file limitcache.hpp
 *
 *  Abstract:		\brief Cache manager with cleaning policy by element number limit.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2007 started
 *			\date 16.04.2008 reimplemented
 *
 *  Classes, methods and structures: \details
 *
 *  External:	multimap, mp, melocker (relocker), allocator
 *  Internal:	limit_cache
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __LIMITCACHE_HPP__
#define __LIMITCACHE_HPP__

#include "tsmap.hpp"

#include "impl/tslist.hpp"
#include "impl/relocker.hpp"

namespace tstl {

template <class Tkey, class Tvalue, class Thash = size_t,
          class Tlocker = melocker<>, class Tallocator = allocator,
          class Tmultimap = nbmap :: multimap <Tkey, Tvalue, Thash, Tallocator>,
          class Tmap_pos  = nbmap :: mp>

class limit_cache
{
  /// double linked looped list element
  typedef struct limit_cache_elem
  {
    /// redanted service information
    list_head lh;
    Thash   hash; ///< for reverse map clean

    /// usefull payload
    Tvalue* pval;
  } lce, *plce;

  plce storage, top_storage, head, tail;
  long use_counter, max_elem;
  Tlocker  list_locker;
  Tallocator allocator;

  /// limit cache hash to plce storage
  multimap <Tkey, lce*, Thash, Tallocator>* plcm;

  plce lookup_elem (Tmap_pos& pos)
  {
    if (!plcm) { brk (); return 0; }

    plce* ppelem = (plce*) plcm->lookup (pos);
 
    if (!storage || !ppelem || !*ppelem || *ppelem >= top_storage || *ppelem < storage)
    { brk (); return 0; }

    return *ppelem;
  }

  /// CleanUp element
  bool remove_dead (plce pelem);

  bool up_elem (plce pelem);

  bool clean_map_refences (plce& pelem);

  bool put_elem (Tmap_pos& pos, plce& pelem, Tkey& key, Thash& hash, 
                 Tvalue* pvalue, const Tvalue* porig_val);

  /// Search array element by key & lock it
  bool search_by_key  (Tmap_pos& pos, plce& pelem, Tkey key);

  /// Search array element by hash & lock it
  bool search_by_hash (Tmap_pos& pos, plce& pelem, Thash hash);

  void remove_all_unsafe ();

public:
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  limit_cache (const long num_elem = 32);

  ~limit_cache ();

  /// Doesn't thread safe method
  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about cache using
  long get_stat () const
  { return use_counter; }

  /// Get hash by key
  Thash hash (Tkey key) const
  {
    if (!plcm) { brk (); return 0; }
    return plcm->hash (key);
  }

  /// Free reference on element
  bool release (Tmap_pos& pos)
  {
    if (!plcm) { brk (); return false; }
    plcm->release (pos);
    return true;
  }

  /// Synoname of Release
  long unlock (Tmap_pos& pos)
  { return release (pos); }

  /// Insert element in cache & if successfull than lock element
  bool set_at (Tmap_pos& pos, Tkey key, Thash hash, const Tvalue* pvalue);

  /// Insert element in cache & if successfull than lock element
  bool set_at (Tmap_pos& pos, Tkey key, const Tvalue* pvalue)
  {
    if (!plcm) { brk (); return false; }
    return set_at (pos, key, plcm->hash (key), pvalue);
  }

  /// Look for element in cache by key & if successfull search than lock element
  bool lookup_by_key (Tmap_pos& pos, Tkey key, Tvalue*& pvalue);

  /// Look for element in cache by keys hash & if successfull search than lock element
  bool lookup_by_key_hash (Tmap_pos& pos, Tkey key, Tvalue*& pvalue);

  /// Look for element in cache by hash & if successfull search than lock element
  bool lookup_by_hash (Tmap_pos& pos, Thash hash, Tvalue*& pvalue);

  /// Look for element in cache by pointer on pval
  Tvalue* lookup (Tmap_pos& pos)
  {
    plce pelem = lookup_elem (pos);
    if (!pelem) return 0;
    return pelem->pval;
  }

  /// Remove element from cache & unlock element if successfull
  bool remove (Tmap_pos& pos);

  /// Remove element from cache on cleanup
  bool remove_dead (Tmap_pos& pos);

  /// Remove element from cache by key
  bool remove_by_key (Tkey key);

  /// Remove element from cache by keys hash
  bool remove_by_key_hash (Tkey key);

  /// Remove element fro1m cache by hash
  bool remove_by_hash (Thash hash);

  void remove_all ();

  /// Next cache enumerating. If it returned fail than last element was unlocked
  bool start (Tmap_pos& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Begin cache enumerating. If it returned success than element was locked
  bool next  (Tmap_pos& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Next cache enumerating. If it returned fail than last element was unlocked
  bool start (Tmap_pos& pos, Thash hash, Tvalue*& pvalue);

  /// Begin cache enumerating. If it returned success than element was locked
  bool next  (Tmap_pos& pos, Thash hash, Tvalue*& pvalue);
};

/// CleanUp element
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_dead (plce pelem)
{
  if (!pelem || pelem >= top_storage)
  { brk (); return false; }

  pelem->hash = 0;

  if (!pelem->pval)
    return true;

  pelem->pval-> ~Tvalue ();

  allocator.deallocate (pelem->pval), pelem->pval = 0;

  return true;
}

template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: up_elem (plce pelem)
{
  if (!storage || !max_elem || !pelem || pelem >= top_storage 
   || !pelem->lh.next || !pelem->lh.prev || !head->lh.next || !head->lh.prev)
  { brk (); return false; }

  if (use_counter < 2
   || pelem == head
  || (pelem == tail
   && pelem == head)
  || list_empty (& pelem->lh))
    return false;

  if (pelem == tail)
  {
    if ((plce)pelem->lh.next == head)
    {
      if (use_counter == 2)
      {/// do swap head and tail
	tail = head;
	head = pelem;
	return true;
      }

      brk ();
      return false;
    }

    tail = (plce) pelem->lh.next;
    head = pelem;
    return true;
   }

  list_del (& pelem->lh);

  /// Add element between head and head->next (tail)
  list_add (& pelem->lh, & head->lh);

  head = pelem;
  return true;
}

template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: clean_map_refences (plce& pelem)
{
  if (!pelem->pval && !pelem->hash)
  { brk (); return true; }

  if (!plcm) { brk (); return true; }

  Tmap_pos pos;
  plce* pvalue = 0;

  long cnt = sizeof (size_t) << (3 + 1);

  while (plcm->start (pos, pelem->hash, pvalue))
  {
next_elem:
    if (pvalue
    && *pvalue == pelem) ///< look for self and remove from map and delete value
    {
      if (!plcm->remove (pos))
      {
	brk (); ///< bad breakpoint
	plcm->release (pos);
	return false;
      }

      pvalue = 0;
    }
    else
    {
      brk (); ///< time for change hash function

      if (!cnt--)
      {
	brk (); ///< bad break point
	plcm->release (pos);
	return false;
      }

      if (!plcm->next (pos, pelem->hash, pvalue))
      {
	brk (); ///< bad break point
	return true; ///< no more referenses from map
      }

      goto next_elem;
    } 
  } /* end while */

  return true;
}

template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: put_elem (Tmap_pos& pos, plce& pelem, Tkey& key, Thash& hash, Tvalue* pval, const Tvalue* porig_val)
{
  if (!storage || !max_elem || !plcm || !pval || !porig_val)
  { brk (); return false; }

  bool up_elem = false;

  if (use_counter < max_elem)
    pelem = & storage [use_counter++];
  else
  {
    long cnt = 0x100 < use_counter ? 0x100 : use_counter;

    pelem = tail;

    /// Try to free element from tail
    while (cnt-- > 0 && pelem->pval)
    {
      if (clean_map_refences (pelem))
      {
	remove_dead (pelem);
	break;
      }

      if ( (plce) pelem->lh.next == head)
      {
	brk ();
	return false;
      }

      /// move tail to head
      head = pelem;
      tail = pelem = (plce) pelem->lh.next;
    }

    if (cnt <= 0 || pelem->pval)
    {
      brk ();
      pelem = 0;
      return false;
    }

    up_elem = true;
  }

  if (!plcm->set_at (pos, key, hash, & pelem) )
  {
    brk ();
    pelem = 0;
    return false;
  }

  tstl :: allocator a;
  :: new ( (void*) pval, a) Tvalue (*porig_val);

  pelem->hash = hash;
  pelem->pval = pval;

  /// Update Head and Tail
  /** setup tail pointer on next victim */
  if (up_elem)
  {
    if ( (plce) pelem->lh.next == head)
    { brk (); }
    else
    {
      tail = (plce) pelem->lh.next;
      head = pelem;
    }
  }
  else
  {
    list_add (& pelem->lh, & head->lh); /// add to head of list
    /// tail auto shifted
    head = pelem;
  }

  return true;
}

/// Search array element by key & lock it
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: search_by_key (Tmap_pos& pos, plce& pelem, Tkey key)
{
  if (!storage || !max_elem || !plcm)
  { brk (); return false; }

  plce* ppvalue = 0;

  if (!plcm->lookup_by_key (pos, key, ppvalue) )
    return false;

  if (!ppvalue || !*ppvalue)
  {
    brk (); /// bad breakpoint

    if (!plcm->remove (pos) )
    { brk (); plcm->release (pos); }

    return false;
  }

  pelem = *ppvalue;
  return true;
}

/// Search array element by hash & lock it
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: search_by_hash (Tmap_pos& pos, plce& pelem, Thash hash)
{
  if (!storage || !max_elem || !plcm)
  { brk (); return false; }

  plce* ppvalue = 0;

  if (!plcm->lookup_by_hash (pos, hash, ppvalue))
    return false;

  if (!ppvalue || !*ppvalue)
  {
    brk (); /// bad breakpoint

    if (!plcm->remove (pos) )
    { brk (); plcm->release (pos); }
     
    return false;
  }

  pelem = *ppvalue;
  return true;
}


template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
limit_cache      <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: limit_cache (const long num_elem = 32)
 : storage (0), top_storage (0), head (0), tail (0), use_counter (0), max_elem (0)
{
  storage = (lce*) allocator.allocate (sizeof (*storage) * num_elem);
  if (!storage) { brk (); return; }

  init_map (plcm, num_elem >> 3);

  if (!plcm) { brk (); allocator.deallocate (storage), storage = 0; return; }

  memset (storage, 0, sizeof (*storage) * num_elem);

  head = tail = storage;
  INIT_LIST_HEAD (& head->lh);

  top_storage = storage + num_elem;
  max_elem    = num_elem;
}

template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
limit_cache      <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: ~limit_cache ()
{
  remove_all_unsafe ();

  max_elem = 0;

  if (storage) allocator.deallocate (storage), storage = top_storage = 0;
  if (plcm)    delete plcm, plcm = 0;
}

/// Insert element in cache & if successfull than lock element
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: set_at (Tmap_pos& pos, Tkey key, Thash hash, const Tvalue* pvalue)
{
  if (!storage || !max_elem || !plcm)
  { brk (); return false; }

  plce pelem = 0;

  list_locker.lock ();

  if (search_by_hash (pos, pelem, hash) )
  {
    release (pos);

    list_locker.unlock ();
    return false;
  }

  Tvalue* new_val = (Tvalue*) allocator.allocate (sizeof (*new_val) );
  if (!new_val) { brk (); list_locker.unlock (); return false; }

  if (put_elem (pos, pelem, key, hash, new_val, pvalue) )
  { list_locker.unlock (); return true; }

  list_locker.unlock ();

  brk ();

  if (new_val) allocator.deallocate (new_val), new_val = 0;
  return false;
}

/// Look for element in cache by key & if successfull search than lock element
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: lookup_by_key (Tmap_pos& pos, Tkey key, Tvalue*& pvalue)
{
  plce pelem = 0;

  list_locker.lock ();

  if (!search_by_key (pos, pelem, key) )
  { list_locker.unlock (); return false; }

  if (!pelem)
  {
    brk (); /// bad break point
    release (pos);

    list_locker.unlock ();
    return false;
  }

  up_elem (pelem);

  list_locker.unlock ();

  pvalue = pelem->pval;
  return true;
}

/// Look for element in cache by keys hash & if successfull search than lock element
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: lookup_by_key_hash (Tmap_pos& pos, Tkey key, Tvalue*& pvalue)
{
  Thash hash = plcm->hash (key);
  plce pelem = 0;

  list_locker.lock ();

  if (!search_by_hash (pos, pelem, hash) )
  { list_locker.unlock (); return false; }

  if (!pelem)
  {
    brk (); ///< bad break point
    release (pos);

    list_locker.unlock ();
    return false;
  }

  up_elem (pelem);

  list_locker.unlock ();

  pvalue = pelem->pval;
  return true;
}

/// Look for element in cache by hash & if successfull search than lock element
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: lookup_by_hash (Tmap_pos& pos, Thash hash, Tvalue*& pvalue)
{
  plce pelem = 0;

  list_locker.lock ();

  if (!search_by_hash (pos, pelem, hash) )
  { list_locker.unlock (); return false; }

  if (!pelem)
  {
    brk (); /// bad break point
    release (pos);

    list_locker.unlock ();
    return false;
  }

  up_elem (pelem);

  list_locker.unlock ();

  pvalue = pelem->pval;
  return true;
}

/// Remove element from cache & unlock element if successfull
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove (Tmap_pos& pos)
{
  if (!plcm) { brk (); return false; }

  plce pelem = lookup_elem (pos);
 
  if (!pelem)
  { brk (); /** bad break point, position locked */ return false; }

  list_locker.lock ();

  if (!plcm->remove (pos) )
  { brk (); list_locker.unlock (); return false; }

  remove_dead (pelem);

  list_locker.unlock ();

  return true;
}

/// Remove element from cache on cleanup
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_dead (Tmap_pos& pos)
{
  if (!plcm) { brk (); return false; }

  plce pelem = lookup_elem (pos);

  if (!pelem)
    return false;

  if (!plcm->remove (pos) )
  { brk (); return false; }

  remove_dead (pelem);
  return true;
}

/// Remove element from cache by key
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_by_key (Tkey key)
{
  if (!plcm) { brk (); return false; }

  Tmap_pos pos;
  plce pelem = 0;

  list_locker.lock ();

  if (!search_by_key (pos, pelem, key) )
  { list_locker.unlock (); return false; }

  if (!pelem)
  {
    brk (); ///< bad break point
    release (pos);

    list_locker.unlock ();
    return false;
  }

  if (!plcm->remove (pos) )
  {
    brk ();
    release (pos);

    list_locker.unlock ();
    return false;
  }

  remove_dead (pelem);

  list_locker.unlock ();
  return true;
}

/// Remove element from cache by keys hash
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_by_key_hash (Tkey key)
{
  if (!plcm) { brk (); return false; }

  Tmap_pos pos;
  plce pelem = 0;

  Thash hash = plcm->hash (key);

  list_locker.lock ();

  if (!search_by_hash (pos, pelem, hash) )
  { list_locker.unlock (); return false; }

  if (!pelem)
  {
    brk (); ///< bad break point
    release (pos);

    list_locker.unlock ();
    return false;
  }

  if (!plcm->remove (pos) )
  {
    brk ();
    release (pos);

    list_locker.unlock ();
    return false;
  }

  remove_dead (pelem);

  list_locker.unlock ();
  return true;
}

/// Remove element fro1m cache by hash
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_by_hash (Thash hash)
{
  if (!plcm) { brk (); return false; }

  Tmap_pos pos;
  plce pelem = 0;

  list_locker.lock ();

  if (!search_by_hash (pos, pelem, hash) )
  { list_locker.unlock (); return false; }

  if (!pelem)
  {
    brk (); ///< bad break point
    release (pos);

    list_locker.unlock ();
    return false;
  }

  if (!plcm->remove (pos))
  {
    brk ();
    release (pos);

    list_locker.unlock ();
    return false;
  }

  remove_dead (pelem);

  list_locker.unlock ();
  return true;
}

/// Doesn't thread safe method, it called from destructor
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
void limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_all_unsafe ()
{
  if (plcm) plcm->remove_all ();

  /// Move to ahead of cache array
  for (plce pelem = storage; pelem < top_storage; pelem++)
    remove_dead (pelem);

  INIT_LIST_HEAD (& head->lh);

  head = tail = storage;
  use_counter = 0;
}

template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
void limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: remove_all ()
{
  list_locker.lock ();

  remove_all_unsafe ();

  list_locker.unlock ();
}

/// Next cache enumerating. If it returned fail than last element was unlocked
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: start (Tmap_pos& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  if (!plcm) { brk (); return false; }

  bool rc = false;

  list_locker.lock ();

  rc = plcm->start (pos, key, hash, pvalue);

  list_locker.unlock ();

  return rc;
}

/// Begin cache enumerating. If it returned success than element was locked
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: next (Tmap_pos& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  if (!plcm) { brk (); return false; }

  bool rc = false;

  list_locker.lock ();

  rc = plcm->next (pos, key, hash, pvalue);

  list_locker.unlock ();

  return rc;
}

/// Next cache enumerating. If it returned fail than last element was unlocked
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: start (Tmap_pos& pos, Thash hash, Tvalue*& pvalue)
{
  if (!plcm) { brk (); return false; }

  bool rc = false;

  list_locker.lock ();

  rc = plcm->start (pos, hash, pvalue);

  list_locker.unlock ();

  return rc;
}

/// Begin cache enumerating. If it returned success than element was locked
template   <class Tkey, class Tvalue, class Thash, class Tlocker, class Tallocator, class Tmultimap, class Tmap_pos>
bool limit_cache <Tkey,       Tvalue,       Thash,       Tlocker,       Tallocator,       Tmultimap,       Tmap_pos>

:: next (Tmap_pos& pos, Thash hash, Tvalue*& pvalue)
{
  if (!plcm) { brk (); return false; }

  bool rc = false;

  list_locker.lock ();

  rc = plcm->next (pos, hash, pvalue);

  list_locker.unlock ();

  return rc;
}

template <class Tcache>
static bool init_lc (Tcache*& pc, const long num_elem)
{
  pc = new Tcache (num_elem);
  return 0 != pc;
};

}; /* end of tstl namespace */

#endif /* __LIMITCACHE_HPP__ */
