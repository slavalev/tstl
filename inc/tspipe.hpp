/*****************************************************************************************************//**
 *
 *  Module Name:	\file tspipe.hpp
 *
 *  Abstract:		\brief Classic queue based on cyclo buffer. Could transport different buffer size data
 *                  without memory allocations.
 *
 *  Author:	        \author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History: \date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  External: allocator, melocker (relocker)
 *  Internal: pipe
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSPIPE_HPP__
#define __TSPIPE_HPP__

#include "impl/relocker.hpp"

namespace tstl {

/// Pipe class template
/** put		- stores   data (thread safe).
  * get		- retrives data (thread safe).
  * free_size	- returns the size of the free space.
  * used_size	- returns the size of the used space. */
template <class Tvalue, class Tsize = unsigned long, class Tlocker = melocker<>, class Tallocator = allocator>

class pipe
{
  Tvalue*  storage;    ///< cyclo buffer
  volatile Tsize buffer_size; ///< cyclo buffer size
  volatile Tsize head; ///< head offset
  volatile Tsize tail; ///< tail offset
  Tlocker head_locker; ///< Head protection on multi writers
  Tallocator allocator;

  bool copy (Tvalue* destination, const Tvalue* source, const Tsize values_number)
  {
    if (!destination || !source || !values_number) { brk (); return false; }

    tstl :: allocator a;

    for (Tsize counter = 0;
         counter < values_number;
         counter++, source++, destination++)
      :: new ( (void*) destination, a) Tvalue (*source);

    return true;
  }

public:
  void* operator new (size_t size)
  { Tallocator allocator; return allocator.allocate (size); }

  void operator delete (void* p)
  { Tallocator allocator; allocator.deallocate (p); }

  pipe (const Tsize init_size = 0x1000) : storage (0), tail (0), head (0), buffer_size (0)
  {
    buffer_size = init_size;
    storage = (Tvalue*) allocator.allocate (buffer_size * sizeof (Tvalue) );

    if (!storage) { brk (); return; }

    memset (storage, 0, buffer_size * sizeof (Tvalue));
  }

  ~pipe ()
  { if (storage) { allocator.deallocate (storage), storage = 0; } else { brk (); } }

  /// Returns the size of the free space.
  Tsize free_size ();

  /// Returns the size of the used space.
  Tsize used_size ();

  /// Check pipe usage
  bool is_empty () const
  { return 0 == used_size (); }

  /// Get statistic about pipe using
  long get_stat () const
  { return (long) used_size (); }

  /// Stores Tvalues.
  /** \param buffer  - pointer to Tvalues
    * \param counter - Tvalues counter
    * \return true if Tvalues put into pipe. */
  bool put (Tvalue* buffer, Tsize counter = 1);

  /// Retrives Tvalues.
  /** \param buffer  - pointer to Tvalues
    * \param counter - Tvalues counter
    * \return true if Tvalues get from pipe. */
  bool get (Tvalue* buffer, Tsize counter = 1);
};

/// Returns the size of the free space.
template <class Tvalue, class Tsize, class Tlocker, class Tallocator>
Tsize pipe     <Tvalue,       Tsize,       Tlocker,       Tallocator>

:: free_size ()
{
  if (!storage)
    return 0;

  Tsize ltail = tail, lhead = head; ///< Tail and Head may be changed

  /// check empty space
  if (ltail > lhead)
    return ltail - lhead;
  else
    return buffer_size - lhead + ltail;
}

/// Returns the size of the used space.
template <class Tvalue, class Tsize, class Tlocker, class Tallocator>
Tsize pipe     <Tvalue,       Tsize,       Tlocker,       Tallocator>

:: used_size ()
{
  if (!storage)
    return buffer_size;

  Tsize ltail = tail, lhead = head; ///< Tail and Head may be changed

  /// get the size of the used space
  if (lhead >= ltail)
    return lhead - ltail;
  else
    return buffer_size - ltail + lhead;
}

/// Stores Tvalues.
/** \param buffer  - pointer to Tvalues
  * \param counter - Tvalues counter
  * \return true if Tvalues put into pipe. */
template <class Tvalue, class Tsize, class Tlocker, class Tallocator>
bool pipe      <Tvalue,       Tsize,       Tlocker,       Tallocator>

:: put (Tvalue* buffer, Tsize counter)
{
  if (!storage || !buffer)
    return false;

  if (!counter)
    return true;

  head_locker.lock ();

  Tsize free_size, part_size, ltail = tail; ///< Tail may be changed

  /// check if the free space is available
  if (head < ltail) ///< :DDDD_H........T_DDDD:
  {
    free_size = ltail - 1 - head;
    part_size = free_size;
  }
  else /// (lTail <= Head) :......T_DDDDD_H......:
  {
    part_size = buffer_size - head;
    free_size = part_size + ltail - 1;
  }

  if (free_size > buffer_size
   || part_size > buffer_size)
  { brk (); head_locker.unlock (); return false; }

  if (free_size < counter)
  {
    brk (); ///< pipe buffer is fully used, increase buffer or wait
    head_locker.unlock ();
    return false;
  }

  Tsize pos = 0;

  if (counter > part_size) ///< do cust
  {
    if (head + part_size > buffer_size
    || !copy (& storage [head], & buffer [pos], part_size) )
    { brk (); head_locker.unlock (); return false; }

    head += part_size;

    if (head == buffer_size) head = 0;

    counter -= part_size;
    pos     += part_size;
  }

  if (counter)
  {
    if (head + counter > buffer_size
    || !copy (& storage [head], & buffer [pos], counter) )
    { brk (); head_locker.unlock (); return false; }

    head += counter;

    if (head == buffer_size) head = 0;
  }

  head_locker.unlock ();
  return true;
}

/// Retrives Tvalues.
/** \param buffer  - pointer to Tvalues
  * \param counter - Tvalues counter
  * \return true if Tvalues get from pipe. */
template <class Tvalue, class Tsize, class Tlocker, class Tallocator>
bool pipe      <Tvalue,       Tsize,       Tlocker,       Tallocator>

:: get (Tvalue* buffer, Tsize counter)
{
  if (!storage || !buffer)
    return false;

  if (!counter)
    return true;

  Tsize used_size, part_size, lhead = head; ///< Head may be changed

  /// gets the size of the used space
  if (lhead < tail) ///< :DDDD_H........T_DDDD:
  {
    part_size = buffer_size - tail;
    used_size = part_size + lhead;
  }
  else /// (Tail <= lHead) :......T_DDDDD_H......:
  {
    used_size = lhead - tail;
    part_size = used_size;
  }

  if (used_size > buffer_size
   || part_size > buffer_size)
  { brk (); return false; }

  if (used_size < counter)
    return false;

  Tsize pos = 0;

  if (counter > part_size) ///< do cust
  {
    if ( (tail + part_size) > buffer_size
      || !copy (& buffer [pos], & storage [tail], part_size) )
    { brk (); return false; }

    tail += part_size;

    if (tail == buffer_size) tail = 0;

    counter -= part_size;
    pos     += part_size;
  }

  if (counter)
  {
    if ( (tail + counter) > buffer_size
      || !copy (& buffer [pos], & storage [tail], counter) )
    { brk (); return false; }

    tail += counter;

    if (tail == buffer_size) tail = 0;
  }

  return true;
}

template <class Tpipe>
static bool init_pipe (Tpipe*& pp, long buffer_size)
{
  pp = new Tpipe (buffer_size);
  return 0 != pp;
};

}; /* end of tstl namespace */

#endif /* __TSPIPE_HPP__ */
