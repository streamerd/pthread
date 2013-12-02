#include <pthread.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

std::vector<pthread_mutex_t> mutex_gather(3, (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER);

//default: use backoff; others: don't use
int backoff = 1;
//default: don't interferring any thread; >0: use sched_yield(); <0: sleep(1)
int yield_flag = 0; 
//iterate ITERATOR times
const int ITERATOR = 10;

char *get_time_now()
{
	time_t now;
	time(&now);
	tm *time_now;
	time_now = localtime(&now);
	char *tmp = asctime(time_now);
	tmp[strlen(tmp) - 2] = 0;
	return tmp;
}

void backoff_algorithm(int direction)
{	
	int fd;
	if(direction == 1)
		fd = open("forward_thread.log", O_WRONLY | O_CREAT);
	else 
		fd = open("backward_thread.log", O_WRONLY | O_CREAT), direction = 2;
	FILE *file_pointer[3] = {stdin, stdout, stderr};
	FILE *fp = file_pointer[direction];
	if(dup2(fd, direction) != direction)
	{
		
		fprintf(fp, "%s: dup2 %d error \n", get_time_now(), direction);
		pthread_exit(NULL);
	}
	if(direction == 1)	
	{
		for(int tm = 0; tm < ITERATOR; ++tm)
		{
			int backoffs = 0;
			fprintf(fp, "%s:----------------- round %d start ---------------\n", get_time_now(), tm);
		
			for(int forward_iter = 0; forward_iter < mutex_gather.size(); ++forward_iter)
			{
				int status;
				if(forward_iter == 0)
				{
					status = pthread_mutex_lock(&(mutex_gather[forward_iter]));
					if(status)
					{
						fprintf(fp, "%s, error occurs when lock %d mutex\n", 
								get_time_now(), forward_iter + 1);
						pthread_exit(NULL);
					}
					continue;
				}
				if(backoff != 1)
					status = pthread_mutex_lock(&(mutex_gather[forward_iter]));
				else status = pthread_mutex_trylock(&(mutex_gather[forward_iter]));
				if(status == EBUSY)
				{
					++backoffs;
					fprintf(fp, "%s: backoff occurs\n", get_time_now());
					for(; forward_iter >= 0; --forward_iter)
					{
						pthread_mutex_unlock(&mutex_gather[forward_iter]);
					}	
				}
				else
				{
					if(status)
					{
						fprintf(fp, "%s, error occurs when lock %d mutex\n", 
								get_time_now(), forward_iter + 1);
						pthread_exit(NULL);
					}
					
				}
				if(yield_flag > 0) sched_yield();
				else if(yield_flag < 0) sleep(1);
			}
			pthread_mutex_unlock(&mutex_gather[0]);	
			pthread_mutex_unlock(&mutex_gather[1]);	
			pthread_mutex_unlock(&mutex_gather[2]);	
			fprintf(fp, "%s: release all locks, %d backoffs occured\n", get_time_now(), backoffs);
			fprintf(fp, "%s:----------------- round %d end -----------------\n", get_time_now(), tm);
			sched_yield();
		}
	}
	else 
	{
		for(int tm = 0; tm < ITERATOR; ++tm)
		{
			int backoffs = 0;
			fprintf(fp, "%s:----------------- round %d start ---------------\n", get_time_now(), tm);
		
		
			for(int backward_iter = mutex_gather.size() - 1; backward_iter >= 0; --backward_iter)
			{
				int status;
				if(backward_iter == mutex_gather.size() - 1)
				{
					status = pthread_mutex_lock(&(mutex_gather[backward_iter]));
					if(status)
					{
						fprintf(fp, "%s, error occurs when lock %d mutex\n", 
								get_time_now(), backward_iter + 1);
						pthread_exit(NULL);
					}
					continue;
				}
				if(backoff != 1)
					status = pthread_mutex_lock(&(mutex_gather[backward_iter]));
				else status = pthread_mutex_trylock(&(mutex_gather[backward_iter]));
				if(status == EBUSY)
				{
					++backoffs;
					fprintf(fp, "%s: backoff occurs\n", get_time_now());
					for(; backward_iter < mutex_gather.size(); ++backward_iter)
					{
						pthread_mutex_unlock(&mutex_gather[backward_iter]);
					}	
				}
				else
				{
					if(status)
					{
						fprintf(fp, "%s, error occurs when lock %d mutex\n", 
								get_time_now(), backward_iter + 1);
						pthread_exit(NULL);
					}
					
				}
				if(yield_flag > 0) sched_yield();
				else if(yield_flag < 0) sleep(1);
			}
			pthread_mutex_unlock(&mutex_gather[0]);	
			pthread_mutex_unlock(&mutex_gather[1]);	
			pthread_mutex_unlock(&mutex_gather[2]);	
			fprintf(fp, "%s: release all locks, %d backoffs occured\n", get_time_now(), backoffs);
			fprintf(fp, "%s:----------------- round %d end -----------------\n", get_time_now(), tm);
			sched_yield();
		}
	}
	
	close(fd);
	return;
}

void * forward_lock(void *arg)
{
	
	backoff_algorithm(2);	
	return NULL;
}

int main(int argc, char ** argv)
{
	int opt;
	while((opt = getopt(argc, argv, "b:y:")) != -1)
	{
		switch(opt)
		{
		case 'b':
			sscanf(optarg, "%d", &backoff);
			break;
		case 'y':
			sscanf(optarg, "%d", &yield_flag);
			break;		
		}
	}
	printf("%d %d\n", backoff, yield_flag);
	pthread_t forward_locker;
	pthread_create(&forward_locker, NULL, forward_lock, NULL);
	
	backoff_algorithm(1);
	pthread_join(forward_locker, NULL);	
	return 0;
}
