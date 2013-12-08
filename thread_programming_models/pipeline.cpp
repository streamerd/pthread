//A demonstration on thread programming using pipeline model

#include <cstdio>
#include <list>
#include <pthread.h>

class processor;
class pipeline
{
private:
	pthread_mutex_t *mutex;
	int active;
public:
	std::vector<processor *> line;
	
	pipeline(int n): line(n + 1, NULL), mutex(NULL)
	{
	}

	~pipeline()
	{
		if(mutex) pthread_mutex_destory(mutex);
		for(int i = 0; i < line.size() - 1; ++i)
		{
			if(line[i]) delete line[i];
		}
	}	
	
	int lock()
	{
		return pthread_mutex_lock(mutex);
	}
	
	int unlock()
	{
		return pthread_mutex_unlock(mutex);
	}
	
	void pipe_start(int send_num);
	int pipe_result(int &result);
	int init();	
}

//processor in the pipeline
//need to be registed in a pipeline or it will be invalid
class processor
{
private:
	pthread_t thread;
	// a mutex connect a data unit and two conditional varibles
	pthread_mutex_t *mutex;
	//ready condition and its predicate
	pthread_cond_t *ready;
	//done condition and its predicate, it means the processor can get new data now
	//but I think In this scene, it doesn't make any sense
	pthread_cond_t *done;
	//the identifier of a processor in the very pipeline
	int id;
	pipeline *pipe;

public:
	//data
	int data;
	bool is_ready;
	processor(): is_ready(0), id(0), pipe(NULL),
	             mutex(NULL), ready(NULL), done(NULL),
		     thread((pthread_t)PTHREAD_MUTEX_INITIALIZER)  
	{
	}
	~processor()
	{
		if(mutex) pthread_mutex_destroy(mutex);
		if(ready) pthread_cond_destroy(ready);
		if(done) pthread_cond_destroy(done);
	}
	int init(int identifier, pipeline *boss)
	{
		id = identifier;
		pipe = boss;
		int statu = pthread_mutex_init(mutex);
		if(statu) return -1;
		statu = pthread_cond_init(ready);
		if(statu) return -1;
		statu = pthread_cond_init(done);
		return statu ? -1 : 0;
	}

	int signal_ready()
	{
		return pthread_cond_signal(ready);	
	}
	
	int lock()
	{
		return pthread_mutex_lock(mutex);
	}

	int unlock()
	{
		return pthread_mutex_unlock(mutex);
	}

	//pipeline job you can reload it by different jobs
	virtual void job(int new_data)
	{
		data = new_data + 1;
		is_ready = 1;
	}

	int wait_data_ready()
	{
		while(is_ready != 1)
		{
			pthread_cond_wait(ready, mutex);
		}
	}
	
	static void * thread_instance(void *arg)
	{
		processor *now = (processor *)arg;
		//don't have to lock the pipe for no one will write it
		processor *next = now->pipe->line[now->id + 1];
		
		now->lock();
		while(1)
		{
			now->wait_data_ready();
			if(next) 
			{
				next->lock();
				next->job(now->data);
				next->signal_ready();
				next->unlock();
				is_ready = 0;
			}
			else
			{
				while(is_ready)
				{
					pthread_cond_wait(avail, mutex);
				}
			}
		}
		return NULL;			
	}
}

void pipeline::pipe_start(int send_num)
{
	lock();
	line[0]->lock();
	line[0]->job(send_num);
	line[0]->unlock();
	++active;
	unlock();
}	

int pipeline::pipe_result(int &result)
{
	lock();
	if(active <= 0) return -1;
	processor *tail = line[line.size() - 2];
	tail->lock();
	tail->wait_data_ready();
	result = tail.data;
	tail->is_ready = 0;
	tail->unlock();	
	--active;
	unlock();
	return 0;
}

int pipeline::init()
{
	for(int i = 0; i < line.size() - 1; ++i)
	{
		line[i] = new processor();
		if(line[i]->init(i, this)) return -1;
	}
	for(int i = 0; i < line.size() - 1; ++i)
	{
		pthread_create(line[i]->thread, NULL, processor::thread_instance, (void *)line[i]);
	}
	return 0;
}


int main(int argc, char **argv)
{
	pipeline task(10);
	task.init();
	char data[64];
	while(gets(data) != NULL)
	{
		int tmp;
		if(data[0] == '=')
		{
			if(task.pipe_result(tmp) == 0)
				printf("the number now is %d\n", tmp);
			else 
				printf("the pipeline has no result now\n");
			continue;
		}
		sscanf(data, "%d", &tmp);
		task.pipe_start(tmp);		
	}
	return 0;
}
