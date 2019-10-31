#include "lsmtree_calc.h"

int main(int argc, char *argv[]){
	if(argc<2){
		while(1){
			lsm_arg *la=lsmtree_init_from_console();
			lsmtree_print_info(la);
		}
	}
	else{
		lsm_arg *la=lsmtree_init(argc,argv);
		lsmtree_print_info(la);
	}
}
