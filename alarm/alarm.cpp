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
#include <unistd.h>
#include <cstdlib>

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
		}
		else 
		{
			sleep(sec);
			fprintf(stdout, "(%d) %s\n", sec, message);
		}
	}
	return 0;
}
