#include "logger.h"
#include "procfs.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdbool.h>

/**
 * Get the hostname from proc
 *
 * @param proc_dir directory of proc to use
 * @param hostname_buf hostname buffer passed in
 * @param buf_sz buffer size
 *
 * @return Success / Error code
 */ 
int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz)
{
    int fd = open_path(proc_dir, "sys/kernel/hostname");
    if(fd <= 0) {
	    perror("open_path");
	    return -1;
    }
    one_lineread(fd, hostname_buf, buf_sz);
    close(fd);
    return 0;
}

/**
 * Get the kernel version
 *
 * @param proc_dir directory of proc to use
 * @param version_buf version buffer
 * @param buf_sz buffer size
 * 
 * @return Success / Error code 
 */ 
int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz)
{
   	int fd = open_path(proc_dir, "/sys/kernel/osrelease");
	if(fd <= 0){
		perror("open_path");
		return -1;
	}

	one_lineread(fd, version_buf, buf_sz);
	size_t dash_pos = strcspn(version_buf, "-");
	version_buf[dash_pos] = '\0';
	close(fd);
	return 0;
}

/**
 * Get the CPU model
 *
 * @param proc_dir proccess directory to use
 * @param model_buf model buffer to use
 * @param buf_sz buffer size
 *
 * @return success / error code
 */  
int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz)
{
  int fd = open_path(proc_dir, "cpuinfo");
  if(fd <= 0){
	 perror("open_path");
	return -1;
  }
  char line[buf_sz + 3];
  one_lineread(fd, line, buf_sz);
  while(strstr(line, "model name") == NULL){
	  one_lineread(fd, line, buf_sz);
  }
  char *next_tok = line;
  next_token(&next_tok, "\t");
  next_token(&next_tok, " ");
  //LOG("Line read: %s\n", next_tok); 
  strcpy(model_buf, next_tok);
  close(fd);
  //LOGP("Read cpuinfo, get the model name.\n");
   return 0;
}

/**
 * Get the CPU units, reads from /cpuinfo
 *
 * @param proc_dir processs directory to read from
 *
 * @return success / failure code
 */ 
int pfs_cpu_units(char *proc_dir)
{
    int fd = open_path(proc_dir, "stat");
    if(fd <=0){
	    perror("open_path");
	    return -1;
    }
    char line[1024];
    one_lineread(fd, line, 100);
    int count  = 0;
    while(strstr(line, "cpu") != NULL){
	    one_lineread(fd, line, 100);
	    count++;
    }
    count--;
    char *next_tok = line;
    next_token(&next_tok, "\t");
    next_token(&next_tok, " ");
    int cores = count > -1 ? count : 0;
    //LOG("Cores: %d\n", cores);
    close(fd);
    return cores;
}

/**
 * Get the device uptime in ms
 *
 * @param proc_dir process directory to read from
 *
 * @return uptime in ms
 */ 
double pfs_uptime(char *proc_dir)
{
    int fd = open_path(proc_dir, "uptime");
    if(fd <= 0){
	    perror("open_path");
	    return 0.0;
    }
    char line[200];
    one_lineread(fd, line, 100);
    char* next_tok = line;
    char* time = next_token(&next_tok, " ");
    double uptime = atof(time);
    if(isnan(uptime)){
	    return 0.0;
    }
    close(fd);
    return uptime;
}

/**
 * Format the device uptime
 *
 * @param time uptime in ms
 * @param uptime_buf uptime buffer to write into
 *
 * @return success / error code
 */ 
int pfs_format_uptime(double time, char *uptime_buf)
{
    int seconds, minutes, hours, days, years;
    years = time / (24 * 3600 * 365);
    days = time / (24 * 3600);
    time = (int)time % (24 * 3600);
    hours = time / 3600;
    time = (int)time % 3600;
    minutes = time / 60;
    time = (int)time % 60;
    seconds = time;

    if(years != 0){
   	 sprintf(uptime_buf, "%d years, %d days, %d hours, %d minutes, %d seconds", years, days, hours, minutes, seconds);
    }
    if(years == 0){
   	 sprintf(uptime_buf, "%d days, %d hours, %d minutes, %d seconds", days, hours, minutes, seconds);
    }


    if(days == 0){
    	sprintf(uptime_buf, "%d hours, %d minutes, %d seconds",  hours, minutes, seconds);
    }

    if(hours == 0){
    	sprintf(uptime_buf, "%d minutes, %d seconds",  minutes, seconds);
    }

    if(minutes == 0){
    	sprintf(uptime_buf, "%d seconds", seconds);
    }

    return 0.0;
}

/**
 * Get the average loadtimes from loadavg
 *
 * @param proc_dir process directory to access
 *
 * @return load_avg structure with one, five, and fifteen load averages included
 */ 
struct load_avg pfs_load_avg(char *proc_dir)
{
   struct load_avg lavg = { 0 };
   int fd = open_path(proc_dir, "loadavg");
   if(fd <= 0){
	   return lavg;
   };
   char line[100];
   one_lineread(fd, line, 50);
   char *next_tok = line;
   char *curr_tok;
   double one, five, fifteen;
   curr_tok = next_token(&next_tok, " ");
   one = atof(curr_tok);
   curr_tok = next_token(&next_tok, " ");
   five = atof(curr_tok);
   curr_tok = next_token(&next_tok, " ");
   fifteen = atof(curr_tok);

   lavg.one = one;
   lavg.five = five;
   lavg.fifteen = fifteen;
   close(fd);
   return lavg;
}

/**
 * Get the CPU usage of the computer
 *
 * @param proc_dir process directory to use
 * @param prev previous cpu stats
 * @param curr current cpu stats
 *
 * @return percentage average between previous and current usage stats
 */ 
double pfs_cpu_usage(char *proc_dir, struct cpu_stats *prev, struct cpu_stats *curr)
{
    int fd = open_path(proc_dir, "stat");
    if(fd <= 0){
	    return 0.0;
    }
    char line[200];
    one_lineread(fd, line, 100);
    char *next_tok = line;
    next_token(&next_tok, " ");
    //LOG("Token: %s\n", next_tok);
    char *user, *nice, *system, *idle, *iowait, *irq, *softirq, *steal, *guest, *guest_nice;
    user = next_token(&next_tok, " ");
    nice = next_token(&next_tok, " " );
    system = next_token(&next_tok, " " );
    idle = next_token(&next_tok, " ");
    iowait = next_token(&next_tok, " ");
    irq = next_token(&next_tok, " ");
    softirq = next_token(&next_tok, " ");
    steal = next_token(&next_tok, " ");
    guest = next_token(&next_tok, " ");
    guest_nice = next_token(&next_tok, " ");
    long total = atol(user) + atol(nice) + atol(system) + atol(idle) + atol(iowait) + atol(irq) + atol(steal) +  atol(softirq) + atol(guest) + atol(guest_nice);
    if(isnan((float)total)){
	    return 0.0;
    }
    long idle_time = atol(idle);
    curr->total = total;
    curr->idle = idle_time;
    //LOG("Total Time: %ld, Idle Time: %ld\n",total, idle_time);
    //LOG("Previous Total Time: %ld, Previous Idle Time: %ld\n", prev -> total, prev -> idle); 
    if(idle_time - prev -> idle < 0 || total - prev -> total < 0){
	    return 0.0;
    }
    double val = 1 - (double)(idle_time - prev -> idle) / (double)(total - prev -> total);
   // LOG("Value: %f\n", val);
    if(isnan(val)){
	    return 0.0;
    }
    close(fd);
    return val;
}

/**
 * Memory usage of computer
 *
 * @param proc_dir process directory to access
 *
 * @return memory statistics struct
 */ 
struct mem_stats pfs_mem_usage(char *proc_dir)
{
    struct mem_stats mstats = { 0 };
    int fd = open_path(proc_dir, "meminfo");
  
    if(fd <= 0){
	    close(fd);
	    return mstats;
    }
    char line[200];
    int count = 0;
    double total = 0.0, used, available = 0.0;
    while(lineread(fd, line, 200) > 0){
	    char *next_tok = line;
	    char *curr;
	    if(count == 2){
		    break;
	    }
	    if((curr  = next_token(&next_tok, ":\t")) != NULL){
	    	if(strcmp(curr, "MemTotal") == 0){
			count++;
			curr = next_token(&next_tok, ":\t");
			total = atof(curr); 
		}
		if(strcmp(curr, "MemAvailable") == 0){
			count++;
			curr = next_token(&next_tok, ":\t");
			available = atof(curr);
		}
	    }
	}
				
    used = total - available;
    LOG("Total: %f, Available: %f, Used: %f\n", total, available, used);
    mstats.total = kb_to_mb(total);
    mstats.used = kb_to_mb(used); 
    close(fd);
    return mstats;
}

/**
 * Create the task statistics
 *
 * @return task_stats structure with task structs inside
 */ 
struct task_stats *pfs_create_tstats()
{
    struct task_stats* stats = malloc(sizeof(struct task_stats));
    stats -> active_tasks = malloc(sizeof(struct task_info) * 200000);
    return stats;
}

/**
 * Destroy the task stats to reset
 *
 * @param tstats task_stats task stats to destroy
 *
 */
void pfs_destroy_tstats(struct task_stats *tstats)
{
	free(tstats -> active_tasks);
	free(tstats);
}

/**
 * Get the tasks
 *
 * @param proc_dir process directory to read tasks from
 * @param tstats task stats to write into
 *
 * @return success / eror codes
 * 
 */ 
int pfs_tasks(char *proc_dir, struct task_stats *tstats)
{
    unsigned int total = 0, running = 0, waiting = 0, sleeping = 0, stopped = 0, zombie = 0;
    DIR *directory;
    if((directory = opendir(proc_dir)) == NULL){
	    perror("opendir");
	    return 1;
    }
    int index = 0;
   
    struct dirent *entry;
    while((entry = readdir(directory)) != NULL){
	    if(is_only_numeric(entry -> d_name)){
		    bool is_sleeping = false;
    		    int count = 0, pid = 0, uid = 0;
		    char name[26], state[13];
		    char path[strlen(entry -> d_name) + strlen("status") + 2];
		    strcpy(path, entry -> d_name);
		    strcat(path, "/");
		    strcat(path, "status");
		    int fd = open_path(proc_dir, path);
		    if(fd <= 0){
			    continue;
		    } else {
			    total++;
		    }
		    char line[300];
		    while(lineread(fd, line, 200) > 0){
			char *next_tok = line;
			char *curr;
			if((curr = next_token(&next_tok, ":\t")) != NULL){
				if(strcmp(curr, "State") == 0){
					count++;
					curr = next_token(&next_tok, ":\t");
					if(curr[0] == 'S'){
						is_sleeping = true;
						sleeping++;
					} else if(curr[0] == 'R'){
						strcpy(state, "running");
						running++;
					} else if(curr[0] == 'I'){
						is_sleeping = true;
						sleeping++;
					} else if(curr[0] == 'Z'){
						strcpy(state, "zombie");
						zombie++;
					} else if(curr[0] == 'D'){
						strcpy(state, "disk sleep");
						waiting++;
					} else if(curr[0] == 'T'){
						char *t_state;

						char *curr_tok = curr;
						next_token(&curr_tok, " ");
						t_state =  next_token(&curr_tok, "( ");
						 if(strcmp(t_state, "tracing") == 0){
							strcpy(state, "tracing stop");
						 } else {
						strcpy(state, "stopped");
						}
						stopped++;
					} else if(curr[0]  == 't'){
						strcpy(state, "tracing stop");
						stopped++;
					} else if(curr[0] == 'X'){
						is_sleeping = true;
						sleeping++;
					}
					if(curr[0] != 'S'){
						pid = atoi(entry -> d_name);
					}
				}
				if(strcmp(curr, "Uid") == 0){
					curr = next_token(&next_tok, ":\t");
					if(is_sleeping == false){
						uid = atoi(curr);
					}
					count++;
				}
				if(strcmp(curr, "Name") == 0){
					curr = next_token(&next_tok, "\t\n");
					if(is_sleeping == false){
						strcpy(name, curr);
						name[25] = '\0';
					}
					count++;
				}	
				if(count == 3){
					if(is_sleeping == false){
						strcpy(tstats -> active_tasks[index].name, name);
						tstats -> active_tasks[index].pid = pid;
						tstats -> active_tasks[index].uid = uid;
						strcpy(tstats -> active_tasks[index].state, state);
						index++;
					}
					break;
				}
			}
		  }
		   close(fd); 
	    }
    }
    tstats -> total = total;
    tstats -> waiting = waiting;
    tstats -> running = running;
    tstats -> sleeping = sleeping;
    tstats -> stopped = stopped;
    tstats -> zombie = zombie;
    closedir(directory);
    return 0;
}

