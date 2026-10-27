#include "jip_engine.h"
#include "jip_engine_test.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ACCEL_TRANSACTION_FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 const int n=17;const uint64_t count=4913;float *data=malloc(count*4);CHECK(data);
 for(uint64_t i=0;i<count;i++)data[i]=(float)(1+10*sin((double)i*.31)+7*cos((double)i*.13));
 jip_image image={{n,n,n},{1,1,1},{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},data,count};
 jip_options options;jip_options_default(&options);options.metric=1;options.affine_parameters[0]=.31f;
 jip_config config;jip_config_default(&config);config.stages=1;
 jip_engine *cpu=NULL,*threaded=NULL;CHECK(!jip_engine_create_configured(&image,&image,&options,&config,&cpu));CHECK(!jip_engine_create_configured(&image,&image,&options,&config,&threaded));
 CHECK(!jip_engine_set_execution(threaded,"threads",4,64));jip_engine_test_acceleration_fault(threaded,2);
 CHECK(!jip_engine_run(cpu,1)&&!jip_engine_run(threaded,1));jip_image a,b;float x,y;CHECK(!jip_engine_result(cpu,&a,&x)&&!jip_engine_result(threaded,&b,&y));CHECK(x==y&&a.count==b.count&&!memcmp(a.data,b.data,count*4));
 jip_execution info={0};info.struct_size=sizeof(info);CHECK(!jip_engine_get_execution(threaded,&info)&&info.fallbacks==1&&!strcmp(info.active,"cpu"));
 jip_engine_destroy(cpu);jip_engine_destroy(threaded);free(data);uint64_t blocks,bytes,handles,resources;jip_engine_test_live(&blocks,&bytes,&handles,&resources);CHECK(!blocks&&!bytes&&!handles&&!resources);
 puts("ACCEL_TRANSACTION_RESULT partial tiled failure recalculated exactly on CPU; accepted result and ownership pass");return 0;
}
