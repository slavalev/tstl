/*****************************************************************************************************//**
 *
 *  Module Name:	\file tsprequeue.hpp
 *
 *  Abstract:		\brief Priority queue manager.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2009 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: allocator, queue, multimap
 *  Internal: prequeue
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSPREQUEUE_HPP__
#define __TSPREQUEUE_HPP__

#include "tsmap.hpp"
#include "tsqueue.hpp"

namespace tstl {

template <class Tvalue, class Thash = size_t, class Tallocator = allocator,
          class Tmultimap = nbmap :: multimap <long, iqueue <Tvalue>*, Thash, Tallocator>,
          class Tmap_pos  = nbmap :: mp>

class prequeue : Tmultimap
{
  const long alloc_cache_elem;
  long  use_counter;

  Tallocator allocator;

public:
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  prequeue (const long in_alloc_cache_elem = 16,
            const unsigned long root_array_elems = 32)
          : Tmultimap (root_array_elems), use_counter (0),
            alloc_cache_elem (in_alloc_cache_elem) {}

  ~prequeue ();

  /// Is not thread safe method
  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about cache using
  long get_stat () const
  { return use_counter; }

  /// Stores Tvalues.
  /** \param[in] buffer is pointer to Tvalues
    * \param[in] prioritet is number from 0 to 2^32
    * \return true if Tvalues put into pipe. */
  bool put (Tvalue* buffer, long prioritet = 0);

  /// Retrives Tvalues.
  /** \param[in] buffer is pointer to Tvalues
    * \param[in] prioritet is number from 0 to 2^32
    * \return true if Tvalues get from pipe. */
  bool get (Tvalue* buffer, long prioritet = 0);
};

/// Stores Tvalues.
/** \param[in] buffer is pointer to Tvalues
  * \param[in] prioritet is number from 0 to 2^32
  * \return true if Tvalues put into pipe. */
template <class Tvalue, class Thash, class Tallocator, class Tmultimap, class Tmap_pos>
bool prequeue  <Tvalue,       Thash,       Tallocator,       Tmultimap,       Tmap_pos>

:: put (Tvalue* buffer, long prioritet)
{
  if (!pqueue_map || !buffer) { brk (); return false; }

  queue <Tvalue>* pq = 0, *ppq = 0;
  Tmap_pos pos;

  if (!lookup_by_key (pos, prioritet, ppq) )
  {
    pq = new queue <Tvalue> (alloc_cache_elem);
    if (!pq) { brk (); return false; }

    if (!set_at (pos, prioritet, & pq) )
    {
	brk ();
	delete (pq), pq = 0;
	return false;
    }
  }
  else
  if (!ppq || !*ppq)
  {
    brk ();
    release (pos);
    return false;
  }
  else
    pq = *ppq;

  if (!pq->put (buffer) )
  {
    brk ();
    release (pos);
    return false;
  }

  release (pos);

  atomic_inc (& use_counter);
  return true;
}

/// Retrives Tvalues.
/** \param[in] buffer is pointer to Tvalues
  * \param[in] prioritet is number from 0 to 2^32
  * \return true if Tvalues get from pipe. */
template <class Tvalue, class Thash, class Tallocator, class Tmultimap, class Tmap_pos>
bool prequeue  <Tvalue,       Thash,       Tallocator,       Tmultimap,       Tmap_pos>

:: get (Tvalue* buffer, long prioritet)
{
  if (!buffer) { brk (); return false; }

  queue <Tvalue>* pq = 0, *ppq = 0;
  Tmap_pos pos;

  if (!lookup_by_key (pos, prioritet, ppq) )
  { brk (); return false; }
  else
  if (!ppq || !*ppq)
  {
    brk ();
    release (pos);
    return false;
  }
  else
    pq = *ppq;

  if (!pq->get (buffer) )
  {
    release (pos);
    return false;
  }

  release (pos);

  atomic_dec (& use_counter);
  return true;
}

template <class Tvalue, class Thash, class Tallocator, class Tmultimap, class Tmap_pos>
prequeue       <Tvalue,       Thash,       Tallocator,       Tmultimap,       Tmap_pos>

:: ~prequeue ()
{
  Thash hash;
  Tmap_pos pos;

  long pprioritet = 0;
  queue <Tvalue>** ppq = 0;

  if (!start (pos, pprioritet, hash, ppq))
    return;

  bool rc = false;

  do
   {
    Tmap_pos prev_pos = pos;

    queue <Tvalue>* pq = 0;

    if (ppq && *ppq)
      pq = *ppq;

    rc = next (pos, pprioritet, hash, ppq);

    remove_dead (prev_pos);

    if (pq) delete (pq), pq = 0;
   }
  while (rc);
}

}; /* end of tstl namespace */

#endif /* __TSPREQUEUE_HPP__ */
