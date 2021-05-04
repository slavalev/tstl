/*****************************************************************************************************//**
 *
 *  Module Name:	\file tstl_test.cpp
 *
 *  Abstract:		\brief TSTL multimap testing application.
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

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <locale.h>
#include <windows.h>

#include "sysiolib.h"

//#include "tsmap.hpp"
#include "tstl.hpp"

#define ITEMS_NUMBER		12000000

#define INSERTER_THREADS_NUMBER	4
#define INDEXER_THREADS_NUMBER	INSERTER_THREADS_NUMBER

using namespace tstl;
using namespace tstl_test;

//struct test_destructor { ~test_destructor () { brk (); } };

//typedef multimap <long, test_destructor> tsm, *ptsm;
typedef multimap <long, long> tsm, *ptsm;

static bool test_with_delay = false;
static volatile long test_exit = 0;
static volatile long items_in_map = 0;

static long items_number = ITEMS_NUMBER;
static long indexer_threads_number = INDEXER_THREADS_NUMBER;
static long inserter_threads_number = INSERTER_THREADS_NUMBER;

static void add_delete_thread (void* context)
{
	if (!context)
	{	brk (); atomic_inc_return ((long*) & test_exit); return; }

	ptsm pmap = (ptsm) context;

	static volatile long key_base = 0;
	long key_start = atomic_inc_return ( (long*) & key_base);

	if (1 == key_start)	printf ("\n");

	long print_cnt = 0;

	mp pos;

	// test_destructor test_destr;

	for (long key = key_start - 1; key <= items_number; key += inserter_threads_number)
	{
	//	if (pmap->set_at (pos, key, & test_destr) )
		if (pmap->set_at (pos, key, & key) )
		{
			pmap->release (pos);
			atomic_inc ( (long*) & items_in_map);
		}

		if (!(print_cnt++ & 0xFFF) )
			printf ("\rInserting item %0d", key);

		if (test_with_delay)
			ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	}

	/// syncronization point
	static volatile long ready_for_delete_threads = 0;
	atomic_inc ( (long*) & ready_for_delete_threads);

	while (inserter_threads_number != ready_for_delete_threads)
	{ ts_sleep (TS_SPINLOCK_SLEEP_TIME); }

	if (1 == key_start)	printf ("\n");

	print_cnt = 0;

	for (long key = key_start - 1; key <= items_number; key += inserter_threads_number)
	{
		if (pmap->remove_by_key (key) )
			atomic_dec ( (long*) & items_in_map);
		else
			printf ("\nItem %d doesn't found in map!!!\n", key);

		if (!(print_cnt++ & 0xFFF) )
			printf ("\rDeleting item %6d", key);

		if (test_with_delay) ts_sleep (TS_SPINLOCK_SLEEP_TIME);
	}

	/// syncronization point
	static volatile long ready_for_stop_threads = 0;
	atomic_inc ( (long*) & ready_for_stop_threads);

	while (inserter_threads_number != ready_for_stop_threads)
	{ ts_sleep (TS_SPINLOCK_SLEEP_TIME); }

	long thread_id = atomic_inc_return ((long*) & test_exit);
	printf ("\nInserting thread %d completed.", thread_id);
}

static void indexer_thread (void* context)
{
	if (!context)
	{	brk (); atomic_inc_return ( (long*) & test_exit); return; }

	ptsm pmap = (ptsm) context;

	mp pos;
	long key;
	long *value;
	//test_destructor *value;
	size_t hash;

	while (test_exit)
	{
		if (!pmap->start (pos, key, hash, value) )
		{	atomic_inc_return ((long*) & test_exit); return; }

		bool rc = true;

		do
		{
			rc = pmap->next (pos, key, hash, value);

			if (test_exit)
			{
				if (rc)
					pmap->release (pos);

				break;
			}

			if (test_with_delay) ts_sleep (TS_SPINLOCK_SLEEP_TIME);
		}
		while (rc);
	}

	long thread_id = atomic_inc_return ((long*) & test_exit);
	// printf ("\nIndexing thread %d completed.", thread_id);
}

static bool create_threads (pthread_ctx* pctxs, long num_ctxs, type_thread_routine thread_routine, void* context)
{
	if (!pctxs || !num_ctxs || !thread_routine)
		return false;

	while (num_ctxs-- > 0)
		create_thread (*pctxs++, thread_routine, context);

	return true;
}

static bool delete_threads (pthread_ctx* pctxs, long num_ctxs)
{
	if (!pctxs || !num_ctxs)
		return false;

	while (num_ctxs-- > 0)
		clean_thread_info (*pctxs++);

	return true;
}

static void stop_threads ()
{
	printf ("Stoping threads.");

	atomic_inc ((long*) & test_exit); ///< send exit signal to threads

	/// wait for all threads
	while (inserter_threads_number +
		   indexer_threads_number  + 1 > test_exit)
	{	ts_sleep (TS_SPINLOCK_SLEEP_TIME); printf ("."); }

	printf ("\n");
}

static void exit_sighandler (int x)
{
	stop_threads ();
	exit (EXIT_FAILURE);
}

static void signals_hook (void)
{
	///========= Catch terminate signals: ================
	/// terminate requests:
	signal (SIGTERM, exit_sighandler);	///< kill
//	signal (SIGHUP,  exit_sighandler);	///< kill -HUP  /  xterm closed
	signal (SIGINT,  exit_sighandler);	///< Interrupt from keyboard
//	signal (SIGQUIT, exit_sighandler);	///< Quit from keyboard
  
	/// fatal errors:
//	signal (SIGBUS,  exit_sighandler);	///< bus error
	signal (SIGSEGV, exit_sighandler);	///< segfault
	signal (SIGILL,  exit_sighandler);	///< illegal instruction
	signal (SIGFPE,  exit_sighandler);	///< floating point exc.
	signal (SIGABRT, exit_sighandler);	///< abort ()
}

int wmain (int argc, wchar_t* argv [], wchar_t* envp [])
{
	setlocale (LC_ALL, ".OCP");

	printf ("TSTL %d.%d.%d testing application. 2009y.\n", TSTL_VERSION, TSTL_PATCHLEVEL, TSTL_SUBLEVEL);

	wprintf (L"usage: tstl_test [items number: %d] [inserter threads number: %d] [indexer threads number: %d]\n",
		 items_number, inserter_threads_number, indexer_threads_number);

	/// parse command line arguments
	if (argc < 2
	 || !(items_number = wcstol (argv [1], NULL, 0) ) )
		printf ("Used default items number: %d\n", items_number = ITEMS_NUMBER);
	else
		printf ("Map items number: %d\n", items_number);

	if (argc < 3
	|| !(inserter_threads_number = wcstol (argv [2], NULL, 0) ) )
		printf ("Used default inserter threads number: %d\n", inserter_threads_number = INSERTER_THREADS_NUMBER);
	else
		printf ("Inserter threads number: %d\n", inserter_threads_number);

	if (argc < 4
	 || !(indexer_threads_number = wcstol (argv [3], NULL, 0) ) )
		printf ("Used default indexer threads number: %d\n", indexer_threads_number = INDEXER_THREADS_NUMBER);
	else
		printf ("Indexer threads number: %d\n", indexer_threads_number);

	/// test prequeue second phase template instance
	// prequeue <long>* ppq = new prequeue <long>;
	// if (ppq) delete (ppq), ppq = NULL;
	/// test prequeue second phase template instance

	/// test limit cache second phase template instance
	// limit_cache <wchar_t*, long>* plc = new limit_cache <wchar_t*, long>;
	// if (plc) delete (plc), plc = NULL;
	/// test limit cache second phase template instance

	/// test limit cache second phase template instance
	// cache <wchar_t*, long>* pc = new cache <wchar_t*, long>;
	// if (pc) delete (pc), pc = NULL;
	/// test limit cache second phase template instance

	ptsm pmap = NULL;

	if (!init_map (pmap, items_number / 8) )
	{	printf ("\tCann't initialyze map.\n"); return EXIT_FAILURE; }

	test_exit = 0;

	pthread_ctx* add_delete_ctxs = new pthread_ctx [inserter_threads_number];

	if (!add_delete_ctxs
	 || !create_threads (add_delete_ctxs, inserter_threads_number, add_delete_thread, pmap))
	{
		printf ("\tCann't create adding/deleting threads.\n");
	 	atomic_inc ((long*) & test_exit);

		if (add_delete_ctxs) delete [] add_delete_ctxs, add_delete_ctxs = NULL;
		delete (pmap), pmap = NULL;
	 	return 2;
	}

	pthread_ctx* indexer_ctxs = new pthread_ctx [indexer_threads_number];

	if (!indexer_ctxs
	 || !create_threads (indexer_ctxs, indexer_threads_number, indexer_thread, pmap))
	{
		printf ("\tCann't create indexing threads.\n");
	 	atomic_inc ((long*) & test_exit);

	 	delete [] add_delete_ctxs, add_delete_ctxs = NULL;
	 	if (indexer_ctxs) delete [] indexer_ctxs, indexer_ctxs = NULL;
		delete (pmap), pmap = NULL;
	 	return 3;
	}

	signals_hook ();

	while (!_kbhit () ) { ts_sleep (TS_SPINLOCK_SLEEP_TIME); }; /* On learn exit with Ctrl+Break or Ctrl+C */

	stop_threads ();

	printf ("Survived items in map: %d\n", items_in_map);

	delete_threads (add_delete_ctxs, inserter_threads_number);
	delete_threads (indexer_ctxs,	 indexer_threads_number);

 	delete [] add_delete_ctxs, add_delete_ctxs = NULL;
 	delete [] indexer_ctxs, indexer_ctxs = NULL;

	delete (pmap), pmap = NULL;

	return EXIT_SUCCESS;
}
