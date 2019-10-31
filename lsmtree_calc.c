#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <string.h>

#include "lsmtree_calc.h"
void help_print();
void lsm_arg_init(lsm_arg *);
uint64_t total_size_parser(char *);
lsm_arg* lsmtree_init(int argc, char**argv){
	lsm_arg *res=(lsm_arg*)calloc(sizeof(lsm_arg),1);
	lsm_arg_init(res);
	int c;
	char def_set=0;
	while((c=getopt(argc,argv,"Tlcfbhkvmdg"))!=-1){
		switch(c){
			case 'h':
				help_print();
				exit(1);
				break;
			case 'k':
				res->key_length=atoi(argv[optind]);
				break;
			case 'v':
				res->value_size=atoi(argv[optind]);
				break;
			case 'c':
				res->Nclevels=atoi(argv[optind]);
				break;
			case 'l':
				res->Nlevels=atoi(argv[optind]);
				break;
			case 'f':
				res->fpr=atof(argv[optind]);
				break;
			case 'T':
				res->total_size=total_size_parser(argv[optind]);
				break;
			case 'm':
				res->total_memory=total_size_parser(argv[optind]);
				break;
			case 'b':
				res->bf_option=atoi(argv[optind]);
				break;
			case 'g':
				res->graph_target=atoi(argv[optind]);
				break;
			case 'd':
				def_set=1;
				break;
		}
		if(def_set) break;
	}
	res->Nruns=GETNRUN((*res));
	return res;
}

uint64_t total_size_parser(char *input){
	uint32_t len=strlen(input);
	if(input[len-1] >='0' && input[len-1]<='9'){
		char buf[256];
		strncpy(buf,input,len-1);
		int times=0;
		switch(input[len-1]){
			case 'K': times=1; break;
			case 'M': times=2; break;
			case 'G': times=3; break;
			case 'T': times=4; break;
			case 'P': times=5; break;
		}
		uint64_t res=1;
		for(int i=0; i<times;i++){
			res*=1024;
		}
		return res*atoi(buf);
	}
	else{
		return atoi(input);
	}
}

float getsizefactor(lsm_arg *la){
	float res=ceil(pow(10,log10(la->Nruns)/(la->Nlevels)));

	la->meta_memory=la->Nruns*(la->key_length+sizeof(uint32_t));
	//la->meta_memory=290L*MB;
	float ff=0.05f;
	float tt=res;
	float original=res;
	uint64_t target=1;
	uint64_t total_runs=0;
	uint64_t memory_usage=0;
	uint64_t now_runs=0;
	uint64_t target_memory=4L*GB-la->meta_memory;
	uint64_t last_before_runs=0;
	la->overmemory=0;
	int i;

retry:
	for(i=0; i<la->Nlevels; i++){
		now_runs=ceil(target*tt);
		total_runs+=now_runs;
		la->level_run[i]=now_runs;
		if(i<la->Nclevels){
			memory_usage+=now_runs*la->page_size;
		}
		if(i==la->Nlevels-2){
			last_before_runs=now_runs;
		}
		tt*=res;
	}

	if(total_runs>la->Nruns){
		res-=ff;
		target=1;
		now_runs=0;
		tt=res;
		total_runs=0;
		memory_usage=0;
		goto retry;
	}else if(target_memory<memory_usage){
		/*
		res-=ff;
		target=1;
		now_runs=0;
		tt=res;
		total_runs=0;
		memory_usage=0;
		la->overmemory=1;
		goto retry;*/
	} 	
	else{
		res+=ff;
		uint64_t gap=la->Nruns-total_runs;
		/*
		if(gap>la->Nruns/100*5){
			res=original;
			ff-=0.005f;
			target=1;
			now_runs=0;
			tt=res;
			total_runs=0;
			memory_usage=0;
			goto retry;
		}*/
		
		if(la->overmemory){
			uint64_t remain_runs=la->Nruns-(total_runs-now_runs);
			la->last_level_size_factor=(double)remain_runs/last_before_runs;
			la->level_run[la->Nlevels-1]=remain_runs;
		}
		else{
			la->last_level_size_factor=res;
		}
	}

	la->sizefactor=res;
	la->cache_memory=memory_usage;
	return res;
}

void target_one_print(char *type,uint64_t value){
	printf("%s: ",type);
	printf("%.0lf(KB) %.2lf(MB) %.2lf(GB)\n",(double)value/KB,(double)value/MB,(double)value/GB);
}

void lsmtree_trade_graph(lsm_arg *la){
	for(int j=0; j<la->graph_target; j++){
		printf("--------pinning level %d\n",j);
		for(int i=2; i<10; i++){
			if(i-j<=0) continue;
			la->Nlevels=i;
			la->Nclevels=j;
			getsizefactor(la);
			float waf=la->sizefactor*(la->Nlevels-la->Nclevels)/(la->page_size/la->key_length);
			printf("%d %lf %d %lf\n",i,waf,i-j,la->sizefactor);
		}
	}
}

void lsmtree_print_info(lsm_arg *la){
	getsizefactor(la);
	printf("# of runs:%lu\n",la->Nruns);
	printf("Level :%u\n",la->Nlevels);
	printf("sizefactor :%lf , %lf\n",la->sizefactor,la->last_level_size_factor);
	printf("Pinned Level :%u\n",la->Nclevels);
	printf("level %u runs: %u ",0,1);
	target_one_print("memory",la->page_size);
	for(int i=0; i<la->Nlevels-1; i++){
		printf("level %u runs: %u ",i+1,la->level_run[i]);
		target_one_print("memory",(uint64_t)la->level_run[i]*la->page_size);
	}
	printf("level %u runs: %u \n",la->Nlevels,la->level_run[la->Nlevels-1]);
	target_one_print("meta_memory",la->meta_memory);
	target_one_print("cache_memory",la->cache_memory);
	target_one_print("total memory",la->meta_memory+la->cache_memory);
	float waf;
	if(la->overmemory){
		waf=la->sizefactor*(la->Nlevels-la->Nclevels-1);
		waf+=la->last_level_size_factor;
	}else{
		waf=la->sizefactor*(la->Nlevels-la->Nclevels)/(la->page_size/la->key_length);
	}
	//waf/=(la->page_size/la->key_length);
	printf("write overhead :%lf\n",waf);
	printf("read overhead: %d\n",la->Nlevels-la->Nclevels);
	if(la->graph_target){
		lsmtree_trade_graph(la);
	}
}

void help_print(){
	printf("==argument description==\n");
	printf("h: show argument description\n");
	printf("k: key-length\n");
	printf("v: value-size\n");
	printf("T: Total Size of Device\n");
	printf("m: Total memory in Device\n");
	printf("l: # of level in lsmtree\n");
	printf("c: # of cached_level in lsmtree\n");
	printf("b: bloomfilter option 0-No Filter 1-Normal 2-Monkey\n");
	printf("f: bloomfilter's fals positive ratio\n");
	printf("[*Nothing]: wait until User input the parameter\n");
}

void lsm_arg_init(lsm_arg *res){
	res->page_size=DEFPAGESIZE;
	res->total_size=DEFTOTALSIZE;
	res->key_length=DEFKEYLENGTH;
	res->value_size=DEFVALUESIZE;
	res->total_memory=DEFMEMORY;

	res->Nlevels=DEFLEVEL;
	res->fpr=DEFFPR;
}

lsm_arg* lsmtree_init_from_console(){
	uint32_t input;
	uint64_t input_ll;
	float input_fpr;
	lsm_arg *res=(lsm_arg*)calloc(sizeof(lsm_arg),1);
	lsm_arg_init(res);
	printf("==User input parameter==\n");
	printf("insert key-length (0 is default value)\n");
	scanf("%d",&input);
	if(input){
		res->key_length=input;
	}
	printf("insert value-size (0 is default value)\n");
	scanf("%d",&input);
	if(input){
		res->value_size=input;
	}

	printf("insert Total Size of Device (0 is default value)\n");
	scanf("%ld",&input_ll);
	if(input){
		res->total_size=input_ll;
	}
	printf("insert Total memory in Device (0 is default value)\n");
	scanf("%ld",&input_ll);
	if(input){
		res->total_memory=input_ll;
	}

	printf("insert # of level in lsmtree (0 is default value)\n");
	scanf("%d",&input);
	if(input){
		res->Nlevels=input;
	}
	printf("insert # of cached_level in lsmtree (0 is default value)\n");
	scanf("%d",&input);
	if(input){
		res->Nclevels=input;
	}

	printf("insert bloomfilter option 0-No Filter 1-Normal 2-Monkey (0 is default value)\n");
	scanf("%d",&input);
	if(input){
		res->bf_option=input;
	}
	printf("insert bloomfilter's fals positive ratio (0 is default value)\n");
	scanf("%f",&input_fpr);
	if(input){
		res->fpr=input_fpr;
	}
	printf("[*Nothing]: wait until User input the parameter (0 is default value)\n");

	res->Nruns=GETNRUN((*res));
	return res;
}
