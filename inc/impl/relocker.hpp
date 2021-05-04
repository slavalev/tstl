/*****************************************************************************************************//**
 *
 *  Module Name:	\file relocker.hpp
 *
 *  Abstract:		\brief Reenterable mutual exclusion locker.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 10.11.2008 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: melocker
 *  Internal: relocker
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __RELOCKER_HPP__
#define __RELOCKER_HPP__

#include "impl/tslist.hpp"
#include "impl/melocker.hpp"

namespace tstl {

/// Reenterable locker
template <class Tlocker = melocker<>, class Tallocator = allocator>

class relocker
{
  typedef struct thread_elem
  {
    list_head list_entry;
    size_t thread_id;
    long entry_cnt;

    void init (const size_t in_thread_id)
    {
      entry_cnt = 1, thread_id = in_thread_id;
      list_entry.prev = 0, list_entry.next = 0;
    }

    thread_elem (const size_t in_thread_id)
    { init (in_thread_id); }

    ~thread_elem ()
    { list_entry.prev = 0; list_entry.next = 0; }

  private:
    thread_elem ();
  } thread_elem, te, *pthread_elem, *pte;

  list_head  thread_list;
  Tlocker locker, list_locker;

  Tallocator allocator;

  long use_counter;

  bool remove (plh& plist_entry, const plh plist_head);

public:

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  relocker () : use_counter (0)
  { INIT_LIST_HEAD (& thread_list); }

  ~relocker ();

  long get_use_counter () const
  { return use_counter; }

  bool lock    (const size_t thread_id);

  bool unlock  (const size_t thread_id);

  bool release (const size_t thread_id)
  { return unlock (thread_id); }
};

template <class Tlocker, class Tallocator>
bool relocker  <Tlocker,       Tallocator>

:: remove (plh& plist_entry, const plh plist_head)
{
  if (!plist_entry || plist_entry == plist_head) ///< List head removing protection
  { brk (); return false; }

  list_del (plist_entry);

  atomic_dec (& use_counter);

  allocator.deallocate (plist_entry), plist_entry = 0;
  return true;
}

template <class Tlocker, class Tallocator>
    relocker   <Tlocker,       Tallocator>

:: ~relocker ()
{
  list_locker.lock ();

  if (list_empty (& thread_list) )
  { list_locker.unlock (); return; }

  plh pentry = 0, pnext = 0;

  list_for_each_safe (pentry, pnext, & thread_list)
  {
    if (!pentry) ///< Element removed, very bad break point !!!
    { brk (); list_locker.unlock (); return; }

    remove (pentry, & thread_list);
  }

  list_locker.unlock ();
}

template <class Tlocker, class Tallocator>
bool relocker  <Tlocker,       Tallocator>

:: lock (const size_t thread_id)
{
  bool first = false;

  list_locker.lock ();

  if (list_empty (& thread_list) )
    first = true;

  if (!first)
  {
    plh pentry = 0;

    list_for_each (pentry, & thread_list)
    {
	if (!pentry) ///< Element removed, very bad break point !!!
	{ brk (); list_locker.unlock (); return false; }

	if (thread_id != ( (pte) pentry)->thread_id)
	  continue;

	atomic_inc (& ( (pte) pentry)->entry_cnt);

	list_locker.unlock ();
	return true;
    }
  }

  pte p = (pte) allocator.allocate (sizeof (*pte) );
  if (!p) { brk (); list_locker.unlock (); return false; }

  p->init (thread_id);

  atomic_inc (& use_counter);

  list_add ( (plh) &p, & thread_list);

  if (first)
    locker.lock ();

  list_locker.unlock ();
  return true;
}

template <class Tlocker, class Tallocator>
bool relocker  <Tlocker,       Tallocator>

:: unlock (const size_t thread_id)
{
  list_locker.lock ();

  if (list_empty (& thread_list) )
  { brk (); list_locker.unlock (); return false; }

  plh pentry = 0, pnext = 0;

  list_for_each_safe (pentry, pnext, & thread_list)
  {
    if (!pentry) ///< Element removed, very bad break point !!!
    { brk (); list_locker.unlock (); return false; }

    if (thread_id != ( (pte) pentry)->thread_id)
      continue;

    if (atomic_dec_return ( & ( (pte)pentry)->entry_cnt) )
    { list_locker.unlock (); return true; }

    bool rc = remove (pentry, &thread_list);

    if (rc && list_empty (& thread_list) )
    {
      locker.unlock ();

      list_locker.unlock ();
      return true;
    }

    list_locker.unlock ();
    return rc;
  }

  list_locker.unlock ();
  return false;
}

}; /* end of tstl namespace */

#endif /* __RELOCKER_HPP__ */
