//A demonstration on thread programming using pipeline model

#include <cstdio>
#include <list>
#include <vector>
#include <pthread.h>

class processor;
class pipeline
{
private:
	pthread_mutex_t mutex;
	int active;
public:
	std::vector<processor *> line;
	
	pipeline(int n): line(n + 2, (processor *)NULL),  
			 active(0)
	{
	}

	~pipeline();
	
	int lock()
	{
		return pthread_mutex_lock(&mutex);
	}
	
	int unlock()
	{
		return pthread_mutex_unlock(&mutex);
	}
	
	void pipe_start(int send_num);
	int pipe_result(int &result);
	int init();	
};

//processor in the pipeline
//need to be registed in a pipeline or it will be invalid
class processor
{
private:
	pthread_t thread;
	// a mutex connect a data unit and two conditional varibles
	pthread_mutex_t mutex;
	//ready condition and its predicate
	pthread_cond_t ready;
	//avail condition and its predicate, it means the processor can get new data now
	//its purpose is to support waiting on the result-getting function
	pthread_cond_t avail;
	//the identifier of a processor in the very pipeline
	int id;
	pipeline *pipe;

public:
	//data
	int data;
	bool is_ready;
	processor(): is_ready(0), id(0), pipe(NULL)
	{
	}
	~processor()
	{
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&ready);
		pthread_cond_destroy(&avail);
	}
	
	pthread_t & get_thread_instance()
	{
		return  thread;	
	}
	int init(int identifier, pipeline *boss)
	{
		id = identifier;
		pipe = boss;
		int statu = pthread_mutex_init(&mutex, NULL);
		if(statu) return -1;
		statu = pthread_cond_init(&ready, NULL);
		if(statu) return -1;
		statu = pthread_cond_init(&avail, NULL);
		return statu ? -1 : 0;
	}

	int signal_ready()
	{
		return pthread_cond_signal(&ready);	
	}
	
	int signal_avail()
	{
		return pthread_cond_signal(&avail);
	}
	
	int lock()
	{
		return pthread_mutex_lock(&mutex);
	}

	int unlock()
	{
		return pthread_mutex_unlock(&mutex);
	}

	//pipeline job you can reload it by different jobs
	virtual void job(int new_data)
	{
		data = new_data + 1;
		is_ready = 1;
	}

	int wait_data_ready()
	{
		while(!is_ready)
		{
			pthread_cond_wait(&ready, &mutex);
		}
	}
	
	int wait_data_avail()
	{
		while(is_ready)
		{
			pthread_cond_wait(&avail, &mutex);
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
			next->lock();	
			next->wait_data_avail();
			next->job(now->data);
			next->signal_ready();
			next->unlock();
			now->is_ready = 0;
			now->signal_avail();
			
		}
		return NULL;			
	}
};

pipeline::~pipeline()
{
	pthread_mutex_destroy(&mutex);
	for(int i = 0; i < line.size(); ++i)
	{
		if(line[i]) delete line[i];
	}
}	
void pipeline::pipe_start(int send_num)
{
	lock();
	line[0]->lock();
	line[0]->wait_data_avail();
	line[0]->job(send_num);
	line[0]->signal_ready();
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
	result = tail->data;
	tail->is_ready = 0;
	tail->signal_avail();
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
	for(int i = 0; i < line.size() - 2; ++i)
	{
		pthread_create(&line[i]->get_thread_instance(), NULL, processor::thread_instance, (void *)line[i]);
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
