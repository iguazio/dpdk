
/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2015 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#include <rte_config.h>
#include <rte_common.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_timer.h>

#include "lthread_api.h"
#include "lthread_diag_api.h"
#include "pthread_shim.h"

#define DEBUG_APP 0
#define HELLOW_WORLD_MAX_LTHREADS 10

__thread int print_count;
__thread pthread_mutex_t print_lock;

__thread pthread_mutex_t exit_lock;
__thread pthread_cond_t exit_cond;

/*
 * A simple thread that demonstrates use of a mutex, a condition
 * variable, thread local storage, explicit yield, and thread exit.
 *
 * The thread uses a mutex to protect a shared counter which is incremented
 * and then it waits on condition variable before exiting.
 *
 * The thread argument is stored in and retrieved from TLS, using
 * the pthread key create, get and set specific APIs.
 *
 * The thread yields while holding the mutex, to provide opportunity
 * for other threads to contend.
 *
 * All of the pthread API functions used by this thread are actually
 * resolved to corresponding lthread functions by the pthread shim
 * implemented in pthread_shim.c
 */
void *helloworld_pthread(void *arg);
void *helloworld_pthread(void *arg)
{
	pthread_key_t key;

	/* create a key for TLS */
	pthread_key_create(&key, NULL);

	/* store the arg in TLS */
	pthread_setspecific(key, arg);

	/* grab lock and increment shared counter */
	pthread_mutex_lock(&print_lock);
	print_count++;

	/* yield thread to give opportunity for lock contention */
	pthread_yield();

	/* retrieve arg from TLS */
	uint64_t thread_no = (uint64_t) pthread_getspecific(key);

	printf("Hello - lcore = %d count = %d thread_no = %d thread_id = %p\n",
			sched_getcpu(),
			print_count,
			(int) thread_no,
			(void *)pthread_self());

	/* release the lock */
	pthread_mutex_unlock(&print_lock);

	/*
	 * wait on condition variable
	 * before exiting
	 */
	pthread_mutex_lock(&exit_lock);
	pthread_cond_wait(&exit_cond, &exit_lock);
	pthread_mutex_unlock(&exit_lock);

	/* exit */
	pthread_exit((void *) thread_no);
}


/*
 * This is the initial thread
 *
 * It demonstrates pthread, mutex and condition variable creation,
 * broadcast and pthread join APIs.
 *
 * This initial thread must always start life as an lthread.
 *
 * This thread creates many more threads then waits a short time
 * before signalling them to exit using a broadcast.
 *
 * All of the pthread API functions used by this thread are actually
 * resolved to corresponding lthread functions by the pthread shim
 * implemented in pthread_shim.c
 *
 * After all threads have finished the lthread scheduler is shutdown
 * and normal pthread operation is restored
 */
__thread pthread_t tid[HELLOW_WORLD_MAX_LTHREADS];

static void initial_lthread(void *args);
static void initial_lthread(void *args __attribute__((unused)))
{
	int lcore = (int) rte_lcore_id();
	/*
	 *
	 * We can now enable pthread API override
	 * and start to use the pthread APIs
	 */
	pthread_override_set(1);

	uint64_t i;

	/* initialize mutex for shared counter */
	print_count = 0;
	pthread_mutex_init(&print_lock, NULL);

	/* initialize mutex and condition variable controlling thread exit */
	pthread_mutex_init(&exit_lock, NULL);
	pthread_cond_init(&exit_cond, NULL);

	/* spawn a number of threads */
	for (i = 0; i < HELLOW_WORLD_MAX_LTHREADS; i++) {

		/*
		 * Not strictly necessary but
		 * for the sake of this example
		 * use an attribute to pass the desired lcore
		 */
		pthread_attr_t attr;
		cpu_set_t cpuset;

		CPU_ZERO(&cpuset);
		CPU_SET(lcore, &cpuset);
		pthread_attr_init(&attr);
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

		/* create the thread */
		pthread_create(&tid[i], &attr, helloworld_pthread, (void *) i);
	}

	/* wait for 1s to allow threads
	 * to block on the condition variable
	 * N.B. nanosleep() is resolved to lthread_sleep()
	 * by the shim.
	 */
	struct timespec time;

	time.tv_sec = 1;
	time.tv_nsec = 0;
	nanosleep(&time, NULL);

	/* wake up all the threads */
	pthread_cond_broadcast(&exit_cond);

	/* wait for them to finish */
	for (i = 0; i < HELLOW_WORLD_MAX_LTHREADS; i++) {

		uint64_t thread_no;

		pthread_join(tid[i], (void *) &thread_no);
		if (thread_no != i)
			printf("error on thread exit\n");
	}

	/* shutdown the lthread scheduler */
	lthread_scheduler_shutdown(rte_lcore_id());
	lthread_detach();
}



/* This thread creates a single initial lthread
 * and then runs the scheduler
 * An instance of this thread is created on each thread
 * in the core mask
 */
static int
lthread_scheduler(void *args);
static int
lthread_scheduler(void *args __attribute__((unused)))
{
	/* create initial thread  */
	struct lthread *lt;

	lthread_create(&lt, -1, initial_lthread, (void *) NULL);

	/* run the lthread scheduler */
	lthread_run();

	/* restore genuine pthread operation */
	pthread_override_set(0);
	return 0;
}

int main(int argc, char **argv)
{
	int num_sched = 0;

	/* basic DPDK initialization is all that is necessary to run lthreads*/
	int ret = rte_eal_init(argc, argv);

	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL parameters\n");

	/* enable timer subsystem */
	rte_timer_subsystem_init();

#if DEBUG_APP
	lthread_diagnostic_set_mask(LT_DIAG_ALL);
#endif

	/* create a scheduler on every core in the core mask
	 * and launch an initial lthread that will spawn many more.
	 */
	unsigned lcore_id;

	for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
		if (rte_lcore_is_enabled(lcore_id))
			num_sched++;
	}

	/* set the number of schedulers, this forces all schedulers synchronize
	 * before entering their main loop
	 */
	lthread_num_schedulers_set(num_sched);

	/* launch all threads */
	rte_eal_mp_remote_launch(lthread_scheduler, (void *)NULL, CALL_MASTER);

	/* wait for threads to stop */
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		rte_eal_wait_lcore(lcore_id);
	}
	return 0;
}
