/*****************************************************************************************************//**
 *
 *  Module Name:	\file ialloccache.hpp
 *
 *  Abstract:		\brief Interlocked operation based memory allocating cache.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: atomic_inc_return, atomic_dec_return, allocator
 *  Internal: alloc_cache
 *
 *  TODO:		\todo replace algorithm with interlocked queue with only one interlocked operation
 *
 *********************************************************************************************************/

#ifndef __IALLOCCACHE_HPP__
#define __IALLOCCACHE_HPP__

#include "tstl.hpp"

#define TS_MAX_SEARCH_DEPTH TS_SPINLOCK_COUNTER

namespace tstl {

template <class Tvalue = char, class Tallocator = allocator, class Taux_allocator = Tallocator>

/// allocating cache
class ialloc_cache
{
  Tvalue   *storage; ///< storage of buffers
  long *ref_storage; ///< cache control storage

  long max_elem;     ///< storage elements number
  long alloc_elem;	 ///< current ready element
  long use_counter;  ///< using counter
  long search_depth; ///< searching depth

  const size_t buffer_size;
  const bool dont_use_global_mempool; /// allocating behaviour

  Taux_allocator aux_allocator;
      Tallocator     allocator;

  Tvalue* get_from_global_mempool (const size_t size)
  {
    if (dont_use_global_mempool)
      return 0;

    return (Tvalue*) allocator.allocate (sizeof (Tvalue) * size);
  }

  bool revert_to_global_mempool (Tvalue* buffer)
  {
    if (!buffer) { brk (); return false; }

    if (dont_use_global_mempool)
      return false;

	allocator.deallocate ( (char*) buffer), buffer = 0;
    return true;
  }

public:

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

   ialloc_cache (const size_t in_buffer_size = 0x400,
                 const long num_elem         = TS_MAX_SEARCH_DEPTH,
                 const long in_search_depth  = TS_MAX_SEARCH_DEPTH,
                 const bool in_dont_use_global_mempool = false);

  ~ialloc_cache ()
  {
    max_elem = 0;

    if (ref_storage) aux_allocator.deallocate (ref_storage), ref_storage = 0;
    if (storage)         allocator.deallocate     (storage),     storage = 0;
  }

  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about cache using
  long get_stat () const
  { return use_counter; }

  bool is_address_from_cache (Tvalue* buffer) const
  {
    if (!buffer || !ref_storage || !storage || !max_elem)
    { brk (); return false; }

    return buffer >= storage
        && buffer < (storage + (buffer_size * max_elem) );
  }

  bool is_size_enough (const size_t size) const
  { return buffer_size <= size; }

  /** \param size is amount of symbols <Tvalue> in buffer */
  Tvalue* get (const size_t size = buffer_size);

  bool revert (Tvalue* buffer);
};

template <class Tvalue, class Tallocator, class Taux_allocator>
ialloc_cache   <Tvalue,       Tallocator,       Taux_allocator>

:: ialloc_cache (const size_t in_buffer_size,
                 const long num_elem        = TS_MAX_SEARCH_DEPTH,
                 const long in_search_depth = TS_MAX_SEARCH_DEPTH,
                 const bool in_dont_use_global_mempool = false)
               : max_elem (num_elem), alloc_elem (0),
                 use_counter (0), buffer_size (in_buffer_size),
                 dont_use_global_mempool (in_dont_use_global_mempool)
{
  ref_storage = (long*) aux_allocator.allocate (sizeof (*ref_storage) * max_elem);

  if (!ref_storage) { brk (); return; }

  memset (ref_storage, 0, sizeof (*ref_storage) * max_elem);

  storage = (Tvalue*) allocator.allocate (sizeof (*storage) * buffer_size * max_elem);

  if (!storage)
  { brk (); aux_allocator.deallocate (ref_storage), ref_storage = 0; return; }

  search_depth = max_elem < in_search_depth ? max_elem : in_search_depth;
}

/// get buffer
/** \param size is amount of symbols <Tvalue> in buffer */
template <class Tvalue,     class Tallocator, class Taux_allocator>
Tvalue* ialloc_cache <Tvalue,     Tallocator,       Taux_allocator>

:: get (const size_t size)
{
  if (!size)
  { brk (); return 0; }

  if (!ref_storage || !storage || !max_elem || !buffer_size)
  { brk (); return get_from_global_mempool (size); }

  if (size > buffer_size
   || use_counter == max_elem) /// cache is fully used
    return get_from_global_mempool (size);

  long try_counter = 0;

  for (long ready_element = (atomic_inc_return (& alloc_elem) - 1) % max_elem;
            try_counter++ < search_depth;
            ready_element = (atomic_inc_return (& alloc_elem) - 1) % max_elem)
  {
    if (ref_storage [ready_element] ) continue; /// small optimization
    if (use_counter == max_elem)      break;    /// second small optimization

    if (1 == atomic_inc_return (& ref_storage [ready_element] ) ) /// we're first
    { ///< we locked a cache element
      atomic_inc (& use_counter);
      return & storage [ready_element * buffer_size];
    }
  }

  return get_from_global_mempool (size);
}

template <class Tvalue,   class Tallocator, class Taux_allocator>
bool ialloc_cache <Tvalue,      Tallocator,       Taux_allocator>

:: revert (Tvalue* buffer)
{
  if (!buffer || !buffer_size || !ref_storage || !storage || !max_elem)
  { brk (); return false; }

  if (!is_address_from_cache (buffer) )
  {
    if (dont_use_global_mempool)
    { brk (); /* it's alien pointer */ }

    revert_to_global_mempool (buffer);
    return false;
  }

  /// calculate buffer index
  size_t index = (buffer - storage) / buffer_size;

  if (index < 0 || index >= (size_t) max_elem)
  {
    brk (); /* very strange breakpoint... */
    revert_to_global_mempool (buffer);
    return false;
  }

  /// free buffer counter
  if (!atomic_dec_return (& ref_storage [index] ) )
    alloc_elem = index;
  else
  { brk (); }

  atomic_dec (& use_counter);

  return true;
}

}; /* end of tstl namespace */

#endif /* __IALLOCCACHE_HPP__ */
