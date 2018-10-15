/*****************************************************************************
 * RRDtool 1.2.10  Copyright by Tobi Oetiker, 1997-2005
 *****************************************************************************
 * rrd_info  Get Information about the configuration of an RRD
 *****************************************************************************/

#include "rrd_tool.h"
#include "rrd_rpncalc.h"
#include <stdarg.h>

/* proto */
info_t *rrd_info(int, char **);
info_t *rrd_info_r(char *filename);

/* allocate memory for string */
char *
sprintf_alloc(char *fmt, ...) {
#ifdef HAVE_VSNPRINTF    
    int maxlen = 50;
#else
    int maxlen = 1000;
#endif
    char *str = NULL;
    va_list argp;
    str = malloc(sizeof(char)*(strlen(fmt)+maxlen));
    if (str != NULL) {
	va_start(argp, fmt);
#ifdef HAVE_VSNPRINTF
	vsnprintf(str, maxlen-1, fmt, argp);
#else
	vsprintf(str, fmt, argp);
#endif
    }
    va_end(argp);
    return str;
}

/* the function formerly known as push was renamed info_push because
 * it is now used outside the scope of this file */
info_t 
*info_push(info_t *info, char *key, enum info_type type, infoval value){
    info_t *next;
    next = malloc(sizeof(*next));
    next->next = (info_t *) 0;
    if( info )
	info->next = next;
    next->type = type;
    next->key  = key;
    switch (type) {
    case RD_I_VAL:
	next->value.u_val = value.u_val;
	break;
    case RD_I_CNT:
	next->value.u_cnt = value.u_cnt;
	break;
    case RD_I_INT:
	next->value.u_int = value.u_int;
	break;
    case RD_I_STR:
	next->value.u_str = malloc(sizeof(char)*(strlen(value.u_str)+1));
	strcpy(next->value.u_str,value.u_str);
	break;
    }
    return(next);
}

#ifndef DISABLE_INFO

info_t *
rrd_info(int argc, char **argv) {   
    info_t             *info;

    if(argc < 2){
        rrd_set_error("please specify an rrd");
        return NULL;
    }

    info = rrd_info_r(argv[1]);

    return(info);
}


  
info_t *
rrd_info_r(char *filename) {   
    unsigned int i,ii=0;
    FILE         *in_file;
    rrd_t        rrd;
    info_t       *data,*cd;
    infoval      info;
	enum cf_en   current_cf;
	enum dst_en  current_ds;

    if(rrd_open(filename,&in_file,&rrd, RRD_READONLY)==-1){
	return(NULL);
    }
    fclose(in_file);

    info.u_str=filename;
    cd=info_push(NULL,sprintf_alloc("filename"),    RD_I_STR, info);
    data=cd;

    info.u_str=rrd.stat_head->version;
    cd=info_push(cd,sprintf_alloc("rrd_version"),    RD_I_STR, info);

    info.u_cnt=rrd.stat_head->pdp_step;
    cd=info_push(cd,sprintf_alloc("step"),       RD_I_CNT, info);

    info.u_cnt=rrd.live_head->last_up;
    cd=info_push(cd,sprintf_alloc("last_update"), RD_I_CNT, info);

    for(i=0;i<rrd.stat_head->ds_cnt;i++){

	info.u_str=rrd.ds_def[i].dst;
	cd=info_push(cd,sprintf_alloc("ds[%s].type",             rrd.ds_def[i].ds_nam), RD_I_STR, info);
  
	current_ds = dst_conv(rrd.ds_def[i].dst);
    switch (current_ds) {
	   case DST_CDEF:
		  {
		  char *buffer = 0;
		  rpn_compact2str((rpn_cdefds_t *) &(rrd.ds_def[i].par[DS_cdef]),
			 rrd.ds_def, &buffer);
		  info.u_str = buffer;
		  cd=info_push(cd,sprintf_alloc("ds[%s].cdef",rrd.ds_def[i].ds_nam),RD_I_STR,info);
		  free(buffer);
		  }
		  break;
	   default:
	   info.u_cnt=rrd.ds_def[i].par[DS_mrhb_cnt].u_cnt;
	   cd=info_push(cd,sprintf_alloc("ds[%s].minimal_heartbeat",rrd.ds_def[i].ds_nam), RD_I_CNT, info);

	   info.u_val=rrd.ds_def[i].par[DS_min_val].u_val;
	   cd=info_push(cd,sprintf_alloc("ds[%s].min",rrd.ds_def[i].ds_nam), RD_I_VAL, info);
	
	   info.u_val=rrd.ds_def[i].par[DS_max_val].u_val;
	   cd=info_push(cd,sprintf_alloc("ds[%s].max",rrd.ds_def[i].ds_nam), RD_I_VAL, info);
	   break;
	}
	
	info.u_str=rrd.pdp_prep[i].last_ds;
	cd=info_push(cd,sprintf_alloc("ds[%s].last_ds",          rrd.ds_def[i].ds_nam), RD_I_STR, info);

	info.u_val=rrd.pdp_prep[i].scratch[PDP_val].u_val;
        cd=info_push(cd,sprintf_alloc("ds[%s].value",            rrd.ds_def[i].ds_nam), RD_I_VAL, info);

	info.u_cnt=rrd.pdp_prep[i].scratch[PDP_unkn_sec_cnt].u_cnt;
	cd=info_push(cd,sprintf_alloc("ds[%s].unknown_sec",      rrd.ds_def[i].ds_nam), RD_I_CNT, info);
    }

    for(i=0;i<rrd.stat_head->rra_cnt;i++){
	info.u_str=rrd.rra_def[i].cf_nam;
	cd=info_push(cd,sprintf_alloc("rra[%d].cf",         i),  RD_I_STR,   info);
	current_cf = cf_conv(rrd.rra_def[i].cf_nam);

	info.u_cnt=rrd.rra_def[i].row_cnt;
	cd=info_push(cd,sprintf_alloc("rra[%d].rows",i),  RD_I_CNT,   info);

	info.u_cnt=rrd.rra_def[i].pdp_cnt;
	cd=info_push(cd,sprintf_alloc("rra[%d].pdp_per_row",i),  RD_I_CNT,   info);

	switch(current_cf)
	{
	   case CF_HWPREDICT:
		  info.u_val=rrd.rra_def[i].par[RRA_hw_alpha].u_val;
		  cd=info_push(cd,sprintf_alloc("rra[%d].alpha",i),RD_I_VAL,info);
		  info.u_val=rrd.rra_def[i].par[RRA_hw_beta].u_val;
		  cd=info_push(cd,sprintf_alloc("rra[%d].beta",i),RD_I_VAL,info);
		  break;
	   case CF_SEASONAL:
	   case CF_DEVSEASONAL:
		  info.u_val=rrd.rra_def[i].par[RRA_seasonal_gamma].u_val;
		  cd=info_push(cd,sprintf_alloc("rra[%d].gamma",i),RD_I_VAL,info);
		  break;
	   case CF_FAILURES:
		  info.u_val=rrd.rra_def[i].par[RRA_delta_pos].u_val;
		  cd=info_push(cd,sprintf_alloc("rra[%d].delta_pos",i),RD_I_VAL,info);
		  info.u_val=rrd.rra_def[i].par[RRA_delta_neg].u_val;
		  cd=info_push(cd,sprintf_alloc("rra[%d].delta_neg",i),RD_I_VAL,info);
		  info.u_cnt=rrd.rra_def[i].par[RRA_failure_threshold].u_cnt;
		  cd=info_push(cd,sprintf_alloc("rra[%d].failure_threshold",i),RD_I_CNT,info);
		  info.u_cnt=rrd.rra_def[i].par[RRA_window_len].u_cnt;
		  cd=info_push(cd,sprintf_alloc("rra[%d].window_length",i),RD_I_CNT,info);
		  break;
	   case CF_DEVPREDICT:
		  break;
	   default:
		  info.u_val=rrd.rra_def[i].par[RRA_cdp_xff_val].u_val;
		  cd=info_push(cd,sprintf_alloc("rra[%d].xff",i),RD_I_VAL,info);
		  break;
	}

	for(ii=0;ii<rrd.stat_head->ds_cnt;ii++){
        switch(current_cf)
		{
		case CF_HWPREDICT:
	    info.u_val=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_hw_intercept].u_val;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].intercept",i,ii), RD_I_VAL, info);
	    info.u_val=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_hw_slope].u_val;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].slope",i,ii), RD_I_VAL, info);
	    info.u_cnt=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_null_count].u_cnt;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].NaN_count",i,ii), RD_I_CNT, info);
		   break;
		case CF_SEASONAL:
	    info.u_val=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_hw_seasonal].u_val;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].seasonal",i,ii), RD_I_VAL, info);
		   break;
		case CF_DEVSEASONAL:
	    info.u_val=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_seasonal_deviation].u_val;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].deviation",i,ii), RD_I_VAL, info);
		   break;
		case CF_DEVPREDICT:
		   break;
		case CF_FAILURES:
		   {
			  unsigned short j;
			  char *violations_array;
			  char history[MAX_FAILURES_WINDOW_LEN+1];
			  violations_array = (char*) rrd.cdp_prep[i*rrd.stat_head->ds_cnt +ii].scratch;
			  for (j = 0; j < rrd.rra_def[i].par[RRA_window_len].u_cnt; ++j)
				 history[j] = (violations_array[j] == 1) ? '1' : '0';
		      history[j] = '\0';
		      info.u_str = history;
			  cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].history",i,ii), RD_I_STR, info);
		   }
		   break;
		default:
	    info.u_val=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_val].u_val;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].value",i,ii), RD_I_VAL, info);
	    info.u_cnt=rrd.cdp_prep[i*rrd.stat_head->ds_cnt+ii].scratch[CDP_unkn_pdp_cnt].u_cnt;
	    cd=info_push(cd,sprintf_alloc("rra[%d].cdp_prep[%d].unknown_datapoints",i,ii), RD_I_CNT, info);
		   break;
        }
    }
	}
	rrd_free(&rrd);
    return(data);

}
#endif /* DISABLE_INFO */
