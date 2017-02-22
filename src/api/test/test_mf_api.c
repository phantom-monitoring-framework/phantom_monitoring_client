#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "resources_monitor.h"
#include "disk_monitor.h"
#include "power_monitor.h"
#include "mf_api.h"

#define SAMPLE_INTERVAL 500

/* some dummy application */
void dummy(void)
{
	int i;
//	int i, j;
	float x1, x2, result[1024*1024];
//	char *tmp;

	for (i = 0; i < 1024 * 1024; i++) {
		x1 = (float)rand()/(float)(RAND_MAX/100.0);
		x2 = (float)rand()/(float)(RAND_MAX/1.0);
		result[i] = x1 * x2;
	}
	FILE *fp = fopen("./result.dat", "w");
	for (i = 0; i < 1024 * 1024; i++) {
		fprintf(fp, "result[%d] = %.3f\n", i, result[i]);
	}
/*
	for (i = 0; i < 1024; i++) {
		tmp = (char *)malloc(sizeof(char) * i * 1024);
		for (j = 0; j < i; j++) {
			sprintf(tmp+strlen(tmp), "* ");
		}
		fprintf(fp, "%s\n", tmp);
		if(tmp != NULL)
			free(tmp);
	}
*/
	fclose(fp);
}

/*******************************************************************************
 * resources_monitor test
 ******************************************************************************/
/* test resources_stat_cpu */
void Test_resources_stat_cpu(void)
{
	int pid = getpid();
	resources_cpu cpu_stats_now;
	resources_stat_cpu(pid, &cpu_stats_now);
	printf("process %d\n\tprocess_CPU_time ---%llu\n\tglobal_CPU_time ---%llu\n", 
		pid, 
		cpu_stats_now.process_CPU_time, 
		cpu_stats_now.global_CPU_time);
	/*wait for check*/
	char c;
	scanf("%c", &c);
}

/* test resources_stat_all_and_calculate */
void Test_resources_stat_all_and_calculate(void)
{
	char c;
	int pid = getpid();
	resources_stats cpu_stats;
	resources_cpu cpu_stats_before, cpu_stats_after;

	/*stats the current cpu time of the process and of the whole system */
	resources_stat_cpu(pid, &cpu_stats_before);
	printf("process %d\n\tprocess_CPU_time ---%llu\n\tglobal_CPU_time ---%llu\n", 
		pid, 
		cpu_stats_before.process_CPU_time, 
		cpu_stats_before.global_CPU_time);
	/*wait for check*/
	scanf("%c", &c);
	
	/*do some dummy calculation */
	dummy();

	/*stats the current cpu time of the process and of the whole system */
	resources_stat_cpu(pid, &cpu_stats_after);
	printf("process %d\n\tprocess_CPU_time ---%llu\n\tglobal_CPU_time ---%llu\n", 
		pid, 
		cpu_stats_after.process_CPU_time, 
		cpu_stats_after.global_CPU_time);
	/*wait for check*/
	scanf("%c", &c);

	/*do stats of ram and swap and calculate the cpu_usage_rate for the duration */
	resources_stat_all_and_calculate(pid, &cpu_stats_before, &cpu_stats_after, &cpu_stats);
	printf("process %d\n\tCPU_usage_rate ---%.3f\n\tRAM_usage_rate ---%.3f\n\tswap_usage_rate ---%.3f\n",
		pid,
		cpu_stats.CPU_usage_rate,
		cpu_stats.RAM_usage_rate, 
		cpu_stats.swap_usage_rate);
	/*wait for check*/
	scanf("%c", &c);	
}

/* test resources_monitor */
void Test_resources_monitor(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 1;
	m_resources.sampling_interval[0] = SAMPLE_INTERVAL; // 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");

	char *datapath = mf_start(&m_resources);
	printf("datapath is :%s\n", datapath);
	sleep(5);
	
	/*do dummy things*/
	for(i = 0; i < 10; i++) {
		dummy();
		sleep(2);
	}

	mf_end();
}

/*******************************************************************************
 * disk_monitor test
 ******************************************************************************/
/* test disk_stats_read */
void Test_disk_stats_read(void)
{
	char c;
	int pid = getpid();
	disk_stats disk_info;

	disk_info.read_bytes_after = 0;
	disk_info.write_bytes_after = 0;

	disk_stats_read(pid, &disk_info);

	printf("process %d\n\tread_bytes_before ---%llu\n\twrite_bytes_before ---%llu\n", 
		pid, 
		disk_info.read_bytes_before,
		disk_info.write_bytes_before);
	/*wait for check*/
	scanf("%c", &c);

	dummy();

	disk_stats_read(pid, &disk_info);

	printf("process %d\n\tread_bytes_after ---%llu\n\twrite_bytes_after ---%llu\n", 
		pid, 
		disk_info.read_bytes_after,
		disk_info.write_bytes_after);
	/*wait for check*/
	scanf("%c", &c);
}

/* test disk_monitor */
void Test_disk_monitor(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 1;
	m_resources.sampling_interval[0] = SAMPLE_INTERVAL; // 1s
	strcpy(m_resources.metrics_names[0], "disk_io");

	char *datapath = mf_start(&m_resources);
	printf("datapath is :%s\n", datapath);
	sleep(5);
	
	/*do dummy things*/
	for(i = 0; i < 10; i++) {
		dummy();
		sleep(2);
	}

	mf_end();
}

/*******************************************************************************
 * resources and disk monitor test 
 ******************************************************************************/
void Test_resources_and_disk(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 2;
	m_resources.sampling_interval[0] = SAMPLE_INTERVAL; // 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");

	m_resources.sampling_interval[1] = SAMPLE_INTERVAL; // 1s
	strcpy(m_resources.metrics_names[1], "disk_io");

	char *datapath = mf_start(&m_resources);
	printf("datapath is :%s\n", datapath);
	
	/*do dummy things*/
	for(i = 0; i < 10; i++) {
		dummy();
		sleep(1);
	}

	mf_end();

	/* when "dummy" application already exists in database /mf/workflows, 
	it is possible to send collected metrics to the server */
	char server[] = "localhost:3030";
	char application_name[] = "dummy";
	char task_name[] = "t1";
	char platform_name[] = "ubuntu";
	char *execution_id = mf_send(server, application_name, task_name, platform_name);
	printf("\nexecution_id is %s\n", execution_id);
	free(execution_id);
}


int main(void)
{
	/* test basic functions */
	//Test_resources_stat_cpu();
	//Test_resources_stat_all_and_calculate();
	//Test_disk_stats_read();

	/* test mf interfaces: mf_start, mf_end */
	//Test_resources_monitor(); 	//only resources monitoring 
	//Test_disk_monitor();		//only disk monitoring

	/* test mf interfaces: mf_start, mf_end, mf_send */	
	Test_resources_and_disk();	//both resources and disk monitoring

	return 0;
}