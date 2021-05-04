/*****************************************************************************************************//**
 *
 *  Module Name:	\file gasm.h
 *
 *  Abstract:		\brief GCC inline assembler macroses implementation.
 *
 *  Author:		\author Andrew Zabolotny (mail-to: anpaza@mail.ru).
 *
 *  Revision History:	\date 05.08.2003 started
 *
 *  Classes, methods and structures: \details
 *
 *  Internal: InterlockedIncrement, InterlockedDecrement, InterlockedExchangeAdd,
 *	          interlocked_exchange, interlocked_compare_exchange
 *
 *  TODO: 		\todo
 *
 *********************************************************************************************************/

#ifndef __GASM_H__
#define __GASM_H__

static inline long InterlockedIncrement (long* Addend)
{
  const long one = 1;
  long res;
  asm (	"	lock xadd %0,(%1)\n"
	"	incl	%0"
	: "=r" (res)
	: "r" (Addend), "0" (1)
	: "memory", "flags");
  return res;
}

static inline long InterlockedDecrement (long* Addend)
{
  long res;
  asm (	"	lock xadd %0,(%1)\n"
	"	decl	%0"
	: "=r" (res)
	: "r" (Addend), "0" (-1)
	: "memory", "flags");
  return res;
}

static inline long InterlockedExchangeAdd (long* Addend, long Value)
{
  long res;
  asm (	"	lock xadd %0,(%1)"
	: "=r" (res)
	: "r" (Addend), "0" (Value)
	: "memory", "flags");
  return res;
}

static inline long interlocked_exchange (long* destination, long exchange)
{
  long res;
  asm (	"1:	lock cmpxchg %1,(%0)"
  	"	jnz     1b\n"
	: "=a" (res)
	: "r" (destination), "r" (exchange), "a" (*destination)
	: "memory", "flags");
  return res;
}

static inline long interlocked_compare_exchange (long* destination, long exchange, long comperand)
{
  long res;
  asm (	"	lock cmpxchg %1,(%0)"
	: "=a" (res)
	: "r" (destination), "r" (exchange), "a" (comperand)
	: "memory", "flags");
  return res;
}

#endif /* __GASM_H__ */
