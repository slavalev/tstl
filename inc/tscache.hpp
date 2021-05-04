/*****************************************************************************************************//**
 *
 *  Module Name:	\file tscache.hpp
 *
 *  Abstract:		\brief Common include file for thread safe generic cache.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 07.08.2009 started
 *
 *  Classes, methods and structures: \details
 *
 *  External:	limitcache, timercache, allocator, melocker (relocker), multimap, mp (map_pos)
 *  Internal:	cache
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSCACHE_HPP__
#define __TSCACHE_HPP__

#include "impl/limitcache.hpp" ///< Cache template with policy based on limitation.
#include "impl/timercache.hpp" ///< Cache template with policy based on time of life.

namespace tstl {

#  if defined (TIMER_CACHE)
template <class Tkey, class Tvalue, class Thash = size_t,
          class Tallocator = allocator, class Tpos = long,
          class Tcache = timer_cache <Tkey, Tvalue, Thash, Tallocator> >
#  else
template <class Tkey, class Tvalue, class Thash = size_t,
          class Tallocator = allocator, class Tlocker = melocker<>, class Tpos = nbmap :: mp,
          class Tcache = limit_cache <Tkey, Tvalue, Thash, Tlocker, Tallocator,
          nbmap :: multimap <Tkey, Tvalue, Thash, Tallocator>, Tpos> >
#  endif
struct cache : Tcache
{
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  cache (const long num_elems = 32)
  : Tcache (num_elems) {}

  cache (const ulonglong timeout, const long num_elems = 32)
  : Tcache (num_elems, timeout) {}

  /// Doesn't thread safe method
  bool is_empty () const
  { return Tcache :: is_empty (); }

  /// Get statistic about map using
  long get_stat () const
  { return Tcache :: get_stat (); }

  /// Get hash by key
  Thash hash (Tkey key) const
  { return Tcache :: hash (key);  }

  /// Insert element in map & if successfull than lock element
  bool set_at (Tpos& pos, Tkey key, Thash hash, const Tvalue* pvalue)
  { return Tcache :: set_at (pos, key, hash, pvalue); }

  /// Insert element in map & if successfull than lock element
  bool set_at (Tpos& pos, Tkey key, const Tvalue* pvalue)
  { return Tcache :: set_at (pos, key, pvalue); }

  /// Look for element in map by key & if successfull search than lock element
  bool lookup_by_key (Tpos& pos, Tkey key, Tvalue*& pvalue)
  { return Tcache :: lookup_by_key (pos, key, pvalue); }

  /// Look for element in map by keys hash & if successfull search than lock element
  bool lookup_by_key_hash (Tpos& pos, Tkey key, Tvalue*& pvalue)
  { return Tcache :: lookup_by_key_hash (pos, key, pvalue); }

  /// Look for element in map by hash & if successfull search than lock element
  bool lookup_by_hash (Tpos& pos, Thash hash, Tvalue*& pvalue)
  { return Tcache :: lookup_by_hash (pos, hash, pvalue); }

  /// Look for element in map by position
  Tvalue* lookup (Tpos& pos) const
  { return Tcache :: lookup (pos); }

  /// Unlock position in map
  void release (Tpos& pos)
  { Tcache :: release (pos); }

  /// Synoname of release
  void unlock (Tpos& pos)
  { Tcache :: unlock (pos); }

  /// Remove element from map and always unlock element
  bool remove (Tpos& pos)
  { return Tcache :: remove (pos); }

  /// Remove element from map on cleanup
  bool remove_dead (Tpos& pos)
  { return Tcache :: remove_dead (pos); }

  /// Remove element from map by key
  bool remove_by_key (Tkey key)
  { return Tcache :: remove_by_key (key); }

  /// Remove element from map by keys hash
  bool remove_by_key_hash (Tkey key)
  { return Tcache :: remove_by_key_hash (key); }

  /// Remove element from map by hash
  bool remove_by_hash (Thash hash)
  { return Tcache :: remove_by_hash (hash); }

  /// Remove all elements
  void remove_all ()
  { Tcache :: remove_all (); }

  /// Next maps enumerating & if failure than unlock last element
  bool next  (Tpos& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
  { return Tcache :: next (Tpos, key, hash, pvalue); }

  /// Begin maps enumerating & if successfull than lock element
  bool start (Tpos& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
  { return Tcache :: start (pos, key, hash, pvalue); }

  /// Begin maps enumerating by hash & if successfull than lock element
  bool start (Tpos& pos, Thash hash, Tvalue*& pvalue)
  { return Tcache :: start (pos, hash, pvalue); }

  /// Next maps enumerating by hash & if failure than unlock last element
  bool next  (Tpos& pos, Thash hash, Tvalue*& pvalue)
  { return Tcache :: next (pos, hash, pvalue); }
};

template <class Tcache>
static bool init_cache (Tcache*& pc, const int root_elements)
{
  pc = new Tcache (root_elements);
  return 0 != pc;
};

}; /* end of tstl namespace */

#endif /* __TSCACHE_HPP__ */
