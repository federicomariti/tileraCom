#ifndef __ERROR_HANDLER_H__
#define __ERROR_HANDLER_H__
#include <string.h>
#include <errno.h>

#define ERRHAND_NN(TODO)			\
  if (NULL == (TODO)) {				\
    fprintf(stderr, __FILE__":%d %s\n  %s\n",	\
	    __LINE__, #TODO, strerror(errno));	\
    exit(EXIT_FAILURE); }			\

#define ERRHAND_NZ(TODO)			\
  if (0 != (TODO)) {				\
    fprintf(stderr, __FILE__":%d %s\n  %s\n",	\
	    __LINE__, #TODO, strerror(errno));	\
    exit(EXIT_FAILURE); }			\

#define ERRHAND(TODO)				\
  if (-1 == (TODO)) {				\
    fprintf(stderr, __FILE__":%d %s\n  %s\n",	\
	    __LINE__, #TODO, strerror(errno));	\
    exit(EXIT_FAILURE); }

#define ERRHAND_TILERA(TODO)			\
  if (-1 == (TODO)) {				\
    fprintf(stderr, __FILE__":%d %s\n  %s\n",	\
	    __LINE__, #TODO, strerror(errno));	\
    tmc_task_die(#TODO); }

#define ERRHAND_TILERA_NN(TODO)			\
  if (NULL == (TODO)) {				\
    fprintf(stderr, __FILE__":%d %s\n  %s\n",	\
	    __LINE__, #TODO, strerror(errno));	\
    tmc_task_die(#TODO); }

#endif /* __ERROR_HANDLER_H__ */ 
