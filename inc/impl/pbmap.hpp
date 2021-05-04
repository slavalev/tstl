/*****************************************************************************************************//**
 *
 *  Module Name:	\file pbmap.hpp
 *
 *  Abstract:		\brief Hash table based multimap with partialy locked linked lists.
 *
 *  Author:	        \author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History: \date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External:	hash_key, ts_sleep, allocator, melocker
 *  Internal:	multimap, map_pos || mp
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __PBMAP_HPP__
#define __PBMAP_HPP__

#include "tstl.hpp"

#include "impl/tshash.hpp"
#include "impl/tslist.hpp"
#include "impl/relocker.hpp"

namespace tstl  {
namespace pbmap {

typedef struct map_pos
{
  plh plist_entry;
  plh plist_head;
  unsigned long map_elem;
  map_pos () : plist_entry (0), plist_head (0), map_elem (0) {}
} mp, *pmp;

template <class Tkey, class Tvalue, class Thash = size_t, class Tallocator = allocator, class Tlocker = melocker <> >

class multimap
{
  typedef struct map_elem
  {
    list_head list_head;
    long use_counter;
    Tlocker lk;

    void init ()
    {
      INIT_LIST_HEAD (& list_head);
      lk.init ();
      use_counter = 0;
    }

    ~map_elem () {}
  } me, *pme;

  typedef struct list_elem
  {
    /// redanted service information
    list_head list_entry;
    long  status; ///< "LIVE" || "KILL" || "DEAD"
    long  ref;

    /// usefull payload
    Thash hash;
    Tkey  key;
  } le, *ple;

  pme storage;
  Thash max_elem;
  long use_counter;

  Tallocator allocator;

  hash_key<Tkey, Thash> hk;

  /// Private map methods

  /// Lock reference on element
  void lock (plh& plist_entry)
  { atomic_inc ( & ( (ple) plist_entry)->ref); }

  /// Free reference on element
  void release (plh& plist_entry)
  { atomic_dec ( & ( (ple) plist_entry)->ref); }

  /// Synoname of Release
  void unlock (plh& plist_entry)
  { release (plist_entry); }

  bool remove (plh& plist_entry, plh plist_head);

  /// Search list entry by key & lock it
  bool search_by_key  (plh& plist_entry, plh plist_head, Tkey key);

  /// Search list entry by hash & lock it
  bool search_by_hash (plh& plist_entry, plh plist_head, Thash hash);

  /// Doesn't thread safe method, it called from destructor
  void remove_all_unsafe ();

public:

  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  multimap (const Thash root_array_elems = 32);

  ~multimap ()
  {
    remove_all_unsafe ();

    for (Thash i = 0; i < max_elem; ++i) storage [i].~map_elem ();

    if (storage) allocator.deallocate (storage), storage = 0;
  }

  /// Doesn't thread safe method
  bool is_empty () const
  { return 0 == use_counter; }

  /// Get statistic about map using
  long get_stat () const
  { return use_counter; }

  /// Get statistic about using map element
  long get_stat (Thash map_elem) const
  { return storage [map_elem % max_elem].use_counter; }

  /// Look for element in map by list_entry
  Tvalue* lookup (mp& pos) const
  { return (Tvalue*) ( ( (ple) pos.plist_entry) + 1); }

  /// Get hash by key
  Thash hash (Tkey key) const
  { return hk.hash (key); }

  /// Insert element in map & if successfull than lock element
  bool set_at (mp& pos, Tkey key, Thash hash, const Tvalue* pvalue);

  /// Insert element in map & if successfull than lock element
  bool set_at (mp& pos, Tkey key, const Tvalue* pvalue)
  { return set_at (pos, key, hk.hash (key), pvalue); }

  /// Look for element in map by key & if successfull search than lock element
  bool lookup_by_key (mp& pos, Tkey key, Tvalue*& pvalue);

  /// Look for element in map by keys hash & if successfull search than lock element
  bool lookup_by_key_hash (mp& pos, Tkey key, Tvalue*& pvalue);

  /// Look for element in map by hash & if successfull search than lock element
  bool lookup_by_hash (mp& pos, Thash hash, Tvalue*& pvalue);

  /// Unlock position in map
  void release (mp& pos)
  {
    release (pos.plist_entry);
    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
  }

  /// Synoname of release
  void unlock (mp& pos)
  { release (pos); }

  /// Remove element from map by ListEntry and always unlock element
  /** \return false if list entry destroyed */
  bool remove (mp& pos);

  bool remove_dead (mp& pos)
  { return remove (pos); }

  /// Remove element from map by key
  bool remove_by_key (Tkey key);

  /// Remove element from map by keys hash
  bool remove_by_key_hash (Tkey key);

  /// Remove element from map by hash
  bool remove_by_hash (Thash hash);

  /// Remove all elements from map
  void remove_all ();

  /// Begin maps enumerating & if successfull than lock element
  bool start (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Next maps enumerating & if failure than unlock last element
  bool next (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue);

  /// Begin maps enumerating by hash (AKA multimap) & if successfull than lock element
  bool start (mp& pos, Thash hash, Tvalue*& pvalue)
  { return lookup_by_hash (pos, hash, pvalue); /* First time use standart method */ }

  /// Next maps enumerating by hash (AKA multimap) & if failure than unlock last element
  bool next (mp& pos, Thash hash, Tvalue*& pvalue);
};

template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove (plh& plist_entry, plh plist_head)
{
  /// List head removing protection
  if (plist_entry == plist_head)
  { brk (); return false; }

  list_del (plist_entry);

#if defined (DEBUG)
  plist_entry->next = plist_entry->prev = 0;
#endif

  ( (Tvalue*) ( ( (ple) plist_entry) + 1) ) -> ~Tvalue ();

  allocator.deallocate (plist_entry);

  atomic_dec (& use_counter);
  return true;
}

/// Search list entry by key & lock it
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: search_by_key (plh& plist_entry, plh plist_head, Tkey key)
{
  if (list_empty (plist_head))
    return false;

  plh plist_entry_next;

  list_for_each_safe (plist_entry, plist_entry_next, plist_head)
  {
    if (!plist_entry)
    {
      /// Element removed, very bad break point !!!
      brk ();
      plist_entry = plist_head;

      return false;
    }

    if (TS_LIVE_SIGN != ( (ple) plist_entry)->status)
    {
      /// Element dead
      brk ();

      if (TS_DEAD_SIGN == ( (ple) plist_entry)->status
       && ( (ple) plist_entry)->ref < 0)
      {
	/// Element marked as dead and element readies for removing
	brk ();
	remove (plist_entry, plist_head);
      }

      continue;
    }

    if ( ( (ple) plist_entry)->key == key)
    {
      lock (plist_entry);

      return true;
    }
  }

  return false;
}

/// Search list entry by hash & lock it
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: search_by_hash (plh& plist_entry, plh plist_head, Thash hash)
{
  if (list_empty (plist_head))
    return false;

  plh plist_entry_next;

  list_for_each_safe (plist_entry, plist_entry_next, plist_head)
  {
    if (!plist_entry)
    {
      /// Element removed, very bad break point !!!
      brk ();
      plist_entry = plist_head;

      return false;
    }

    if (TS_LIVE_SIGN != ( (ple) plist_entry)->status)
    {
      /// Element dead
      brk ();

      if (TS_DEAD_SIGN == ( (ple) plist_entry)->status
      && ( (ple) plist_entry)->ref < 0)
      {
	/// Element marked as dead and element readies for removing
	brk ();
	remove (plist_entry, plist_head);
      }

      continue;
    }

    if ( ( (ple) plist_entry)->hash == hash)
    {
      lock (plist_entry);
      return true;
    }
  }

  return false;
}

template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
multimap       <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: multimap (const Thash root_array_elems = 32) : max_elem (root_array_elems)
{
  storage = (pme) allocator.allocate (sizeof (*storage) * max_elem);
  if (!storage) { brk (); return; }

  pme p = storage;

  for (Thash i = 0; i < max_elem; i++, p++) p->init ();

  use_counter = 0;
}

/// Insert element in map & if successfull than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: set_at (mp& pos, Tkey key, Thash hash, const Tvalue* pvalue)
{
  if (!storage || !max_elem) { brk (); return false; }

  pos.plist_entry = (plh) allocator.allocate (sizeof (le) + sizeof (Tvalue) );

  if (!pos.plist_entry)
  { brk (); return false; }

  ( (ple) pos.plist_entry)->key  = key;
  ( (ple) pos.plist_entry)->hash = hash;

  tstl :: allocator a;
  //*( (Tvalue*) ( ( (ple) pos.plist_entry) + 1) ) = *pvalue;
  :: new ( (void*) ( ( (ple) pos.plist_entry) + 1), a) Tvalue (*pvalue);

  ( (ple) pos.plist_entry)->ref  = 1; ///< Lock element
  ( (ple) pos.plist_entry)->status = TS_LIVE_SIGN;

  pos.map_elem = (unsigned long) hash % max_elem;
  pos.plist_head = & storage [pos.map_elem].list_head;

  storage [pos.map_elem].lk.lock (); ///< Lock linked list

  atomic_inc (& use_counter);
  atomic_inc (& storage [pos.map_elem].use_counter);

  list_add (pos.plist_entry, pos.plist_head);

  return true;
}

/// Look for element in map by key & if successfull search than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: lookup_by_key (mp& pos, Tkey key, Tvalue*& pvalue)
{
  if (!storage || !max_elem) { brk (); return false; }

  pos.map_elem  = (unsigned long) hk.hash (key) % max_elem;
  pos.plist_head = & storage [pos.map_elem].list_head;

  storage [pos.map_elem].lk.lock (); ///< Lock linked list

  if (!search_by_key (pos.plist_entry, pos.plist_head, key))
  {
    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
    return false;
  }

  pvalue = lookup (pos);
  return true;
}

/// Look for element in map by keys hash & if successfull search than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: lookup_by_key_hash (mp& pos, Tkey key, Tvalue*& pvalue)
{
  if (!storage || !max_elem) { brk (); return false; }

  Thash hash    = hk.hash (key);
  pos.map_elem  = hash % max_elem;
  pos.plist_head = & storage [pos.map_elem].list_head;

  storage [pos.map_elem].lk.lock (); ///< Lock linked list

  if (!search_by_hash (pos.plist_entry, pos.plist_head, hash))
  {
    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
    return false;
  }

  pvalue = lookup (pos);
  return true;
}

/// Look for element in map by hash & if successfull search than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: lookup_by_hash (mp& pos, Thash hash, Tvalue*& pvalue)
{
  if (!storage || !max_elem) { brk (); return false; }

  pos.map_elem   = hash % max_elem;
  pos.plist_head = & storage [pos.map_elem].list_head;

  storage [pos.map_elem].lk.lock (); ///< Lock linked list

  if (!search_by_hash (pos.plist_entry, pos.plist_head, hash))
  {
    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
    return false;
  }

  pvalue = lookup (pos);
  return true;
}

/// Remove element from map by ListEntry and always unlock element
/** \return false if list entry destroyed */
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove (mp& pos)
{
  if (!storage || !max_elem) { brk (); return false; }

  if (!pos.plist_entry)
  {
    /// Element removed, very bad break point !!!
    brk ();

    storage [pos.map_elem].lk.unlock (); /// Unlock linked list
    return false;
  }

  if (pos.plist_entry == pos.plist_head)
  {
    /// Try to remove head of list, very bad break point !!!
    brk ();

    storage [pos.map_elem].lk.unlock (); /// Unlock linked list
    return false;
  }

  /// If LIVE status passed, than entry to killing status
  long status = atomic_compare_exchange (& ( (ple) pos.plist_entry)->status, TS_KILL_SIGN, TS_LIVE_SIGN);

  /// Release element
  release (pos.plist_entry);

  /// List element status control
  if (TS_LIVE_SIGN != status)
  {
    if (TS_KILL_SIGN == status)
    {
      /// Already in killing mode - release and go out
      tbrk ();
      atomic_dec (& ( (ple) pos.plist_entry)->ref);
    }
    else
    if (TS_DEAD_SIGN == status)
    {
      /// Element marked as dead
      if ( ( (ple) pos.plist_entry)->ref < 0)
      {
	/// Element ready for removing
	brk ();
	remove (pos.plist_entry, pos.plist_head);

	atomic_dec (& storage [pos.map_elem].use_counter);

	storage [pos.map_elem].lk.unlock (); /// Unlock linked list

	return true;
      }
      else
      {
	/// Element doesn't ready for removing
	brk ();
      }
    }
    else
    {
      /// List status corrupted, very bad break point !!!
      brk ();
    }

    storage [pos.map_elem].lk.unlock (); /// Unlock linked list
    return false;
  }
  
  /// Try to lock reference counter for using
  long ref = atomic_dec_return (& ( (ple) pos.plist_entry)->ref);

  if (ref < 0)
  {
    /// Reference counter locked successfull
    remove (pos.plist_entry, pos.plist_head);

    atomic_dec (& storage [pos.map_elem].use_counter);

    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
    return true;
  }

  /// List element's in using, try to wait releasing
  long retry = TS_SPINLOCK_COUNTER;

  for (; retry > 0; retry--)
  {
    if ( ( (ple) pos.plist_entry)->ref < 0)
      break;

    ts_sleep (TS_SPINLOCK_SLEEP_TIME);
  }

  if (!retry)
  {
    brk ();

    /// Mark list element for removing
    status = atomic_compare_exchange (& ( (ple) pos.plist_entry)->status, TS_DEAD_SIGN, TS_KILL_SIGN);

    atomic_dec (& storage [pos.map_elem].use_counter);

    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
    return true;
  }

  brk ();

  if (!pos.plist_entry->next || !pos.plist_entry->prev)
  {
    /// List element removed, vary bad break point !!!
    brk ();

    storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
    return true;
  }

  remove (pos.plist_entry, pos.plist_head);

  atomic_dec (& storage [pos.map_elem].use_counter);

  storage [pos.map_elem].lk.unlock (); ///< Unlock linked list
  return true;
}

/// Remove element from map by key
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove_by_key (Tkey key)
{
  mp pos;
  Tvalue* pvalue;

  if (!lookup_by_key (pos, key, pvalue) )
    return false;

  return remove (pos);
}

/// Remove element from map by keys hash
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove_by_key_hash (Tkey key)
{
  mp pos;
  Tvalue* pvalue;

  if (!lookup_by_key_hash (pos, key, pvalue) )
    return false;

  return remove (pos);
}

/// Remove element from map by hash
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove_by_hash (Thash hash)
{
  mp pos;
  Tvalue* pvalue;

  if (!lookup_by_hash (pos, hash, pvalue) )
    return false;

  return remove (pos);
}

/// Remove all elements in map
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
void multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove_all ()
{
  Tkey key;
  Thash hash;
  Tvalue* pvalue;

  mp pos;

  while (start (pos, key, hash, pvalue) )
  {
    if (!remove (pos) )
    { brk (); release (pos); }
  }
}

/// Doesn't thread safe method, it called from destructor
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
void multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: remove_all_unsafe ()
{
  mp pos;

  /// Move to ahead of maps array
  for (pos.map_elem = 0; pos.map_elem < max_elem; pos.map_elem++)
  {
    pos.plist_head = & storage [pos.map_elem].list_head;

    if (list_empty (pos.plist_head) )
      continue;

    plh plist_entry_next;
    list_for_each_safe (pos.plist_entry, plist_entry_next, pos.plist_head)
    {
      brk ();
      remove (pos.plist_entry, pos.plist_head);

      atomic_dec (& storage [pos.map_elem].use_counter);
      atomic_dec (& use_counter);
    }
  }
}

/// Begin maps enumerating & if successfull than lock element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: start (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  /// Move to ahead of maps array
  for (pos.map_elem = 0;
       pos.map_elem < max_elem;
       storage [pos.map_elem].lk.unlock (),
       pos.map_elem++)
  {
    storage [pos.map_elem].lk.lock (); ///< Lock linked list

    pos.plist_head = & storage [pos.map_elem].list_head;

    if (list_empty (pos.plist_head) )
      continue;
    
    pos.plist_entry = pos.plist_head->next;

    if (!pos.plist_entry)
    {
      /// List element removed, very bad break point !!!
      brk ();
      continue;
    }

    /// List element status control
    while (pos.plist_entry != pos.plist_head
	  && TS_LIVE_SIGN  != ( (ple) pos.plist_entry)->status)
    {
      /// Element dead
      brk ();

      if (TS_DEAD_SIGN == ( (ple) pos.plist_entry)->status
      && ( (ple) pos.plist_entry)->ref < 0)
      {
	/// Element marked as dead and element readies for removing
	brk ();

	/// Save pointer on next entry
	plh plist_entry_next = pos.plist_entry->next;

	remove (pos.plist_entry, pos.plist_head);

	/// Set pointer on next entry
	pos.plist_entry = plist_entry_next;
      }
      else
      {
	/// Set pointer on next entry
	pos.plist_entry = pos.plist_entry->next;
      }
    } ///< End while dead elements

    if (pos.plist_entry == pos.plist_head)
    { /// End of list reached
      /// Go to next list
      continue; 
    }

    lock (pos.plist_entry);

    key  = ( (ple) pos.plist_entry)->key;
    hash = ( (ple) pos.plist_entry)->hash;
    pvalue = lookup (pos);
    return true;
  } ///< End for map_elem

  return false;
}

/// Next maps enumerating & if failure than unlock last element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: next (mp& pos, Tkey& key, Thash& hash, Tvalue*& pvalue)
{
  do
  {
    if (pos.plist_entry == pos.plist_entry->next)
    {
      /// List looped, very bad break point !!!
      brk ();
      pos.plist_entry->next = pos.plist_head;
    }

    /// Move to ahead of linked list
    plh plist_entry_prev = pos.plist_entry;
    pos.plist_entry = pos.plist_entry->next;

    if (plist_entry_prev != pos.plist_head)
	release (plist_entry_prev);

    if (!pos.plist_entry)
    {
      /// List element removed, very bad break point !!!
      brk ();
      pos.plist_entry = pos.plist_head;
    }

    /// List element status control
    while (pos.plist_entry != pos.plist_head
	&& TS_LIVE_SIGN != ( (ple) pos.plist_entry)->status)
    {
      /// Element dead
      brk ();

      if (TS_DEAD_SIGN == ( (ple) pos.plist_entry)->status
      && ( (ple) pos.plist_entry)->ref < 0)
      {
	/// Element marked as dead and element readies for removing
	brk ();

	/// Save pointer on next entry
	plh plist_entry_next = pos.plist_entry->next;
	remove (pos.plist_entry, pos.plist_head);

	/// Set pointer on next entry
	pos.plist_entry = plist_entry_next;
      }
      else
      {
	/// Set pointer on next entry
	pos.plist_entry = pos.plist_entry->next;
      }
    } ///< End while dead elements

    if (pos.plist_entry != pos.plist_head)
    {	
      lock (pos.plist_entry);

      key  = ( (ple) pos.plist_entry)->key;
      hash = ( (ple) pos.plist_entry)->hash;
      pvalue = lookup (pos);
      return true;
    }

    /// Go to next list

    /// Move to ahead of maps array
    for (storage [pos.map_elem].lk.unlock ();
         ++pos.map_elem < max_elem;
         storage [pos.map_elem].lk.unlock () )
    {
      storage [pos.map_elem].lk.lock (); ///< Lock linked list

      pos.plist_head = &storage [pos.map_elem].list_head;

      if (!list_empty (pos.plist_head) )
      {
	pos.plist_entry = pos.plist_head;
	break;
      }
    }
  }
  while (pos.map_elem < max_elem);

  return false;
}

/// Next maps enumerating by hash (AKA multimap) & if failure than unlock last element
template <class Tkey, class Tvalue, class Thash, class Tallocator, class Tlocker>
bool multimap  <Tkey,       Tvalue,       Thash,       Tallocator,       Tlocker>

:: next (mp& pos, Thash hash, Tvalue*& pvalue)
{
  if (pos.plist_entry == pos.plist_entry->next)
  {
    /// List looped, very bad break point !!!
    brk ();
    pos.plist_entry->next = pos.plist_head;
  }

  /// Move to ahead of linked list
  plh plist_entry_prev = pos.plist_entry;
  pos.plist_entry = pos.plist_entry->next;

  if (plist_entry_prev != pos.plist_head)
    release (plist_entry_prev);

  if (!pos.plist_entry)
  {
    /// List element removed, very bad break point !!!
    brk ();
    pos.plist_entry = pos.plist_head;
  }

  /// List element status control
  while (pos.plist_entry != pos.plist_head
      && (TS_LIVE_SIGN != ( (ple) pos.plist_entry)->status
	 || hash !=       ( (ple) pos.plist_entry)->hash))
  {
    if (TS_DEAD_SIGN == ((ple)pos.plist_entry)->status
    && ( (ple) pos.plist_entry)->ref < 0)
    {
      /// Element marked as dead and element readies for removing
      brk ();

      /// Save pointer on next entry
      plh plist_entry_next = pos.plist_entry->next;
      remove (pos.plist_entry, pos.plist_head);

      /// Set pointer on next entry
      pos.plist_entry = plist_entry_next;
    }
    else
    {
      /// Set pointer on next entry
      pos.plist_entry = pos.plist_entry->next;
    }
  } /// End while dead elements

  if (pos.plist_entry != pos.plist_head)
  {	
    /// Bingo !!! Founded equial hash - time to change hash function !!!
    brk ();

    lock (pos.plist_entry);

    pvalue = lookup (pos);
    return true;
  }
  
  storage [pos.map_elem].lk.unlock ();
  return false;
}

}; /* end of pbmap namespace */

}; /* end of tstl namespace */

#endif /* __PBMAP_HPP__ */
