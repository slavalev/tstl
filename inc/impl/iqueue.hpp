/*****************************************************************************************************//**
 *
 *  Module Name:	\file iqueue.hpp
 *
 *  Abstract:		\brief Interlocked queue manager.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 13.08.2007 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: alloc_cache, melocker (relocker), allocator
 *  Internal: iqueue
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __IQUEUE_HPP__
#define __IQUEUE_HPP__

#include "alloccache.hpp"

namespace tstl {

template <class Tvalue = size_t, class Tlocker = melocker<>, class Tallocator = allocator,
          class Talloc_cache = iqalloc_cache <char, Tallocator, Tallocator> >

class iqueue
{
  const bool many_readers;

  typedef struct queue_elem { queue_elem* volatile next; Tvalue value; } qe, *pqe;

  queue_elem *volatile head, *volatile tail, *volatile next;

  Talloc_cache* palloc_cache;

  Tallocator allocator;

  Tlocker tail_locker;

  long use_counter;

public:

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  iqueue (const long alloc_cache_elem = 64, const bool in_many_readers = false)
        : head ( (pqe) & next), tail ( (pqe) & next), next (0), palloc_cache (0),
          many_readers (in_many_readers), use_counter (0)
  {
    palloc_cache = new Talloc_cache (sizeof (qe), alloc_cache_elem);
    if (!palloc_cache) { brk (); }
  }

  ~iqueue ()
  {
    Tvalue buffer;
    while (get (& buffer) ) {}

    if (palloc_cache) { delete palloc_cache, palloc_cache = 0; } else { brk (); }
  }

  /// Is not thread safe method
  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about cache using
  long get_stat () const
  { return use_counter; }

  /// Stores Tvalues.
  /** \param[in] buffer is pointer to Tvalues
    * \return true if Tvalues put into pipe. */
  bool put (Tvalue* buffer);

  /// Retrives Tvalues.
  /** \param[in] buffer is pointer to Tvalues
    * \return true if Tvalues get from pipe. */
  bool get (Tvalue* buffer);
};

/// Stores Tvalues.
/** \param[in] buffer is pointer to Tvalues
  * \return true if Tvalues put into pipe. */
template <class Tvalue, class Tlocker, class Tallocator, class Talloc_cache>
bool iqueue    <Tvalue,       Tlocker,       Tallocator,       Talloc_cache>

:: put (Tvalue* buffer)
{
  if (!buffer) { brk (); return false; }

  /// get piece of memory
  pqe pe = palloc_cache ? (pqe) palloc_cache->get  (sizeof (*pe) )
                        : (pqe) allocator.allocate (sizeof (*pe) );
  if (!pe)
  { brk (); return false; }

  /// put value to temporary buffer
  tstl :: allocator a;
  :: new ( (void*) & pe->value, a) Tvalue (*buffer);

  pe->next = 0;

  atomic_inc (& use_counter);

  /// insert to head of list
  pqe prev = (pqe) atomic_exchange ( (void**) & head, pe); ///< swap head

  prev->next = pe;

  return true;
}

/// Retrives Tvalues.
/** \param[in] buffer is pointer to Tvalues
  * \return true if Tvalues get from pipe. */
template <class Tvalue, class Tlocker, class Tallocator, class Talloc_cache>
bool iqueue    <Tvalue,       Tlocker,       Tallocator,       Talloc_cache>

:: get (Tvalue* buffer)
{
  if (!buffer || !tail) { brk (); return false; }

  /// remove tail from list
  if (many_readers) tail_locker.lock ();

  if (!tail->next)
  { if (many_readers) tail_locker.unlock (); return false; }

  pqe prev_tail = tail;
  tail = tail->next; ///< swap tail

  /// copy value from temporary buffer
  tstl :: allocator a;
  :: new ( (void*) buffer, a) Tvalue (tail->value);

  if (many_readers) tail_locker.unlock ();

  atomic_dec (& use_counter);

  if (prev_tail == (pqe) & next)
    return false;    ///< skip first empty node

  /// free temporary buffer
  if (palloc_cache)
    palloc_cache->revert ( (char*) prev_tail), prev_tail = 0;
  else
    allocator.deallocate ( (char*) prev_tail), prev_tail = 0;

  return true;
}

}; /* end of tstl namespace */

#endif /* __IQUEUE_HPP__ */
