/*****************************************************************************************************//**
 *
 *  Module Name:	\file iqalloccache.hpp
 *
 *  Abstract:		\brief Interlocked queue based memory àllocating ñache.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: atomic_inc_return, atomic_dec_return, allocator
 *  Internal: ialloc_cache
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __IQALLOCCACHE_HPP__
#define __IQALLOCCACHE_HPP__

#include "tstl.hpp"

#define TS_MAX_TRY_COUNTER TS_SPINLOCK_COUNTER
#define TS_ALLOC_INDEX_SIZE   (sizeof (short) * 8)
#define TS_UNUSED_ALLOC_INDEX ( (1 << TS_ALLOC_INDEX_SIZE) - 1)

namespace tstl {

template <class Tvalue = char, class Tallocator = allocator, class Taux_allocator = Tallocator>

/// dynamic memory allocating cache
class iqalloc_cache
{
  Taux_allocator aux_allocator;
  Tallocator         allocator;

  #include <pshpack2.h>

  typedef struct queue_pos
  {
    volatile long index        : TS_ALLOC_INDEX_SIZE;
    volatile long deep_counter : (sizeof (long) * 8) - TS_ALLOC_INDEX_SIZE;
  } qp, *pqp;

  #include <poppack.h>

  typedef struct queue_elem { Tvalue* buffer; queue_pos next; } qe, *pqe;

  qe* ref_storage;   ///< free buffers queue storage
  Tvalue* storage;   ///< storage of buffers

  queue_elem *volatile head;
  queue_pos tail;

  const bool dont_use_global_mempool; /// allocating behaviour
  const size_t buffer_size;

  long max_elem;     ///< storage elements number
  long use_counter;  ///< using counter
  long try_counter;  ///< tail update counter

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

   iqalloc_cache (const size_t in_buffer_size = 0x400,
                  const long num_elem       = TS_MAX_TRY_COUNTER,
                  const long in_try_counter = TS_MAX_TRY_COUNTER,
                  const bool in_dont_use_global_mempool = false);

  ~iqalloc_cache ()
  {
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
iqalloc_cache  <Tvalue,       Tallocator,       Taux_allocator>

:: iqalloc_cache (const size_t in_buffer_size = 0x400,
                  const long num_elem       = TS_MAX_TRY_COUNTER,
                  const long in_try_counter = TS_MAX_TRY_COUNTER,
                  const bool in_dont_use_global_mempool = false)
                : head (0), buffer_size (in_buffer_size), max_elem (num_elem),
                  dont_use_global_mempool (in_dont_use_global_mempool),
                  use_counter (0), try_counter (in_try_counter)
{
  if (max_elem >= TS_UNUSED_ALLOC_INDEX) { brk (); return; } ///< till it's limit of alloc cache

  ref_storage = (qe*) aux_allocator.allocate (sizeof (*ref_storage) * max_elem);

  if (!ref_storage) { brk (); return; }

  memset (ref_storage, 0, sizeof (*ref_storage) * max_elem);

  storage = (Tvalue*) allocator.allocate (sizeof (*storage) * buffer_size * max_elem);

  if (!storage)
  { brk (); aux_allocator.deallocate (ref_storage), ref_storage = 0; return; }

  Tvalue* p = storage;

  for (long i = 0; i < max_elem; ++i, p += buffer_size)
  {
    ref_storage [i].buffer = p;
    ref_storage [i].next.index = i + 1;
    ref_storage [i].next.deep_counter = 0;
  }

  /// init ends
  ref_storage [max_elem - 1].next.index = TS_UNUSED_ALLOC_INDEX;

  head = & ref_storage [max_elem - 1];

  tail.index = 0;
  tail.deep_counter = 0;
}

/// get buffer
/** \param size is amount of symbols <Tvalue> in buffer */
template <class Tvalue,  class Tallocator, class Taux_allocator>
Tvalue* iqalloc_cache <Tvalue, Tallocator,       Taux_allocator>

:: get (const size_t size)
{
  if (!size)
  { brk (); return 0; }

  if (!ref_storage || !storage || !max_elem || !buffer_size)
  { brk (); return get_from_global_mempool (size); }

  if (size > buffer_size)
    return get_from_global_mempool (size);

  queue_pos prev_tail = tail;
  register long tail_index = TS_UNUSED_ALLOC_INDEX;

  /// try to concurently get tail block
  long i = 0;

  for (; i < try_counter; i++, prev_tail = tail)
  {
    tail_index = prev_tail.index & TS_UNUSED_ALLOC_INDEX;

    if (tail_index == TS_UNUSED_ALLOC_INDEX
     || tail_index >= max_elem)
    {
      brk (); ///< tail already uses actual index
      return get_from_global_mempool (size);
    }

    register long next_index = ref_storage [tail_index].next.index & TS_UNUSED_ALLOC_INDEX;

    if (next_index == TS_UNUSED_ALLOC_INDEX) /// empty head element detected
      return get_from_global_mempool (size);

    if (next_index >= max_elem)
    {
      brk (); ///< bad index detected
      return get_from_global_mempool (size);
    }

    /// deep counter needs for protection tail->next.
    /// if tail content(next) with same index or pointer 
    /// was replaced till tail interlocked changing, 
    /// we won't replace a tail, because tail + deep != tail + deep + x

    /// init new tail
    queue_pos new_tail;
    new_tail.index = next_index;
    new_tail.deep_counter = prev_tail.deep_counter + 1; ///< new_tail = next_index | ( (deep_counter + 1) << 0x10)

    /// swap tail
//    if ( ( (long) prev_tail) == atomic_compare_exchange ( (long*) & tail, (long) new_tail, (long) prev_tail) )
    if (* ( (long*) (& prev_tail) ) == atomic_compare_exchange ( (long*) & tail, * ( (long*) (& new_tail) ), * ( (long*) (& prev_tail) ) ) )
      break;
  }

  if (i >= try_counter)
  {
    brk (); ///< couldn't update tile try_counter times
    return get_from_global_mempool (size);
  }

  if (TS_UNUSED_ALLOC_INDEX == tail_index)
  {
    brk (); ///< tail_index wasn't setuped
    return get_from_global_mempool (size);
  }

  atomic_inc (& use_counter);

  ref_storage [tail_index].next.index = TS_UNUSED_ALLOC_INDEX; /// tail element freed from list

  if (!ref_storage [tail_index].buffer) { brk (); /* ref_storage corrupted */ }

  return ref_storage [tail_index].buffer;
}

template <class Tvalue, class Tallocator, class Taux_allocator>
bool iqalloc_cache <Tvalue,   Tallocator,       Taux_allocator>

:: revert (Tvalue* buffer)
{
  if (!buffer || !buffer_size || !ref_storage || !storage || !max_elem)
  { brk (); return false; }

  if (!is_address_from_cache (buffer) )
  { brk (); revert_to_global_mempool (buffer); return false; }

  /// calculate buffer index
  size_t index = (buffer - storage) / buffer_size;

  if (index < 0 || index >= (size_t) max_elem)
  { brk (); revert_to_global_mempool (buffer); return false; }

  /// prepare new head structure
  qe* pe = & ref_storage [index];

  pe->next.index = TS_UNUSED_ALLOC_INDEX;

  atomic_dec (& use_counter);

  /// swap head structure pointer
  pqe prev = (pqe) atomic_exchange ( (void**) & head, pe); ///< swap head

  prev->next.index = index;

  return true;
}

}; /* end of tstl namespace */

#endif /* __IQALLOCCACHE_HPP__ */
