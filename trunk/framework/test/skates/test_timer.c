#include <stdio.h>
#include <stdlib.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/mempool.h>
#include <skates/threadpool.h>
#include <skates/timer.h>

void timer_event(TIMER handle, void* key)
{
	printf("timer %p fired key = %p \n", handle, key);
}

int main(int argc, char* argv[])
{
	int i = 0;
	TIMER handles[4096];
	
	mempool_init();	
	threadpool_init(10);	
	timer_init(100);
	//
	//while(i != 1000)
	//{
	//	timer_add(rand()%10000, 0, TIMER_ONESHOOT, timer_event, (void*)i++);  	  	
	//}
	//
	//getchar();
	
	i = 0;
	
	while(i != 100)
	{
		handles[i] = timer_add(1000, 1000, timer_event, &handles[i]);
		i++;
	}
	
	getchar();
	
	while(--i >= 0)
	{	  		
		timer_force_remove(handles[i]);
	}
	
	printf("all removed \n");
	
	getchar(); 
	
	timer_final();
	threadpool_final();
	mempool_final();
	
	return(0);
}
