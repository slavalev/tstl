# tstl
Thread Safe Template Library (TSTL) is a C++ library that provides thread-safe containers with high grade of locking. It includes classes for multimaps based on a B-tree and a hash table. It also includes classes for a queue, a priority queue, a pipe, an alloccache, a timercache, a limitcache without global locking, and a fast shared mutual exclusion locker (also known as shared lock or read write lock). It's useful for writing multi-threaded applications, network services, and operating system components with high requirements for interactivity level.

 Authors: Vyacheslav I. Levtchenko (mail-to: slavalev at gmail.com),
          Andrew Zabolotny (mail-to: anpaza at mail.ru).

 Main include file is "tstl.hpp".
 Sample file is "tstl_test\app\tstl_test.cpp".

 * Thread safe allocation cache:   "iqalloccache.hpp" - memory allocation cache based 
                                 on interlocked FIFO queue of empty memory blocks.

 * Thread safe allocation cache:   "ialloccache.hpp" - memory allocation cache based 
                                 on interlocked array of empty memory blocks.

 * Thread safe allocation cache:   "alloccache.hpp" - generic memory allocation
                                 cache based with choosable storing strategi.
                                 You can choose interlocked queue based cache
                                 or interlocked array based cache.

 * Thread safe multimap:          "nbmap.hpp" - interlocked b-tree based multimap.
                                 Multimap locking granularaty is one leaf of tree 
                                 (one element of level array).

 * Thread safe multimap:          "pbmap.hpp" - hash table based multimap.
                                 Multimap locking granularaty is one linked list.

 * Thread safe multimap:          "tsmap.hpp" - generic multimap template with 
                                 choosable storing strategi. You can choose 
                                 interlocked b-tree or partialy locked hash table
                                 as template paremeter.

 * Thread safe limited cache:      "limitcache.hpp" - limited cache storage
                                 with cleanup of element by limit of storage.

 * Thread safe timer cache:        "timercache.hpp" - buble sorted cache storage
                                 with cleanup of element by timer.

 * Thread safe cache:              "tscache.hpp" - generic cache template with 
                                 choosable caching strategi. You can choose 
                                 limit cache or timer cache caching strategi
                                 as template paremeter.

 * Thread safe pipe:               "tspipe.hpp" - simple classic pipe with many
                                 or one writer and one reader. It based on 
                                 cyclo buffer and choosable mutual exclusion locker.

 * Thread safe queue:              "iqueue.hpp" - based on interlocked FIFO queue,
                                 allocation cache template. On put element tries 
                                 get memory from allocation cache.

 * Thread safe queue:              "cqueue.hpp" - based on list, allocation cache 
                                 template and mutual exclusion locker. On put 
                                 element tries get memory from allocation cache.

 * Thread safe queue:              "tsqueue.hpp" - generic queue template. Could 
                                 be parametrized by interlocked queue, classic 
                                 queue or pipe template.

 * Thread safe priority queue:     "tsprequeue.hpp" - same tsqueue, but 
                                 messages have prioritet. It is inside a map of 
                                 queues chosed via prioritet as map key.

 * Shared locker:                  "rwlocker.hpp" - variant of semaphore with 
                                 one or many writers and many readers (shared 
                                 locker). Readers share guarded object without 
                                 synchronization locking.

 * Mutual exclusion locker:        "melocker.hpp" - waitable locker template. 
                                 Could be parametrized by native mutex or 
                                 TSTL fast variant of mutex or spinlock.

 * Reenterable mutex locker:       "relocker.hpp" - reenterable version of 'melocker'.

