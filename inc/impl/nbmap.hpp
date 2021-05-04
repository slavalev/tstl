/*****************************************************************************************************//**
 *
 *  Module Name:	\file nbmap.hpp
 *
 *  Abstract:		\brief B-tree based multimap with only interlocked locked each cell of container.
 *
 *  Author:	        \author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History: \date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External:	hash_key, ts_sleep, allocator
 *  Internal:	multimap, enum_pos, mp (map_pos)
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __NBMAP_HPP__
#define __NBMAP_HPP__

#include "tstl.hpp"

#include "impl/tshash.hpp"

namespace tstl {
namespace nbmap {

/// Map initial configuration
#define NB_MAP_LEVEL_LENGTH	4
#define NB_MAP_HASH_LENGTH	(sizeof (Thash) << 3)
#define NB_MAP_MAX_LEVELS	( (NB_MAP_HASH_LENGTH / NB_MAP_LEVEL_LENGTH) + 1)

/// Used on map enumerating
typedef struct enum_pos
{
  void* map;          ///< back trace map
  unsigned long elem; ///< back trace element
} ep, *pep;

template <class Tallocator = allocator>

struct map_pos
{
  void* map;
  pep   p;            ///< Used for map enumerating
  unsigned long cnt;  ///< Enumeration counter
  unsigned long elem; ///< Current locked element

  Tallocator allocator;

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

   map_pos () : map (0), elem (0), cnt (0), p (0) {}
  ~map_pos () { if (p) { allocator.deallocate (p), p = 0; } }

  map_pos& operator = (const map_pos& old)
  {
    map  = old.map;
    elem = old.elem;
    return *this;
  }

  bool enum_init (const long levels)
  {
    if (!p)
    {
      p = (pep) allocator.allocate (sizeof (*p) * levels);
      if (!p) { brk ();	return false; }
    }

    memset (p, 0, sizeof (ep) * levels);
    cnt = 0;
    return true;
  }
};

typedef map_pos<> mp, *pmp;

/// Object Status Graph (OSG)
/** +---------------------LOOPBACK----------------------+
  * +-> FREE -> BUSY -> LIVE -> KILL +---->---+-> ERAS -+
  *                                  +-> DEAD +        */
template <class Tkey, class Tvalue, class Thash = size_t, class Tallocator = allocator>

class multimap
{
  typedef struct map_elem
  {
    /// redanted service information
    long  status; ///< "FREE" || "BUSY" || "LIVE" || "KILL" || "DEAD" || "ERAS"
    long  ref;

    /// usefull payload
    Tvalue*   pval;
    multimap* pmap; ///< Collision map
    Thash hash;
    Tkey  key;
  } me, *pme;

  pme storage;

  /// b-tree branch description
  typedef struct hash_part
  {
    Thash mask;
    unsigned char off;
  } hash_part, *phash_part;

  /// b-tree design template description
  typedef struct map_arch
  {
    hash_part hp [NB_MAP_MAX_LEVELS];
    unsigned char levels; /// 0-based. levels = (real Levels - 1)
  } ma, *pma;

  pma pmap_arch;

  unsigned long level_elems;
  long use_counter;
  unsigned char level;

  Tallocator allocator;
  
  /// Hash by Key Generator
  hash_key<Tkey, Thash> hk;

  /// Maps private methods

  /// Lock reference on element
  long lock (pme& pelem) const
  { return atomic_inc_return (& pelem->ref); }

  /// Free reference on element
  long release (pme& pelem) const
  { return atomic_dec_return (& pelem->ref); }

  /// Synoname of release
  long unlock (pme& pelem) const
  { return release (pelem); }

  /// Lock reference counter removed element
  long lock_remove (pme& pelem) const
  { return (atomic_add_return (& pelem->ref, TS_MINUS_NULL) + TS_MINUS_NULL); }

  /// Free reference counter removed element
  void release_remove (pme& pelem) const
  { atomic_add_return (& pelem->ref, TS_MINUS_MEDIAN); }

  /// Synoname of release_remove
  void unlock_remove (pme& pelem) const
  { release_remove (pelem); }

  /// Change status of map element from XXX -> (to) YYY
  long change_status (pme& pelem, long new_status, long previos_status) const
  { return atomic_compare_exchange (& pelem->status, new_status, previos_status); }

  /// Cleanup element
  bool remove_dead (pme pelem);

  /// ERAS -> FREE. Enable only in ERASE status. If successfull than setup FREE status
  bool erase (pme& pelem);

  /// DEAD -> ERAS -> FREE. Return true if erase dead element
  bool erase_dead (pme& pelem);

  /// KILL -> ERAS -> FREE. Return true if erase element
  bool erase_killed (pme& pelem);

  /// LIVE -> KILL (-> ERAS -> FREE) || LIVE -> KILL -> DEAD
  /** Set status FREE or DEAD, begin with LIVE status going over KILL and ERAS
    * \return false if pelem biger of TopStorage */
  bool remove (pme pelem);

  /// Doesn't thread safe method, it called from destructor
  void remove_all_unsafe ();

  /// Check and lock LIVE element else go down in to lower map
  bool look_for_live_elem (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Get element index in map by key hash
  unsigned long get_elem (const Thash& hash) const
  {
    if (!pmap_arch) { brk (); return 0; }
    phash_part pHP = & pmap_arch->hp [level];
    return (unsigned long) ( (hash >> pHP->off) & pHP->mask);
  }

  unsigned char num_bits (Thash val) const
  {
    unsigned char cnt = 0;
    while (val & 1) { val >>= 1; cnt++; }
    return cnt;
  }

  Thash get_mask (unsigned char size) const
  {
    Thash mask = 0;
    while (size--) { mask |= (Thash) 1 << size; }
    return mask;
  }

  unsigned long get_max_boolean_divider (unsigned long dividend);

  bool map_init ();

public:

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  /// Root map initilize
  multimap (const unsigned long root_array_elems = 32);

  /// Sub map initilize
  multimap (const unsigned char in_level, const pma in_pmap_arch) : storage (0), use_counter (0), level_elems (0), level (1)
  {
    if (!in_pmap_arch || !in_level || in_level > in_pmap_arch->levels)
    { brk (); return; }

    level     = in_level;
    pmap_arch = in_pmap_arch;

    map_init ();
  }

  ~multimap ()
  {
    remove_all_unsafe ();

    if (storage) allocator.deallocate (storage), storage = 0;
    if (!level && pmap_arch) allocator.deallocate (pmap_arch), pmap_arch = 0;
  }

  /// Doesn't thread safe method
  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about map using
  long get_stat () const
  { return use_counter; }

  /// Get statistic about using map element
  long get_stat (Thash map_elem) const
  {
    if (!storage || !level_elems)
    { brk (); return 0; }

    return storage [map_elem % level_elems].ref;
  }

  /// Get hash by key
  Thash hash (Tkey key) const
  { return hk.hash (key); }

  /// Search array element by key & lock it
  bool search_by_key (mp& pos, Tkey key, Thash hash, Tvalue*& pvalue);

  /// Search array element by hash & lock it
  bool search_by_hash (mp& pos, Thash hash, Tvalue*& pvalue);

  /// Insert element in map & if successfull than lock element
  bool set_at (mp& pos, Tkey key, Thash hash, const Tvalue* pvalue);

  /// Insert element in map & if successfull than lock element
  bool set_at (mp& pos, Tkey key, const Tvalue* pvalue)
  { return set_at (pos, key, hk.hash (key), pvalue); }

  /// Look for element in map by key & if successfull search than lock element
  bool lookup_by_key (mp& pos, Tkey key, Tvalue*& pvalue)
  { return search_by_key (pos, key, hk.hash (key), pvalue); }

  /// Look for element in map by keys hash & if successfull search than lock element
  bool lookup_by_key_hash (mp& pos, Tkey key, Tvalue*& pvalue)
  { return search_by_hash (pos, hk.hash (key), pvalue); }

  /// Look for element in map by hash & if successfull search than lock element
  bool lookup_by_hash (mp& pos, Thash hash, Tvalue*& pvalue)
  { return search_by_hash (pos, hash, pvalue); }

  /// Look for element in map by position
  Tvalue* lookup (mp& pos);

  /// Unlock position in map
  void release (mp& pos);

  /// Synoname of release
  void unlock (mp& pos)
  { release (pos); }

  /// Remove element from map and always unlock element
  bool remove (mp& pos);

  /// Remove element from map on cleanup
  bool remove_dead (mp& pos);

  /// Remove element from map by key
  bool remove_by_key (Tkey key);

  /// Remove element from map by keys hash
  bool remove_by_key_hash (Tkey key);

  /// Remove element from map by hash
  bool remove_by_hash (Thash hash);

  void remove_all ();

  /// Next maps enumerating & if failure than unlock last element
  bool next (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Begin maps enumerating & if successfull than lock element
  bool start (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  multimap* get_next_map (Thash& hash)
  {
    if (!storage) { brk (); return 0; }
    return storage [get_elem (hash) ].pmap;
  }

  /// Begin maps enumerating by hash & if successfull than lock element
  bool start (mp& pos, Thash hash, Tvalue*& pvalue)
  { return search_by_hash (pos, hash, pvalue); }

  /// Next maps enumerating by hash & if failure than unlock last element
  bool next (mp& pos, Thash hash, Tvalue*& pvalue);
};

/// Cleanup element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_dead (pme pelem)
{
  if (!pelem) { brk (); return false; }

  if (!pelem->pval) ///< BUSY && ERAS may be
  {
    pelem->ref  = 0;
    pelem->hash = 0;
    pelem->key  = 0;
    pelem->status = TS_FREE_SIGN;
    return false;
  }

  pelem->pval-> ~Tvalue ();

  allocator.deallocate (pelem->pval), pelem->pval = 0;

  pelem->ref  = 0;
  pelem->hash = 0;
  pelem->key  = 0;
  pelem->status = TS_FREE_SIGN;

  atomic_dec (& use_counter);
  return true;
}

/// ERAS -> FREE. Enable only in ERASE status. If successfull than setup FREE status
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: erase (pme& pelem)
{
  Tvalue* locp = pelem->pval;
  Tvalue* prev = (Tvalue*) atomic_compare_exchange ( (void**) & pelem->pval, 0, locp);

  if (locp
   && locp == prev)
  {
    pelem->hash = 0;
    pelem->key  = 0;

    atomic_dec (& use_counter);

    if (locp)
    {
      ///< delete pval
      locp-> ~Tvalue ();

      allocator.deallocate (locp), locp = 0;
    }

    /// Set FREE status
    change_status (pelem, TS_FREE_SIGN, TS_ERAS_SIGN);
    return true;
  }
  else
  {
    /// Element corupted, very bad break point !!!
    brk ();
    /// status don't touch
    return false;
  }
}

/// DEAD -> ERAS -> FREE. Return true if erase dead element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: erase_dead (pme& pelem)
{
  /// Change status on ERASE
  long status = change_status (pelem, TS_ERAS_SIGN, TS_DEAD_SIGN);

  if (TS_DEAD_SIGN == status)
  {
    /// Setup ERASE status successfull
    if (pelem->ref == TS_MINUS_NULL)
    {
      /// Reference counter locked successfull
      /** Begin termination dead element of map */
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
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: erase_killed (pme& pelem)
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
  * \return false if pelem biger of TopStorage */
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove (pme pelem)
{
  if (pelem >= storage + level_elems) { brk (); return false; }

  /// If LIVE status passed, than entry to killing status
  long status = change_status (pelem, TS_KILL_SIGN, TS_LIVE_SIGN);

  /// Release element
  release (pelem); // == 0

  /// Map element status control
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
    { brk (); }
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
    /// Begin termination element of map
    if (erase_killed (pelem) )
    {
      /// Unlock reference counter from unchanged state
      release_remove (pelem);
      return true;
    }
  }

  /// Map element's in using, try to wait releasing
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

    /// Mark map element for removing
    status = change_status (pelem, TS_DEAD_SIGN, TS_KILL_SIGN);
    return false;
  }

  /// Begin termination element of map
  if (erase_killed (pelem) )
  {
    /// Unlock reference counter from unchanged state
    release_remove (pelem);
    return true;
  }

  return false;
}

template <class Tkey, class Tvalue, class Thash, class Tallocator>
unsigned long multimap <Tkey,   Tvalue,   Thash,       Tallocator>

:: get_max_boolean_divider (unsigned long dividend)
{
  if (!dividend) { brk (); return 0; }

  unsigned long divider = 1, max_divider = 0, i = 0;

  for (; i++ < sizeof (long) * 8; divider <<= 1)
    if (dividend & divider)
      max_divider = divider;

  if (!max_divider) { brk (); }

  return max_divider;
}

template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: map_init ()
{
  level_elems = (unsigned long) 1 << num_bits (pmap_arch->hp [level].mask);

  /// Use level_elems
  storage = (me*) allocator.allocate (sizeof (*storage) * level_elems);

  if (!storage) { brk(); return false; }

  memset (storage, 0, sizeof (me) * level_elems);

  pme p = storage;

  /// Initialize map elements
  for (unsigned long i = 0; i < level_elems; i++, p++)
    p->status = TS_FREE_SIGN;

  return true;
}

/// Root map initilise
template <class Tkey, class Tvalue, class Thash, class Tallocator>
multimap       <Tkey,       Tvalue,       Thash,       Tallocator>

:: multimap (const unsigned long root_array_elems = 32)
 : storage (0), use_counter (0), level_elems (0), level (0)
{
  if (!root_array_elems) { brk (); return; }

  unsigned long root_level_elems = get_max_boolean_divider (root_array_elems);

  if (!root_level_elems) { brk (); root_level_elems = 0x100; }

  pmap_arch = (ma*) allocator.allocate (sizeof (*pmap_arch) );
  if (!pmap_arch) { brk (); return; }

  pmap_arch->levels = 0;

  unsigned char size = num_bits (root_level_elems - 1);
  pmap_arch->hp [pmap_arch->levels].off  = 0;
  pmap_arch->hp [pmap_arch->levels].mask = get_mask (size);

  /// Generate level_sizes and level_mask
  unsigned char last = (unsigned char) NB_MAP_HASH_LENGTH - size;

  while (last)
  {
    if (++(pmap_arch->levels) >= (unsigned char)(NB_MAP_MAX_LEVELS - 1) )
    {
      brk ();
      pmap_arch->levels = (unsigned char)(NB_MAP_MAX_LEVELS - 1);
      return;
    }

    /// previous level size plus previous offset
    pmap_arch->hp [pmap_arch->levels].off = pmap_arch->hp [pmap_arch->levels - 1].off + size;

    size = last > NB_MAP_LEVEL_LENGTH ? NB_MAP_LEVEL_LENGTH : last;

    pmap_arch->hp [pmap_arch->levels].mask = get_mask (size);
    last -= size;
  }

  map_init ();
}

#define TS_GO_DOWN_RET(METHOD)	\
  if (pelem->pmap)		\
    return pelem->pmap->METHOD;	\
  else				\
    return false;

/// Search array element by key & lock it
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: search_by_key (mp& pos, Tkey key, Thash hash, Tvalue*& pvalue)
{
  if (!storage || !level_elems)
  { brk (); return false; }

  pos.map   = this;
  pos.elem  = get_elem (hash);
  pme pelem = & storage [pos.elem];

  if (TS_LIVE_SIGN != pelem->status)
  {
    if (erase_dead (pelem))
    {
      brk ();
      /// Unlock reference counter from unchanged state
      release_remove (pelem);
    }

    /// Element empty, go down
    TS_GO_DOWN_RET (search_by_key (pos, key, hash, pvalue) );
  }

  /// Lock element
  if (lock (pelem) <= 0)
  {
    /// Locked counter detected
    unlock (pelem);

    TS_GO_DOWN_RET (search_by_key (pos, key, hash, pvalue) );
  }

  if (TS_LIVE_SIGN != pelem->status)
  {
    unlock (pelem);

    TS_GO_DOWN_RET (search_by_key (pos, key, hash, pvalue) );
  }

  /// Element successfully locked
  if (pelem->key == key)
  {
    pvalue = pelem->pval;
    return true;
  }

  /// Key doesn't equial, go down
  unlock (pelem);

  TS_GO_DOWN_RET (search_by_key (pos, key, hash, pvalue));
}

/// Search array element by hash & lock it
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: search_by_hash (mp& pos, Thash hash, Tvalue*& pvalue)
{
  if (!storage || !level_elems)
  { brk (); return false; }

  pos.map   = this;
  pos.elem  = get_elem (hash);
  pme pelem = & storage [pos.elem];

  if (TS_LIVE_SIGN != pelem->status)
  {
    if (erase_dead (pelem))
    {
      brk ();
      /// Unlock reference counter from unchanged state
      release_remove (pelem);
    }

    /// Element empty, go down
    TS_GO_DOWN_RET (search_by_hash (pos, hash, pvalue) );
  }

  /// Lock element
  if (lock (pelem) <= 0)
  {
    /// Locked counter detected
    unlock (pelem);

    TS_GO_DOWN_RET (search_by_hash (pos, hash, pvalue) );
  }

  if (TS_LIVE_SIGN != pelem->status)
  {
    unlock (pelem);

    TS_GO_DOWN_RET (search_by_hash (pos, hash, pvalue) );
  }

  /// Element successfully locked
  if (pelem->hash == hash)
  {
    pvalue = pelem->pval;
    return true;
  }

  /// Hash doesn't equial, go down
  unlock (pelem);

  TS_GO_DOWN_RET (search_by_hash (pos, hash, pvalue));
}

/// Insert element in map & if successfull than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: set_at (mp& pos, Tkey key, Thash hash, const Tvalue* pvalue)
{
  if (!storage || !level_elems)
  { brk (); return false; }

  pos.map   = this;
  pos.elem  = get_elem (hash);
  pme pelem = & storage [pos.elem];

  long status = TS_BUSY_SIGN, retry = TS_SPINLOCK_COUNTER;

  for (; retry > 0; retry--)
  {
    if (TS_FREE_SIGN != pelem->status)
    {
      if (erase_dead (pelem) )
      {
	/// Try to lock free map element
	status = change_status (pelem, TS_BUSY_SIGN, TS_FREE_SIGN);

	if (TS_FREE_SIGN != status)
	{
	  brk (); ///< Bingo!!! - concurent set_at in progress!!!

	  /// Unlock reference counter from unchanged state
	  release_remove (pelem);

	  ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	  continue;
	}

	brk ();

	/// Lock element
	lock (pelem);

	/// Unlock reference counter from unchanged state
	release_remove (pelem);

	break;
      }
      else /// Collision detected
      if (TS_FREE_SIGN != pelem->status)
      {
	if (level == pmap_arch->levels)
	{ brk (); continue; }
	
	if (pelem->pmap) ///< Already used, go down
	  return pelem->pmap->set_at (pos, key, hash, pvalue);

	/// This is simple element, try replace on map
	multimap* map = new multimap (level + 1, pmap_arch);

	if (!map)
	{
	  brk ();
	  ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	  continue;
	}

	if (!map->set_at (pos, key, hash, pvalue) )
	{
	  brk ();
	  delete map;
	  ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	  continue;
	}

	/// Activate record
	if (atomic_compare_exchange ( (void**) & pelem->pmap, map, 0) )
	{
	  brk (); ///< Concurent map set_at detected
	  delete map;

	  if (pelem->pmap) ///< Already used, go down
	    return pelem->pmap->set_at (pos, key, hash, pvalue);
	  else
	  {
	    brk ();
	    ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	    continue;
	  } ///< may be only with removing submap
	}
	else
	  return true;
      }
    } ///< !FREE

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);

      ts_sleep (TS_SPINLOCK_SLEEP_TIME);
      continue;
    }

    /// Try to lock free map element
    status = change_status (pelem, TS_BUSY_SIGN, TS_FREE_SIGN);

    if (TS_FREE_SIGN == status)
      break;

    unlock (pelem);
  } ///< end for retry

  if (!retry)
  {
    if (pelem->pmap) ///< while used, go down
      return pelem->pmap->set_at (pos, key, hash, pvalue);
    else
      brk ();

    return false;
  }

  /// Allocate new map value
  Tvalue* pval = (Tvalue*) allocator.allocate (sizeof (*pval) );

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

  change_status (pelem, TS_LIVE_SIGN, TS_BUSY_SIGN);

  atomic_inc (& use_counter);
  return true;
}

/// Look for element in map by position
template <class Tkey, class Tvalue, class Thash, class Tallocator>
Tvalue* multimap <Tkey,     Tvalue,       Thash,       Tallocator>

:: lookup (mp& pos)
{
  if (!pos.map) { brk (); return 0; }

  if (this != (multimap*) pos.map)
    return ( (multimap*) pos.map)->lookup (pos);

  if (!( (multimap*) pos.map)->storage
   || pos.elem >= ( (multimap*) pos.map)->level_elems)
  { brk (); return 0; }

  return ( (multimap*) pos.map)->storage [pos.elem].pval;
}

/// Unlock position in map
template <class Tkey, class Tvalue, class Thash, class Tallocator>
void multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: release (mp& pos)
{
  if (!pos.map) { brk (); return; }

  if (this != (multimap*) pos.map)
  {
    ( (multimap*) pos.map)->release (pos);
    return;
  }

  if (!( (multimap*) pos.map)->storage
   || pos.elem >= ( (multimap*) pos.map)->level_elems)
  { brk (); return; }

  pme pelem = & ( (multimap*) pos.map)->storage [pos.elem];
  release (pelem);
}

/// Remove element from map and always unlock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove (mp& pos)
{
  if (!pos.map) { brk (); return false; }

  if (this != (multimap*) pos.map)
    return ( (multimap*) pos.map)->remove (pos);

  if (!( (multimap*) pos.map)->storage
    || pos.elem >= ( (multimap*) pos.map)->level_elems)
  { brk (); return false; }

  return remove ( & ( (multimap*) pos.map)->storage [pos.elem]);
}

/// Remove element from map on cleanup
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_dead (mp& pos)
{
  if (!pos.map) { brk (); return false; }

  if (this != (multimap*) pos.map)
    return ( (multimap*) pos.map)->remove_dead (pos);

  if (!( (multimap*) pos.map)->storage
    || pos.elem >= ( (multimap*) pos.map)->level_elems)
  { brk (); return false; }

  return remove_dead (& ( (multimap*) pos.map)->storage [pos.elem]);
}

/// Remove element from map by key
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_by_key (Tkey key)
{
  mp pos;
  Tvalue* pvalue;

  if (!lookup_by_key (pos, key, pvalue) )
    return false;

  return remove (pos);
}

/// Remove element from map by keys hash
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_by_key_hash (Tkey key)
{
  mp pos;
  Tvalue* pvalue;

  if (!lookup_by_key_hash (pos, key, pvalue) )
    return false;

  return remove (pos);
}

/// Remove element from map by hash
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_by_hash (Thash hash)
{
  mp pos;
  Tvalue* pvalue;

  if (!lookup_by_hash (pos, hash, pvalue) )
    return false;

  return remove (pos);
}

/// Doesn't thread safe method, it called from destructor
template <class Tkey, class Tvalue, class Thash, class Tallocator>
void multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_all_unsafe ()
{
  if (!storage || !level_elems)
  { brk (); return; }

  pme top_storage = storage + level_elems;

  /// Move to ahead of map array
  for (pme pelem = storage; pelem < top_storage; pelem++)
  {
    if (pelem->pmap)
    {
      pelem->pmap->remove_all_unsafe ();
      delete pelem->pmap, pelem->pmap = 0;
    }

    if (!pelem->pval) ///< BUSY && ERAS may be
      continue;

    pelem->pval-> ~Tvalue ();

    allocator.deallocate (pelem->pval), pelem->pval = 0;

    atomic_dec (& use_counter);
  }
}

template <class Tkey, class Tvalue, class Thash, class Tallocator>
void multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: remove_all ()
{
  if (!storage || !level_elems)
  { brk (); return; }

  pme top_storage = storage + level_elems;

  /// Move to ahead of map array
  for (pme pelem = storage; pelem < top_storage; pelem++)
  {
    if (pelem->pmap)
    {
      pelem->pmap->remove_all ();
      delete pelem->pmap, pelem->pmap = 0;
    }

    if (TS_LIVE_SIGN != pelem->status)
    {
      if (erase_dead (pelem) )
      {
	brk ();
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
      }

      continue;
    }

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);
      continue;
    }

    if (TS_LIVE_SIGN != pelem->status)
    {
      unlock (pelem);
      continue;
    }

    remove (pelem);
  }
}

/// Check and lock LIVE element else go down in to lower map
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: look_for_live_elem (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  if (!storage) { brk (); return false; }

  /// Look for LIVE element or not empty low map
  for (; pos.elem < level_elems; pos.cnt++, pos.elem++)
  {
    register pme pelem = & storage [pos.elem];

    if (pelem->pmap)
    {/// Store current location
      pos.p [level].map  = pos.map;
      pos.p [level].elem = pos.elem;

      if (pelem->pmap->start (pos, key, hash, pvalue) )
	return true;
    }

    if (TS_LIVE_SIGN != pelem->status)
    {
      if (erase_dead (pelem) )
      {
	/// Unlock reference counter from unchanged state
	release_remove (pelem);
      }

      continue;
    }

    /// Lock element
    if (lock (pelem) <= 0)
    {
      /// Locked counter detected
      unlock (pelem);
      continue;
    }

    if (TS_LIVE_SIGN != pelem->status)
    {
      unlock (pelem);
      continue;
    }

    key    = pelem->key;
    hash   = pelem->hash;
    pvalue = pelem->pval;
    return true;
  } ///< End while pos.elem <= level_elems

  if (!level)
    return false;

  /// End current map, return to previous map element
  pos.map  = pos.p [level - 1].map;
  pos.elem = pos.p [level - 1].elem;
  return false;
}

/// Next maps enumerating & if failure than unlock last element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: next (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  if (!pos.map) { brk (); return false; }

  /// Go to current map
  if (this != pos.map)
  {
   //  return ( (multimap*) pos.map)->next (pos, key, hash, pvalue);
    if ( ( (multimap*) pos.map)->next (pos, key, hash, pvalue) )
      return true;

    if (this != pos.map) /// swim up
      return false;
  }
  else
  {
    /// Process previous element
    register pme pelem = & storage [pos.elem];
    unlock (pelem);
  }

  /// Go to new next map element
  pos.cnt++, pos.elem++;

  return look_for_live_elem (pos, key, hash, pvalue);
}

/// Begin maps enumerating & if successfull than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: start (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  if (!pmap_arch) { brk (); return false; }

  if (!level
   && !pos.enum_init (pmap_arch->levels) )
    return false;

  pos.map  = this;
  pos.elem = 0;
  return look_for_live_elem (pos, key, hash, pvalue);
}

/// Next maps enumerating by hash & if failure than unlock last element
template <class Tkey, class Tvalue, class Thash, class Tallocator>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator>

:: next (mp& pos, Thash hash, Tvalue*& pvalue)
{
  if (!pos.map) { brk (); return false; }

  ( (multimap*) pos.map)->unlock (pos);

  /// Go on next map
  multimap* next_map = ( (multimap*) pos.map)->get_next_map (hash);

  if (next_map)
    return next_map->search_by_hash (pos, hash, pvalue);
  else
    return false;
}

}; /* end of nbmap namespace */

}; /* end of tstl namespace */

#endif /* __NBMAP_HPP__ */
