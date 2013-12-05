#include <cstdio>
#include <ctime>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <list>

class alarm_manager
{
public:
	pthread_mutex_t list_mutex;
	pthread_cond_t alarm_cond;
	int current_wait_time;
	int expired;
	
	alarm_manager()
	{
		list_mutex = PTHREAD_MUTEX_INITIALIZER;
		alarm_cond = PTHREAD_COND_INITIALIZER;
		current_wait_time = expired = 0;
	}
	
	~alarm_manager() {}
	
	inline int lock()
	{
		return pthread_mutex_lock(&list_mutex);
	}
	inline int unlock()
	{
		return pthread_mutex_unlock(&list_mutex);
	}
	inline int cond_wait()
	{
		return pthread_cond_wait(&alarm_cond, &list_mutex);
	}
	inline int cond_time_wait(timespec next_time)
	{
		return pthread_cond_timedwait(&alarm_cond, &list_mutex, &next_time);
	}
	inline int signal()
	{
		return pthread_cond_signal(&alarm_cond);
	}
};

alarm_manager mgr;
std::list<int> alarm_list;

char * get_time_now(int time_now)
{
	time_t tmp = time_now;
	return ctime(&tmp);
}

void * alarm_thread(void *arg)
{
	while(1)
	{
		mgr.lock();
		while(mgr.current_wait_time == 0)
		{	
			//100s time out for no input
			timespec tmp = {100 + time(NULL), 0};
			int status = mgr.cond_time_wait(tmp);
			if(status == ETIMEDOUT)
			{
				mgr.unlock();
				pthread_exit(NULL);
			}
		}
		//must not be empty
		//if(!alarm_list.empty()) 
		{
			mgr.current_wait_time = alarm_list.front();
			alarm_list.erase(alarm_list.begin());
		}
		timespec tmp = {mgr.current_wait_time, 0};
		while(mgr.current_wait_time == tmp.tv_sec)
		{
			int status = mgr.cond_time_wait(tmp);
			if(status == ETIMEDOUT)
			{
				mgr.expired = 1;
				break;
			}
		}
		if(mgr.expired == 0)
		{
			//change the alarm you are waiting, and insert the previous one
			std::list<int> tmp_list(1, tmp.tv_sec);
			alarm_list.merge(tmp_list);
		}
		else 
		{
			//the time is expired, output the alarm time
			mgr.expired = 0;
			printf("alarm at %s", get_time_now(mgr.current_wait_time));
		}
		//set the current wait time to ZERO in case of list-empty
		mgr.current_wait_time = 0;
		//if the list isn't empty, set next alarm
		if(!alarm_list.empty()) mgr.current_wait_time = alarm_list.front();
		mgr.unlock();
	}
	pthread_exit(NULL);
}

int main()
{
	pthread_t alarmer;
	pthread_create(&alarmer, NULL, alarm_thread, NULL);
	int alarm_time;
	while(scanf("%d", &alarm_time) == 1)
	{
		int time_now = time(NULL);
		mgr.lock();
		std::list<int>::iterator iter;
		for(iter = alarm_list.begin(); iter != alarm_list.end(); ++iter)
		{
			
			if(*iter <= alarm_time + time_now) continue;
			break;
		}
		if(iter == alarm_list.begin())
		{
			mgr.current_wait_time = alarm_time + time_now;
			mgr.signal();
		}
		alarm_list.insert(iter, alarm_time + time_now);
		mgr.unlock();
	}
	pthread_join(alarmer, NULL);
	return 0;
}
