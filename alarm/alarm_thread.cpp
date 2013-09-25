/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file alarm_thread.cpp
 * @author yangyihao(com@baidu.com)
 * @date 2013/09/23 14:54:06
 * @brief 
 *  
 **/



#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <pthread.h>

const int MAX_MSG_LEN = 64;

struct alarm_t
{
	int sec;
	char msg[MAX_MSG_LEN];
};

void * alarm_thread(void *p)
{
	alarm_t *message = (alarm_t *)p;
	printf("%d\n", message->sec);
	sleep(message->sec);
	printf("(%d)%s\n", message->sec, message->msg);
	sleep(100);
	fflush(0);
	free(p);
	//pthread_detach(pthread_self());
	//pthread_exit(0);
}

int main()
{
	int sec = 0;
	char message[128] = "hello world\n";
	char line[1024];
	pthread_t tmp;
	while(1)
	{
		if(fgets(line, sizeof(line), stdin) == NULL) exit(0);
		if(strlen(line) <= 1) continue;
		if(sscanf(line, "%d %128[^\n]", &sec, message) != 2)
		{
			fprintf(stderr, "bad common\n");
			continue;
		}
		alarm_t *at = (alarm_t *)malloc(sizeof(alarm_t));
		at->sec = sec;
		strncpy(at->msg, message, MAX_MSG_LEN);
		int ret = pthread_create(&tmp, NULL, alarm_thread, at);
		//sleep(0);
		if(ret)
		{
			fprintf(stderr, "create thread error\n");
			continue;
		}
	}
	return 0;
}


















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
