/*****************************************************************************************************//**
 *
 *  Module Name:	\file tshash.hpp
 *
 *  Abstract:		\brief Hashes templates implementation. It uses in thread safe containers 
 *                  as storage index generator by key.
 *
 *  Author:		\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Export: hash_key, hash_key<const unsigned short*>, hash_key<const char*>
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __TSHASH_HPP__
#define __TSHASH_HPP__

namespace tstl {

template <class Tkey, class Thash = size_t>
struct hash_key
{
  Thash hash (Tkey key) const
  { return (Thash)( ( (Thash) key >> 3) + ( (Thash) key >> 2) + ( (Thash) key >> 1) + ( (Thash) key & 1) ); }
};

#define TS_STRING_HASH_FUNC(char_type) \
{ Thash hash (char_type* key) const    \
   { register Thash hash = (Thash) 0;  \
    while (*key) hash += (hash << 5) + *key++;\
    return hash; } }

#if _MSC_VER < 1300
#  define TS_STRING_HASH_TMPL(char_type)	\
   template <> struct hash_key <char_type*> TS_STRING_HASH_FUNC (char_type);
#else
#  define TS_STRING_HASH_TMPL(char_type)	\
   template <class Thash> struct hash_key <char_type*, Thash> TS_STRING_HASH_FUNC (char_type);
#endif /* _MSC_VER < 1300 */

#if defined (_MSC_VER)
#  if defined ( _WCHAR_T_DEFINED)
TS_STRING_HASH_TMPL (wchar_t);
TS_STRING_HASH_TMPL (const wchar_t);
#  else
TS_STRING_HASH_TMPL (unsigned short);
TS_STRING_HASH_TMPL (const unsigned short);
#  endif
#else
TS_STRING_HASH_TMPL (wchar_t);
TS_STRING_HASH_TMPL (const wchar_t);

TS_STRING_HASH_TMPL (unsigned short);
TS_STRING_HASH_TMPL (const unsigned short);
#endif

TS_STRING_HASH_TMPL (char);
TS_STRING_HASH_TMPL (const char);

}; /* end of tstl namespace */

#endif /* __TSHASH_HPP__ */
