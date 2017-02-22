#ifndef _MF_API_H
#define _MF_API_H

//#define SERVER               "localhost:3030"
#define MAX_NUM_METRICS      3
#define NAME_LENGTH          32

typedef struct metrics_t {
	long sampling_interval[MAX_NUM_METRICS];	//in milliseconds
	char metrics_names[MAX_NUM_METRICS][NAME_LENGTH];		//user defined metrics
	int num_metrics;
} metrics;

extern int running;
/* 
Get the pid, and setup the DataPath for data storage 
For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
Return the path of data files
*/
char *mf_start(metrics *m);

/*
Stop threads.
Close all the files for data storage
*/
void mf_end(void);

/*
Generate the execution_id.
Send the monitoring data in all the files to mf_server.
Return the execution_id
*/
char *mf_send(char *server, char *application_id, char *component_id, char *platform_id);

#endif /* _MF_API_H */