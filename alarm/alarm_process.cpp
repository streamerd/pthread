/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file alarm_process.cpp
 * @author yangyihao(com@baidu.com)
 * @date 2013/09/23 14:53:56
 * @brief 
 *  
 **/


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main()
{
	int sec = 0;
	char message[128];
	char line[1024];
	while(1)
	{
		if(fgets(line, sizeof(line), stdin) == NULL) exit(0);
		if(strlen(line) <= 1) continue;
		if(sscanf(line, "%d %128[^\n]", &sec, message) != 2)
		{
			fprintf(stderr, "bad common\n");
			continue;
		}
		int pid = fork();
		if(pid < 0)
		{
			fprintf(stderr, "fork error");
			continue;
		}
		if(pid == 0)  
		{
			sleep(sec);
			fprintf(stdout, "(%d) %s\n", sec, message);
			exit(0);
		}
		do
		{
			pid = waitpid(-1, NULL, WNOHANG);
			if(-1 == pid)
			{
				fprintf(stderr, "error_no:%d", errno);
				exit(-1);
			}
		}
		while(pid);
	}
	return 0;
}



















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
