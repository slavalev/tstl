/*****************************************************************************************************//**
 *
 *  Module Name:	\file alloccache.hpp
 *
 *  Abstract:		\brief Interlocked operation based memory allocating cache.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: atomic_inc_return, atomic_dec_return, allocator, ialloc_cache iqalloc_cache
 *  Internal: alloc_cache
 *
 *  TODO:		\todo replace algorithm with interlocked queue with only one interlocked operation
 *
 *********************************************************************************************************/

#ifndef __ALLOCCACHE_HPP__
#define __ALLOCCACHE_HPP__

#include "impl/ialloccache.hpp"
#include "impl/iqalloccache.hpp"

#define TS_ALLOC_CACHE_BUFFER_SIZE 0x400

namespace tstl {

template <class Tvalue = char, class Tallocator = allocator, class Taux_allocator = Tallocator,
          class Talloc_cache = iqalloc_cache  <Tvalue, Tallocator, Taux_allocator> >

/// allocating cache
struct alloc_cache : Talloc_cache
{
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

   alloc_cache (const size_t buffer_size = TS_ALLOC_CACHE_BUFFER_SIZE,
                const long max_elem      = TS_MAX_SEARCH_DEPTH,
                const long search_depth  = TS_MAX_SEARCH_DEPTH,
                const bool dont_use_global_mempool = false)
              : Talloc_cache (buffer_size, max_elem, search_depth, dont_use_global_mempool) {}

  bool is_empty () const
  { return Talloc_cache :: is_empty (); }

  /// Get statistic about cache using
  long get_stat () const
  { return Talloc_cache :: get_stat (); }

  bool is_address_from_cache (Tvalue* buffer) const
  { return Talloc_cache :: is_address_from_cache (buffer); }

  bool is_size_enough (const size_t size) const
  { return Talloc_cache :: is_size_enough (size); }

  /** \param size is amount of symbols <Tvalue> in buffer */
  Tvalue* get (const size_t size)
  { return Talloc_cache :: get (size); }

  bool revert (Tvalue* buffer)
  { return Talloc_cache :: revert (buffer); }
};

template <class Tvalue = char, class Tallocator = allocator, class Taux_allocator = Tallocator,
          class Talloc_cache = iqalloc_cache  <Tvalue, Tallocator, Taux_allocator> >

/// allocating cache array
class alloc_cache_array
{
  Talloc_cache* array;
  long    use_counter;  ///< using counter
  const long array_size;
  const bool dont_use_global_mempool; /// allocating behaviour

  Tallocator allocator;

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

  alloc_cache_array (const size_t begin_buffer_size = TS_ALLOC_CACHE_BUFFER_SIZE,
                     const long begin_elem_number = TS_MAX_SEARCH_DEPTH,
                     const long search_depth      = TS_MAX_SEARCH_DEPTH,
                     const bool in_dont_use_global_mempool = false,
                     const long in_array_size = 6,
                     const long buffer_size_increment = TS_ALLOC_CACHE_BUFFER_SIZE,
                     const long elem_number_power_decrement = 1);

  ~alloc_cache_array ()
  {
    if (!array || !array_size) { brk (); return; }

    for (long cnt = 0; cnt < array_size; cnt++)
    {
      if (!array [cnt]) { brk (); continue; }
      delete (array [cnt]), array [cnt] = 0;
    }
  }

  bool is_empty () const
  {
    bool is_used = false;

    for (long cnt = 0; cnt < array_size; cnt++)
    {
      if (!array [cnt]) continue;
      is_used |= !array [cnt]->is_empty ();
    }

    return !is_used;
  }

  /// Get statistic about cache using
  long get_stat (long array_index = -1) const;

  bool is_address_from_cache (Tvalue* buffer) const
  {
    for (long cnt = 0; cnt < array_size; cnt++)
    {
      if (!array [cnt]) continue;
      if (array [cnt]->is_address_from_cache (buffer) )
        return true;
    }

    return false;
  }

  bool is_size_enough (const size_t size) const
  {
    for (long cnt = 0; cnt < array_size; cnt++)
    {
      if (!array [cnt]) continue;
      if (array [cnt]->is_size_enough (size) )
        return true;
    }

    return fal se;
  }

  /** \param size is amount of symbols <Tvalue> in buffer */
  Tvalue* get (const size_t size);

  bool revert (Tvalue* buffer);
};

template <class Tvalue, class Tallocator, class Taux_allocator, class Talloc_cache>
alloc_cache_array <Tvalue,    Tallocator,       Taux_allocator,       Talloc_cache>

:: alloc_cache_array (const size_t begin_buffer_size = TS_ALLOC_CACHE_BUFFER_SIZE,
                      const long begin_elem_number = TS_MAX_SEARCH_DEPTH,
                      const long search_depth      = TS_MAX_SEARCH_DEPTH,
                      const bool in_dont_use_global_mempool = false,
                      const long in_array_size = 6,
                      const long buffer_size_increment = TS_ALLOC_CACHE_BUFFER_SIZE,
                      const long elem_number_power_decrement = 1)

                    : array (0), use_counter (0), array_size (in_array_size),
                      dont_use_global_mempool (in_dont_use_global_mempool)
{
  array = allocator.allocate (array_size * sizeof (*array) );
  if (!array) { brk (); return; }

  memset (array, 0, array_size * sizeof (*array) );

  size_t buffer_size = begin_buffer_size;
  long   max_elem    = begin_elem_number;

  for (long cnt = 0; cnt < array_size; cnt++,
       buffer_size += buffer_size_increment,
       max_elem   >>= elem_number_power_decrement)
  {
    array [cnt] = new Talloc_cache (buffer_size, max_elem, search_depth, true);
    if (!array [cnt]) { brk (); }
  }
}

/// Get statistic about cache using
template <class Tvalue, class Tallocator, class Taux_allocator, class Talloc_cache>
long alloc_cache_array <Tvalue, Tallocator,     Taux_allocator,       Talloc_cache>

:: get_stat (long array_index = -1) const
{
  if (-1 != array_index
   && array_index < array_size)
   return array [array_index]->get_stat ();

  long used = 0;

  for (long cnt = 0; cnt < array_size; cnt++)
  {
    if (!array [cnt]) continue;
    used += array [cnt]->get_stat ();
  }

  return used;
}

template <class Tvalue, class Tallocator, class Taux_allocator, class Talloc_cache>
Tvalue* alloc_cache_array <Tvalue, Tallocator,  Taux_allocator,       Talloc_cache>

:: get (const size_t size)
{
  long cnt = 0;

  for (; cnt < array_size; cnt++)
  {
    if (!array [cnt]) continue;
    if (array [cnt]->is_size_enough (size) )
      break;
  }

  if (cnt >= array_size)
    return get_from_global_mempool (size);

  for (try_count = 0; cnt < array_size && try_count < 3; cnt++, try_count++)
  {
    if (!array [cnt]) continue;

    Tvalue* pvalue = array [cnt]->get (size);

    if (!pvalue) continue;

    atomic_inc (& use_counter);
    return pvalue;
  }

  return get_from_global_mempool (size);
}

template <class Tvalue, class Tallocator, class Taux_allocator, class Talloc_cache>
bool alloc_cache_array <Tvalue, Tallocator,     Taux_allocator,       Talloc_cache>

:: revert (Tvalue* buffer)
{
  long cnt = 0;

  for (; cnt < array_size; cnt++)
  {
    if (!array [cnt]) continue;
    if (array [cnt]->is_address_from_cache (buffer) )
      break;
  }

  if (cnt >= array_size)
  {
    if (dont_use_global_mempool)
    { brk (); /* it's alien pointer */ }

    revert_to_global_mempool (buffer);
    return false;
  }

  bool rc = array [cnt]->revert (buffer);

  if (rc) atomic_dec (& use_counter);

  return rc;
}

}; /* end of tstl namespace */

#endif /* __ALLOCCACHE_HPP__ */
