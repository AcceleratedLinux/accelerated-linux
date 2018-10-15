/*
 * CPU/Memory load 
 *
 * strip from matchbox-panel
 * 
 * YYHuang @Ralink
 */
#include <stdlib.h>
#include <stdio.h>

#define CPU_MEM_RRD	"/var/cpu_mem.rrd"

#define SAMPLE		2
#define RRDTOOL		"/bin/rrdtool"

#define u_int64_t unsigned long long

struct {
   /* cpu data  */
   int loadIndex;
   int samples;
   u_int64_t *load, *total;

   /* memory data  */
   u_int64_t mem_used;
   u_int64_t mem_max;
   u_int64_t swap_used;
   u_int64_t swap_max;
   unsigned int swap_percent;  /* swap used, in percent */
   unsigned int mem_percent;   /* memory used, in percent */
} msd;

int system_cpu(void)
{
    unsigned int cpuload;
    u_int64_t load, total, oload, ototal;
    u_int64_t ab, ac, ad, ae;
    int i;
    FILE *stat;

    if ((stat = fopen("/proc/stat", "r")) == NULL) {
        fprintf(stderr, "failed to open /proc/stat. Exiting\n");
        exit(1);
	}

    fscanf(stat, "%*s %Ld %Ld %Ld %Ld", &ab, &ac, &ad, &ae);
    fclose(stat);

    /* Find out the CPU load */
    /* user + sys = load
     * total = total */
    load = ab + ac + ad;        /* cpu.user + cpu.sys; */
    total = ab + ac + ad + ae;        /* cpu.total; */

    /* "i" is an index into a load history */
    i = msd.loadIndex;
    oload = msd.load[i];
    ototal = msd.total[i];

    msd.load[i] = load;
    msd.total[i] = total;
    msd.loadIndex = (i + 1) % msd.samples;

    if (ototal == 0)        
        cpuload = 0;
    else
        cpuload = (100 * (load - oload)) / (total - ototal);

    return cpuload;
}

int system_memory(void)
{
    u_int64_t my_mem_used, my_mem_max;
    u_int64_t my_swap_max;

    static int mem_delay = 0;
    FILE *mem;
    static u_int64_t total, used, mfree, shared, buffers, cached,
      cache_total, cache_used;

    /* put this in permanent storage instead of stack */
    static char not_needed[1024];

    if (mem_delay-- <= 0) {
      if ((mem = fopen("/proc/meminfo", "r")) == NULL)
    {
      fprintf(stderr, "failed to open /proc/meminfo.\n");
      exit(1);
      }

    fgets(not_needed, 1024, mem);
    /*
        total:    used:    free:  shared: buffers:  cached:
    */
    fscanf(mem, "%*s %Ld %Ld %Ld %Ld %Ld %Ld", &total, &used, &mfree,
           &shared, &buffers, &cached);

    fscanf(mem, "%*s %Ld %Ld", &cache_total, &cache_used);
    fclose(mem);
    mem_delay = 25;

    /* calculate it */
    my_mem_max = total;
    my_swap_max = cache_total;

    my_mem_used = cache_used + used - cached - buffers;

    msd.mem_used = my_mem_used;
    msd.mem_max = my_mem_max;

    msd.mem_percent = (100 * msd.mem_used) / msd.mem_max;
    /* memory info changed - update things */
    return 1;
    }
	return 0;
}

void cpu_mem_init()
{
	int i;
	u_int64_t load = 0, total = 0;

	msd.samples = SAMPLE;
	if (msd.load) {
		load = msd.load[msd.loadIndex];
		free(msd.load);
	}
	if (msd.total) {
		total = msd.total[msd.loadIndex];
		free(msd.total);
	}

	msd.loadIndex = 0;
	msd.load = malloc(msd.samples * sizeof(u_int64_t));
	msd.total = malloc(msd.samples * sizeof(u_int64_t));
	for (i = 0; i < msd.samples; i++) {
		msd.load[i] = load;
		msd.total[i] = total;
	}
}

void create_cpu_mem_rrd(void)
{
	FILE *fp;
	char cmd[1024];

	if((fp = fopen(CPU_MEM_RRD, "rb"))){
		fclose(fp);
		return;
	}

	snprintf(cmd, 1024, "%s create %s  -s 3 \
DS:cpu:GAUGE:6:0:10000000 \
DS:mem:GAUGE:6:0:10000000 \
RRA:AVERAGE:0.5:1:144	\
RRA:AVERAGE:0.5:6:48		\
RRA:AVERAGE:0.5:24:12		\
RRA:AVERAGE:0.5:288:6", RRDTOOL, CPU_MEM_RRD);
	system(cmd);
}


void update_cpu_mem_rrd(int cpu, int mem)
{
	char cmd[1024];
	snprintf(cmd, 1024, "%s update %s N:%d:%d", RRDTOOL, CPU_MEM_RRD, cpu, mem);
	system(cmd);
}


int main(void)
{
	// init cpu mem usage functions
	cpu_mem_init();

	// create cpu/mem RRD Table
	create_cpu_mem_rrd();

	while(1){
		system_memory();
		update_cpu_mem_rrd( system_cpu(), msd.mem_percent);
 		sleep(1);
	}

	return 0;
}
