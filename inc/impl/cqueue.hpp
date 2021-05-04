/*****************************************************************************************************//**
 *
 *  Module Name:	\file cqueue.hpp
 *
 *  Abstract:		\brief Classic queue manager based on linked list and allocating cache.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 13.08.2007 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: alloc_cache, melocker (relocker), allocator
 *  Internal: cqueue
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __CQUEUE_HPP__
#define __CQUEUE_HPP__

#include "alloccache.hpp"
#include "impl/tslist.hpp"

namespace tstl {

template <class Tvalue = size_t, class Tlocker = melocker<>, class Tallocator = allocator,
          class Talloc_cache = iqalloc_cache <char, Tallocator, Tallocator> >

class cqueue
{
  list_head lh;

  typedef struct queue_elem { list_head lh; Tvalue value; } qe, *pqe;

  Talloc_cache* palloc_cache;

  Tallocator allocator;

  Tlocker queue_locker;

  long use_counter;

public:

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  cqueue (const long alloc_cache_elem = 64) : use_counter (0)
  {
    INIT_LIST_HEAD (& lh);

    palloc_cache = new qeac (sizeof (qe), alloc_cache_elem);
    if (!palloc_cache) { brk (); }
  }

  ~cqueue ()
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
bool cqueue    <Tvalue,       Tlocker,       Tallocator,       Talloc_cache>

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

  atomic_inc (& use_counter);

  /// add to list tail new element
  queue_locker.lock ();

  list_add_tail ( (plh) pe, & lh);

  queue_locker.unlock ();

  return true;
}

/// Retrives Tvalues.
/** \param[in] buffer is pointer to Tvalues
  * \return true if Tvalues get from pipe. */
template <class Tvalue, class Tlocker, class Tallocator, class Talloc_cache>
bool cqueue    <Tvalue,       Tlocker,       Tallocator,       Talloc_cache>

:: get (Tvalue* buffer)
{
  if (!buffer) { brk (); return false; }

  /// remove tail from list
  queue_locker.lock ();

  if (list_empty (& lh) )
  { queue_locker.unlock (); return false; }

  pqe pe = (pqe) lh.next;

  list_del (lh.next);

  queue_locker.unlock ();

  /// copy value from temporary buffer
  atomic_dec (& use_counter);

  tstl :: allocator a;
  :: new ( (void*) buffer, a) Tvalue (pe->value);

  /// free temporary buffer
  if (palloc_cache)
    palloc_cache->revert ( (char*) pe), pe = 0;
  else
    allocator.deallocate ( (char*) pe), pe = 0;

  return true;
}

}; /* end of tstl namespace */

#endif /* __CQUEUE_HPP__ */
