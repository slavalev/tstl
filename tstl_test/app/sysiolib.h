/*****************************************************************************************************//**
 *
 *  Module Name:	\file sysiolib.h
 *
 *  Abstract:		\brief OS specific routines definition.
 *
 *  Author:	    	\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History:	\date 10.07.2009 started
 *
 *  Classes, methods and structures: \details
 *
 *  TODO:		\todo
 *
 *********************************************************************************************************/

#ifndef __SYSIOLIB_H__
#define __SYSIOLIB_H__

namespace tstl_test {

typedef void (*type_thread_routine) (void* context);

/// thread context
typedef struct thread_ctx
{
	void* context;
	type_thread_routine routine;

	thread_ctx () : context (0), routine (0) {}
} thread_ctx, *pthread_ctx;

/// Create thread form routine and context pointers
/** \param[out] ctx
    \param[in]  routine
    \param[in]  context
    \retval ctx  */
extern bool create_thread (pthread_ctx& ctx, type_thread_routine routine, void* context);

/// Clean thread context
/** \param[in] ctx */
extern bool clean_thread_info (pthread_ctx& ctx);

}; /* namespace tstl_test */

#endif /* __SYSIOLIB_H__ */
