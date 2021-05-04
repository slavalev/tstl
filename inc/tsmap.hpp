/*****************************************************************************************************//**
 *
 *  Module Name:	\file tsmap.hpp
 *
 *  Abstract:		\brief Common include file for thread safe multimmap templates.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External:	hash_key, ts_sleep, nbmap, pbmap, allocator
 *  Internal:	multimap, mp (map_pos)
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSMAP_HPP__
#define __TSMAP_HPP__

#include "impl/pbmap.hpp" ///< Hash table based map template.
#include "impl/nbmap.hpp" ///< B-tree based map template.

namespace tstl {

#  if defined (PART_LOCKED_MAP)
typedef pbmap :: mp mp;

template <class Tkey, class Tvalue, class Thash = size_t, class Tallocator = allocator,
          class Tmultimap = pbmap :: multimap <Tkey, Tvalue, Thash, Tallocator> >
#  else
typedef nbmap :: mp mp;

template <class Tkey, class Tvalue, class Thash = size_t, class Tallocator = allocator, 
          class Tmultimap = nbmap :: multimap <Tkey, Tvalue, Thash, Tallocator> >
#  endif
struct multimap : Tmultimap
{
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  multimap (const unsigned long root_array_elems = 32) : Tmultimap (root_array_elems) {}

  /// Doesn't thread safe method
  bool is_empty ()
  { return Tmultimap :: is_empty (); }

  /// Get statistic about map using
  long get_stat ()
  { return Tmultimap :: get_stat (); }

  /// Get statistic about using map element
  long get_stat (Thash map_elem)
  { return Tmultimap :: get_stat (map_elem); }

  /// Get hash by key
  Thash hash (Tkey key)
  { return Tmultimap :: hash (key); }

  /// Insert element in map & if successfull than lock element
  bool set_at (mp& pos, Tkey key, Thash hash, const Tvalue* pvalue)
  { return Tmultimap :: set_at (pos, key, hash, pvalue); }

  /// Insert element in map & if successfull than lock element
  bool set_at (mp& pos, Tkey key, const Tvalue* pvalue)
  { return Tmultimap :: set_at (pos, key, pvalue); }

  /// Look for element in map by key & if successfull search than lock element
  bool lookup_by_key (mp& pos, Tkey key, Tvalue*& pvalue)
  { return Tmultimap :: lookup_by_key (pos, key, pvalue); }

  /// Look for element in map by keys hash & if successfull search than lock element
  bool lookup_by_key_hash (mp& pos, Tkey key, Tvalue*& pvalue)
  { return Tmultimap :: lookup_by_key_hash (pos, key, pvalue); }

  /// Look for element in map by hash & if successfull search than lock element
  bool lookup_by_hash (mp& pos, Thash hash, Tvalue*& pvalue)
  { return Tmultimap :: lookup_by_hash (pos, hash, pvalue); }

  /// Look for element in map by position
  Tvalue* lookup (mp& pos)
  { return Tmultimap :: lookup (pos); }

  /// Unlock position in map
  void release (mp& pos)
  { Tmultimap :: release (pos); }

  /// Synoname of release
  void unlock (mp& pos)
  { Tmultimap :: unlock (pos); }

  /// Remove element from map and always unlock element
  bool remove (mp& pos)
  { return Tmultimap :: remove (pos); }

  /// Remove element from map on cleanup
  bool remove_dead (mp& pos)
  { return Tmultimap :: remove_dead (pos); }

  /// Remove element from map by key
  bool remove_by_key (Tkey key)
  { return Tmultimap :: remove_by_key (key); }

  /// Remove element from map by keys hash
  bool remove_by_key_hash (Tkey key)
  { return Tmultimap :: remove_by_key_hash (key); }

  /// Remove element from map by hash
  bool remove_by_hash (Thash hash)
  { return Tmultimap :: remove_by_hash (hash); }

  /// Remove all elements
  void remove_all ()
  { Tmultimap :: remove_all (); }

  /// Next maps enumerating & if failure than unlock last element
  bool next  (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
  { return Tmultimap :: next (pos, key, hash, pvalue); }

  /// Begin maps enumerating & if successfull than lock element
  bool start (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
  { return Tmultimap :: start (pos, key, hash, pvalue); }

  /// Begin maps enumerating by hash & if successfull than lock element
  bool start (mp& pos, Thash hash, Tvalue*& pvalue)
  { return Tmultimap :: start (pos, hash, pvalue); }

  /// Next maps enumerating by hash & if failure than unlock last element
  bool next  (mp& pos, Thash hash, Tvalue*& pvalue)
  { return Tmultimap :: next (pos, hash, pvalue); }
};

template <class Tmap>
static bool init_map (Tmap*& pm, const int root_elements)
{
  pm = new Tmap (root_elements);
  return 0 != pm;
};

}; /* end of tstl namespace */

#endif /* __TSMAP_HPP__ */
