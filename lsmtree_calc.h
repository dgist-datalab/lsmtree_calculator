#ifndef __LSM_CALC_H__
#define __LSM_CALC_H__
#include <stdint.h>

enum Xbytes{
	N,
	K, //1024
	M, //1024K
	G, //1024M
	T, //1204G
	P //1024T
};

enum filter_option{
	NON,NORMAL,MONKEY
};

#define KB 1024
#define MB (1024L*KB)
#define GB (1024L*MB)
#define TB (1024L*GB)
#define PB (1024L*TB)

#define DEFPAGESIZE (32L*KB)
#define DEFTOTALSIZE (4L*TB)
#define DEFVALUESIZE (1L*KB)
#define DEFKEYLENGTH (32)
#define DEFLEVEL 5
#define DEFMEMORY (DEFTOTALSIZE/KB)
#define DEFFPR (1.0f)

#define GETNRUN(arg) \
	arg.total_size/(arg.page_size/(arg.key_length+4)*arg.value_size)
	

typedef struct lsmtree_argument{
	uint64_t Nruns;
	uint32_t Nlevels;
	uint32_t Nclevels;
	float sizefactor;
	float last_level_size_factor;
	float fpr;
	uint64_t total_memory;
	uint32_t graph_target;

	uint32_t page_size;
	uint32_t key_length;
	uint32_t value_size;
	uint64_t total_size;

	uint64_t bf_memory;
	uint64_t cache_memory;
	uint64_t meta_memory;
	
	char overmemory;
	uint8_t bf_option; 
	uint32_t level_run[100];
}lsm_arg;

lsm_arg* lsmtree_init(int argc, char**argv);
lsm_arg* lsmtree_init_from_console();
void lsmtree_print_info(lsm_arg *);

#endif
