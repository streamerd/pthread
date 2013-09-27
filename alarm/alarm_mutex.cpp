#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <cstring>
#include <ctime>
#include <unistd.h>

#define MAX_MSG_LEN 64

typedef struct alarm_node_t
{
	alarm_node_t *next;
	time_t sleep_to;
	int sec;
	char msg[MAX_MSG_LEN];
}alarm_t;

static pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_t *alarm_list = NULL;

void *read_thread(void *p)
{
	while(1)
	{
		int sleep_time = 0;
		int lock_ret = pthread_mutex_lock(&alarm_mutex);
		if(lock_ret)
		{
			fprintf(stderr, "failed to lock the mutex\n");
			pthread_exit(0);
			continue;
		}
		alarm_t *alarm_node = alarm_list;
		if(alarm_node == NULL)
		{
			sleep_time = 1;
		}
		else
		{
			alarm_list = alarm_node->next;
			time_t time_now = time(NULL);
			sleep_time = alarm_node->sleep_to - time_now;
		}
		lock_ret = pthread_mutex_unlock(&alarm_mutex);
		if(lock_ret)
		{
			fprintf(stderr, "failed to unlock the mutex\n");
			pthread_exit(0);
		}
		fprintf(stdout, "%d\n", sleep_time);
		if(sleep_time > 0) 
		{
			sleep(sleep_time);
		}
		else sched_yield();

		if(alarm_node)
		{
			fprintf(stdout, "(%d) %s", alarm_node->sec, alarm_node->msg);
			alarm_node->next = NULL;
			free(alarm_node);
			alarm_node = NULL;
		}

	}
}

int main()
{
	int sec = 0;
	char message[MAX_MSG_LEN];
	char line[1024];
	time_t start = time(NULL);
	pthread_t tmp;
	int status = pthread_create(&tmp, NULL, read_thread, NULL);
	if(status)
	{
		fprintf(stderr, "failed to pthread_create\n");
		return 0;
	}
	while(1)
	{
		if(fgets(line, sizeof(line), stdin) == NULL) exit(0);
		if(strlen(line) <= 1) continue;
		if(sscanf(line, "%d %64[^\n]", &sec, message) != 2)
		{
			fprintf(stderr, "bad common\n");
		}
		else
		{
			alarm_t *tmp = (alarm_t *)malloc(sizeof(alarm_t));
			strncpy(tmp->msg, message, MAX_MSG_LEN);
			time_t now = time(NULL);
			tmp->sec = sec;
			tmp->sleep_to = now + sec;
			
			int lock_ret = pthread_mutex_lock(&alarm_mutex);
			alarm_t *alarm_node = alarm_list;
			alarm_t *alarm_prev_node = NULL;
			while(alarm_node)
			{
				if(alarm_node->sleep_to < tmp->sleep_to) 
				{
					alarm_prev_node = alarm_node;
					alarm_node = alarm_node->next;
					continue;
				}
				else break;	
			}
			if(alarm_prev_node == NULL)
			{
				alarm_list = tmp;
				tmp->next = NULL;
			}
			else
			{
				tmp->next = alarm_prev_node->next;
				alarm_prev_node->next = tmp;
			}
			lock_ret = pthread_mutex_unlock(&alarm_mutex);
			if(lock_ret)
			{
				fprintf(stderr, "failed to unlock the mutex\n");
				pthread_exit(0);
			}
		}
	}
	pthread_exit(0);
	return 0;
}
