#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <tmc/cpus.h>
#include <tmc/task.h>
#include <tmc/perf.h>
#include <arch/cycle.h>

typedef struct {
  int rank; int cpuid; int sec_to_spin; pthread_barrier_t *barrier;
} task_spin_arg_t;

#define MAX_NUM_CPU 64

    /* verifica dataplane cpus 
    {
      task_spin_arg_t arg1 = { 0, cpu_white, 10, &computation_start };
      task_spin_arg_t arg2 = { 1, cpu_black, 10, &computation_start };

      pthread_create(&thread_white, NULL, task_spin, (void *)&arg1);
      pthread_create(&thread_black, NULL, task_spin, (void *)&arg2);
      pthread_join(thread_white, NULL);
      pthread_join(thread_black, NULL);
    }
    */

void *task_spin(void *);

int
main()
{
  int num_dp;
  int i;
  task_spin_arg_t arg[MAX_NUM_CPU];
  pthread_t thread[MAX_NUM_CPU];
  pthread_barrier_t computation_start;
  cpu_set_t dp;

  tmc_cpus_get_dataplane_cpus(&dp);
  num_dp = tmc_cpus_count(&dp);

  pthread_barrier_init(&computation_start, NULL, num_dp);

  for (i=0; i<num_dp; i++) {
    arg[i].rank = i;
    arg[i].cpuid = i;
    arg[i].sec_to_spin = 10;
    arg[i].barrier = &computation_start;
    pthread_create(thread+i, NULL, task_spin, arg+i);
  }
  
  for (i=0; i<num_dp; i++)
    pthread_join(thread[i], NULL);

  return 0;
}


#define TASK_SPIN_THRESHOLD 2000

void *task_spin(void *arg) {
  task_spin_arg_t *     _arg = (task_spin_arg_t *)arg;
  uint64_t              hertz;
  uint64_t              start, end, now, prev, base;
  unsigned int          hiccpu=0;

  hertz = tmc_perf_get_cpu_speed();
  if (-1 == tmc_cpus_set_my_cpu(_arg->cpuid)) {
    tmc_task_die("tmc_cpus_set_my_cpu");
  }

  //printf("task_%d: start on cpu %d\n", _arg->rank, tmc_cpus_get_my_cpu());

  pthread_barrier_wait(_arg->barrier);

  base = get_cycle_count();
  start = base + hertz/5;
  prev = base;
  end = base + hertz * _arg->sec_to_spin;

  while ((now = get_cycle_count()) < end) {
    if (now - prev > TASK_SPIN_THRESHOLD) {
      hiccpu = hiccpu + 1;
      if (now > start) {
        printf("task_%d: numero di cicli prima dell'interruzione = %lld, "
	       "hiccpu=%d\n", _arg->rank, now-prev, hiccpu);
        return NULL;
      }
    }
    prev = now;
  }

  printf("task_%d: end, hiccpu = %u\n", _arg->rank, hiccpu);


  while (get_cycle_count() < end + hertz) ;

  return NULL;
}
