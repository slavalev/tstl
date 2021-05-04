/*****************************************************************************************************//**
 *
 *  Module Name:	\file sysiolib.cpp
 *
 *  Abstract:		\brief OS specific routines implementation.
 *
 *  Author:	    	\author Vyacheslav I. Levtchenko (mail-to: slavalev@gmail.com)
 *
 *  Revision History: \date 10.07.2009 started
 *
 *  Classes, methods and structures: \details
 *
 *  TODO:			\todo
 *
 *********************************************************************************************************/

#if defined (WIN32)
#  include <windows.h>
#elif defined (WINNT) && defined (DRIVER)
#  include <ntddk.h>
#else
#  error Target platform wasn't set!!!
#endif

#include "impl/tsdebug.h"

#include "sysiolib.h"

namespace tstl_test {

static bool user_thread_routine (pthread_ctx pctx)
{
	if (!pctx || !pctx->routine) { brk (); return false; }

	pctx->routine (pctx->context);
	return true;
}

#if defined (_NTDDK_)

static void NTAPI system_thread_routine (void* context)
{
	user_thread_routine ((pthread_ctx) context);

	PsTerminateSystemThread (STATUS_SUCCESS);
}

static bool system_create_thread (pthread_ctx ctx)
{
	HANDLE hThread = (HANDLE) NULL;
	PsCreateSystemThread (&hThread, 0, NULL, NULL, NULL, system_thread_routine, ctx);

	if (hThread)
	{	ZwClose (hThread), hThread = (HANDLE) NULL; return true; }

	brk ();
	return false;
}

#else

static DWORD WINAPI system_thread_routine (void* context)
{
	user_thread_routine ((pthread_ctx) context);

	ExitThread (ERROR_SUCCESS);
	return 0;
}

static bool system_create_thread (pthread_ctx ctx)
{
	HANDLE hThread = (HANDLE) NULL;
	hThread = CreateThread (NULL, 0, system_thread_routine, ctx, 0, NULL);

	if (hThread)
	{	CloseHandle (hThread), hThread = NULL; return true; }

	brk ();
	return false;
}

#endif

bool clean_thread_info (pthread_ctx& ctx)
{
	if (!ctx) { brk (); return false; }
	delete (ctx), ctx = NULL;

	return true;
}

bool create_thread (pthread_ctx& ctx, type_thread_routine routine, void* context)
{
	if (!routine) { brk (); return false; }

	ctx = new thread_ctx;
	if (!ctx) { brk (); return false; }

	ctx->routine = routine;
	ctx->context = context;

	if (system_create_thread (ctx))
		return true;

	clean_thread_info (ctx);
	return false;
}

}; /* namespace tstl_test */
