/*****************************************************************************************************//**
 *
 *  Module Name:	\file tsqueue.hpp
 *
 *  Abstract:		\brief Queue manager.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 13.08.2007 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: allocator, alloc_cache, melocker (relocker)
 *  Internal: queue
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSQUEUE_H__
#define __TSQUEUE_H__

#include "tspipe.hpp"

#include "impl/cqueue.hpp"
#include "impl/iqueue.hpp"

namespace tstl {

template <class Tvalue, class Tallocator = allocator,
          class Tqueue = iqueue <Tvalue, melocker<>, Tallocator> >

struct queue : Tqueue ///< Tqueue == iqueue || cqueue || pipe
{
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  queue (const long alloc_cache_elem = 64) : Tqueue (alloc_cache_elem) {}

  /// Is not thread safe method
  bool is_empty () const
  { return Tqueue :: is_empty (); }

  /// Get statistic about cache using
  long get_stat () const
  { return Tqueue :: get_stat (); }

  /// Stores Tvalues.
  /** \param[in] buffer is pointer to Tvalues
    * \return true if Tvalues put into pipe. */
  bool put (Tvalue* buffer)
  { return Tqueue :: put (buffer); }

  /// Retrives Tvalues.
  /** \param[in] buffer is pointer to Tvalues
    * \return true if Tvalues get from pipe. */
  bool get (Tvalue* buffer)
  { return Tqueue :: get (buffer); }
};

template <class Tqueue>
static bool init_queue (Tqueue*& pq, long alloc_cache_elem)
{
  pq = new Tqueue (alloc_cache_elem);
  return 0 != pq;
};

}; /* end of tstl namespace */

#endif /* __TSQUEUE_H__ */
