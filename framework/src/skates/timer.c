#include <assert.h>
#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/threadpool.h"
#include "../../inc/skates/timer.h"
#include "../../inc/skates/mempool.h"

#ifndef __GETTIMEOFDAY_C
#define __GETTIMEOFDAY_C

#if defined(_MSC_VER) || defined(_WINDOWS_)
   #include <time.h>
   #if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
         struct timeval 
         {
            long tv_sec;
            long tv_usec;
         };
   #endif 
#else
   #include <sys/time.h>
#endif 

#if defined(_MSC_VER) || defined(_WINDOWS_)

#define       timerisset(tvp)\
               ((tvp)->tv_sec || (tvp)->tv_usec)
#define       timercmp(tvp, uvp, cmp)\
               ((tvp)->tv_sec cmp (uvp)->tv_sec ||\
               (tvp)->tv_sec == (uvp)->tv_sec &&\
               (tvp)->tv_usec cmp (uvp)->tv_usec)
#define       timerclear(tvp)\
               ((tvp)->tv_sec = (tvp)->tv_usec = 0)

   int gettimeofday(struct timeval* tv, void* arg2) 
   {
      union {
         long long ns100;
         FILETIME ft;
      } now;
     
      GetSystemTimeAsFileTime (&now.ft);
      tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
      tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
     return (0);
   }
#endif

#endif


#define	TVR_SIZE 100
#define	TVN_SIZE 256

#define TFLAG_IDLE			1	
#define TFLAG_TERMINATE		2
#define TFLAG_INPROCESS		3

/*一个简单的链表定义*/
typedef	struct wlist
{
	struct wlist* pre;
	struct wlist* next;
}wlist;

/*timer 结构定义*/
typedef	struct wtimer
{
	wlist  list;							//	链表
	void*  key;								//	用户 填充key
	struct timeval time;					//	激发时间
	unsigned long period;					//	触发的时间间隔
	TIMER_EVENT	event;						//	callback函数
	int flag;								//	用来标识timer当前状态
	short flag_remove_by_exec_thtead;       //  当timerevent 正在执行时发生未决的remove
	int exec_thread_id;
}wtimer, *HWTIMER;		

/*第一级timer 数组*/
typedef	struct TVR
{
	int	index;
	wlist vec[TVR_SIZE];
}TVR;

/*第 2 3 4 5 级timer数组的定义*/
typedef	struct TVN
{
	int	index;
	wlist vec[TVN_SIZE];
}TVN;					

/*timer运行现场*/
typedef	struct wtimer_context
{	
	TVR	tv1;						//	0			-	1000 ms		
	TVN	tv2;						//	0x0			-	0xff s
	TVN	tv3;						//	0x100		-	0xffff s 
	TVN	tv4;						//	0x10000		-	0xffffff s
	TVN	tv5;						//	0x1000000	-	0xffffff s
	
	struct timeval timer_val;		//	timer	自己的计时，随timer的处理进程步进，
									//	和 curr_val的差是需要被	处理的时间段	
	os_mutex_t	mutex_timers;	//	用来同步对timer数组数据结构的处理	
	
	os_thread_t hthread;				//	扫描线程
	unsigned short exitflag;		//	线程的退出标志 非0 退出
	
	int	interval;					//	线程扫描的时间精度 10	ms - 1000	ms比较好
	
	MEMPOOL_HANDLE	hpool;			//	池子的句柄
	os_mutex_t	mutex_pool;		//  pool的同步保护
	
}wtimer_context, *HWTIMERCONTEXT;

/*---------------------------------------------------------------------------*/
/*全局变量定义*/
wtimer_context context,	*hctx;	// timer 数据结构的现场
/*---------------------------------------------------------------------------*/
/*局部函数声明*/
static unsigned int ZION_CALLBACK wtimer_thread(void*	param);
static wtimer*	malloc_timer();
static void free_timer(wtimer*	timer);
static TIMER get_handle(wtimer* timer); 
static wtimer* get_timer(TIMER handle);
static void cascade_timers(HWTIMERCONTEXT hctx, wlist *head);
static void wlist_add(wlist *list,	wtimer *timer);
static int wtimer_add_internal(HWTIMERCONTEXT hctx, wtimer	*timer);
static void timerthread_event(void* arg);
/*---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------- 
* Name     : timer_initialize 
* Comments : 初始化timer运行现场
* ----------------------------------------------------------------------------
* Parameters: [in]init_size timer的初始化个数,传给mempool_create作为第二个参数
*			   [in]interval timer的扫描时间间隔,ms为单位,推荐取值10-1000一般100
* Return    : ERR_NOERROR 成功 
*			   ERR_REPPOOL_CREATE ERR_THREAD_CREATE
* Remarks   : 必须在mempool初始化之后调用
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/
int	timer_init(int interval)
{
	int	irv;
	int	loopi;
	
	hctx = &context;
	
	//扫描精度毫秒和微妙的转换
	hctx->interval = interval *	1000;
	
	//初始化 timer数组数据结构				
	//tv1
	hctx->tv1.index	= 0;
	for(loopi = 0; loopi < TVR_SIZE; loopi++)
	{
		hctx->tv1.vec[loopi].pre = &(hctx->tv1.vec[loopi]);
		hctx->tv1.vec[loopi].next =	&(hctx->tv1.vec[loopi]);
	}
	//tv2
	hctx->tv2.index	= 1;
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		hctx->tv2.vec[loopi].pre = &(hctx->tv2.vec[loopi]);
		hctx->tv2.vec[loopi].next =	&(hctx->tv2.vec[loopi]);
	}
	//tv3
	hctx->tv3.index	= 1;
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		hctx->tv3.vec[loopi].pre = &(hctx->tv3.vec[loopi]);
		hctx->tv3.vec[loopi].next =	&(hctx->tv3.vec[loopi]);
	}
	//tv4
	hctx->tv4.index	= 1;
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		hctx->tv4.vec[loopi].pre = &(hctx->tv4.vec[loopi]);
		hctx->tv4.vec[loopi].next =	&(hctx->tv4.vec[loopi]);
	}
	//tv5
	hctx->tv5.index	= 1;
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		hctx->tv5.vec[loopi].pre = &(hctx->tv5.vec[loopi]);
		hctx->tv5.vec[loopi].next =	&(hctx->tv5.vec[loopi]);
	}	
	
	gettimeofday(&hctx->timer_val, NULL);
	
	//init index
	hctx->tv1.index = hctx->timer_val.tv_usec /10000;
	hctx->tv2.index = (hctx->timer_val.tv_sec & 0x000000ff) + 1;
	hctx->tv3.index = ((hctx->timer_val.tv_sec >> 8) & 0x000000ff) + 1;
	hctx->tv4.index = ((hctx->timer_val.tv_sec >> 16) & 0x000000ff) + 1;
	hctx->tv5.index = ((hctx->timer_val.tv_sec >> 24) & 0x000000ff) + 1;
	
	//printf("init index: tv1:%d tv2:%d tv3:%d tv4:%d tv5:%d", hctx->tv1.index, hctx->tv2.index, hctx->tv3.index, hctx->tv4.index, hctx->tv5.index);
	
	os_mutex_init(&hctx->mutex_timers);
	os_mutex_init(&hctx->mutex_pool);
	
	hctx->exitflag = 0;
	
	//init pool
	hctx->hpool	= mempool_create("TIMER", sizeof(wtimer), 0);
	if(hctx->hpool == NULL)
	{
		//SYSLOG(LOG_ERROR, MODULE_NAME, "timer_initialize : mempool_create return NULL\n");
		return ERR_UNKNOWN;
	}
	
	//at last start	timer thread
	irv	= os_thread_begin(&hctx->hthread, wtimer_thread, hctx);
	if(irv != 0)
	{
		//线程创建失败
		return ERR_UNKNOWN;
	}
	return(ERR_NOERROR);
}

/* ---------------------------------------------------------------------------- 
* Name     : timer_uninitialize
* Comments : 结束和清理timer现场
* ----------------------------------------------------------------------------
* Parameters:
* Return    : ERR_NOERROR
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
int	timer_final()
{
	int	loopi;
	wlist	*head, *curr;
	
	hctx->exitflag = 1;
	os_thread_wait(hctx->hthread, NULL);
	os_mutex_destroy(&hctx->mutex_timers);
	os_mutex_destroy(&hctx->mutex_pool);
	
	//free all timer
	for(loopi	=0 ; loopi < TVR_SIZE; loopi++)
	{
		head = &hctx->tv1.vec[loopi];
		curr = hctx->tv1.vec[loopi].pre;
	
		while(curr != head)
		{
			curr = curr->pre;
			while(((wtimer*)curr->next)->flag != TFLAG_IDLE && 
				((wtimer*)curr->next)->flag != TFLAG_TERMINATE);
			free_timer((wtimer*)curr->next);		
		}			
	}
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		head = &hctx->tv2.vec[loopi];
		curr = hctx->tv2.vec[loopi].pre;
	
		while(curr != head)
		{
			curr = curr->pre;
			while(((wtimer*)curr->next)->flag != TFLAG_IDLE && 
				((wtimer*)curr->next)->flag != TFLAG_TERMINATE);
			free_timer((wtimer*)curr->next);		
		}			
	}
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		head = &hctx->tv3.vec[loopi];
		curr = hctx->tv3.vec[loopi].pre;
	
		while(curr != head)
		{
			curr = curr->pre;
			while(((wtimer*)curr->next)->flag != TFLAG_IDLE && 
				((wtimer*)curr->next)->flag != TFLAG_TERMINATE);
			free_timer((wtimer*)curr->next);		
		}			
	}
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		head = &hctx->tv4.vec[loopi];
		curr = hctx->tv4.vec[loopi].pre;
	
		while(curr != head)
		{
			curr = curr->pre;
			while(((wtimer*)curr->next)->flag != TFLAG_IDLE && 
				((wtimer*)curr->next)->flag != TFLAG_TERMINATE);
			free_timer((wtimer*)curr->next);	
		}			
	}
	for(loopi = 0; loopi < TVN_SIZE; loopi++)
	{
		head = &hctx->tv5.vec[loopi];
		curr = hctx->tv5.vec[loopi].pre;
	
		while(curr != head)
		{
			curr = curr->pre;
			while(((wtimer*)curr->next)->flag != TFLAG_IDLE && 
				((wtimer*)curr->next)->flag != TFLAG_TERMINATE);
			free_timer((wtimer*)curr->next);		
		}			
	}
	mempool_destroy(hctx->hpool);	
	return(ERR_NOERROR);
}

/* ---------------------------------------------------------------------------- 
* Name     : timer_add
* Comments : 注册一个timer
* ----------------------------------------------------------------------------
* Parameters: [in]duetime 超期时间,timer注册到第一次触发的时间间隔
*			   [in]period  周期,对TIMER_MULTSHOOT有效,timer的触发周期
*			   [in]flags   TIMER_ONESHOOT 或 TIMER_MULTSHOOT,指定timer是一次性
*						   还是循环使用
*			   [in]event   timer触发时的回调函数
*			   [in]key 	   作为TIMER_EVENT的参数
* Return    : timer的句柄,返回NULL失败
* Remarks   : 
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
TIMER timer_add(unsigned long duetime, unsigned long period, TIMER_EVENT event, void* key)
{
	int	rv;
	wtimer *timer;	
		
	if(duetime == 0 && period == 0)
	{
		return NULL;
	}
	
	//当duetime为0 取	period的值
	duetime	= duetime == 0 ? period	: duetime;
	
	//开辟 并	初始化 一个	timer结构
	timer = malloc_timer();
	
	timer->list.pre	= NULL;
	timer->list.next = NULL;	
	timer->key = key;
	timer->event = event;	
	gettimeofday(&timer->time, NULL);	
	timer->time.tv_sec += (timer->time.tv_usec + duetime * 1000) / 1000000;
	timer->time.tv_usec	= (timer->time.tv_usec + duetime * 1000) % 1000000;
	timer->period = period;
	timer->flag = TFLAG_IDLE;
	timer->flag_remove_by_exec_thtead = 0;
	timer->exec_thread_id = -1;
	
	//printf("timer_add key = %d duetime = %d	", key, duetime);
	//printf("time: %d %d\n", timer->time.tv_sec, timer->time.tv_usec);
	
	//add to list
	os_mutex_lock(&hctx->mutex_timers);
	rv = wtimer_add_internal(hctx, timer);
	os_mutex_unlock(&hctx->mutex_timers);	
	
	if(rv != 0)
	{
		free_timer(timer);
		return NULL;
	}
	
	return get_handle(timer);	

}

/* ---------------------------------------------------------------------------- 
* Name     : timer_remove
* Comments : 取消一个timer
* ----------------------------------------------------------------------------
* Parameters: handle timer的句柄
* Return    : ERR_NOERROR 成功
* Remarks   : TIMER_ONESHOOT的timer在激发之后会自动移除,不用显示的调用timer_remove
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
int	timer_remove(TIMER handle)
{
	int temp_flag;
	wtimer*	timer;	

	timer = get_timer(handle);

	//printf("cancle timer key = %d \n", timer->key);
	
	if(timer->exec_thread_id != -1 &&
	   timer->exec_thread_id == threadpool_getindex())
	{
		//此时正处于timer执行线程中必定是TFLAG_INPROCESS状态
		timer->flag_remove_by_exec_thtead = 1;
		return ERR_NOERROR;
	}
	
	temp_flag = atom_cas((unsigned int*)&timer->flag, TFLAG_TERMINATE, TFLAG_IDLE);
	
	if(temp_flag == TFLAG_IDLE)
	{
		return ERR_NOERROR;
	}
	else if(temp_flag == TFLAG_INPROCESS)
	{
		return ERR_BLOCK;
	}
	else
	{
		//当 one shut timer 已经被释放
		return ERR_NOERROR;
	}
}

/* ---------------------------------------------------------------------------- 
* Name     : wtimer_thread
* Comments : timer	扫描 线程
* ----------------------------------------------------------------------------
* Parameters: [in]param timer的运行现场
* Return    : 
* Remarks   : HWTIMERCONTEXT包括了用来存放timer的5个数组,而wtimer_thread通过
*			   扫描数组来确定已经到期需要触发的timer
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
unsigned int ZION_CALLBACK wtimer_thread(void* param)
{
	HWTIMERCONTEXT hctx;
	struct timeval time, ttime;
	wlist	*head, *curr, *fire;
	int tflag;
	unsigned long sleeptime;
	
	hctx = (HWTIMERCONTEXT)param;
	
	//printf("wtimer_thread, start\n");
	
	while(hctx->exitflag ==	0)
	{		
		//获取 当前时间
		gettimeofday(&time,	NULL);
		
		os_mutex_lock(&hctx->mutex_timers);
		
		//printf("wtimer_thread currtime:	%d %d, timertime: %d %d\n",	time.tv_sec, time.tv_usec, hctx->timer_val.tv_sec, hctx->timer_val.tv_usec);
		//printf("tv1	from index = %d\n", hctx->tv1.index);
		
		//处理已经到期的 timer
		while(timercmp(&hctx->timer_val, &time,	<=))
		{						
			head = &(hctx->tv1.vec[hctx->tv1.index]);
			curr = head->next;
			
			//printf("wtimer_thread tv1 index = %d\n", hctx->tv1.index);
			
			while(curr != head)
			{
				fire = curr;
				
				curr->pre->next	= curr->next;
				curr->next->pre	= curr->pre;
				curr = curr->next;	
				
				//call back				
				
				//fire event
				tflag = atom_cas((unsigned int*)(&((wtimer*)fire)->flag), TFLAG_INPROCESS, TFLAG_IDLE);
				if(tflag==TFLAG_TERMINATE)
				{
					free_timer((wtimer*)fire);
				}
				else 
				{
					if(tflag == TFLAG_IDLE)
					{
						//printf("fire event key=%p\n", ((wtimer*)fire)->key);
						if(threadpool_queueitem(timerthread_event, fire)!=ERR_NOERROR) {
							assert(0);
						}
					}
					//检查是否oneshoot timer
					if(((wtimer*)fire)->period)
					{
						//reset	timer
						((wtimer*)fire)->time.tv_sec +=	(((wtimer*)fire)->time.tv_usec + ((wtimer*)fire)->period * 1000) / 1000000;
						((wtimer*)fire)->time.tv_usec = (((wtimer*)fire)->time.tv_usec + ((wtimer*)fire)->period * 1000) % 1000000;
						wtimer_add_internal(hctx,	((wtimer*)fire));
						
						//printf("wtimer_add_internal %p %u:%u\n", ((wtimer*)fire)->key, (unsigned int)((wtimer*)fire)->time.tv_sec, (unsigned int)((wtimer*)fire)->time.tv_usec);					
					}
					//else if(((wtimer*)fire)->flags & TIMER_ONESHOOT)
					//{
						//free_timer((wtimer*)fire);
					//}	
				}				
			}
			
			
			//printf("step1\n");
			
			hctx->tv1.index = (hctx->tv1.index+1) % 100;
			hctx->timer_val.tv_sec += (hctx->timer_val.tv_usec + 10000)	/	1000000;
			hctx->timer_val.tv_usec	= (hctx->timer_val.tv_usec + 10000)	%	1000000;
			
			//printf("step2 %d\n", hctx->tv1.index);
			if(hctx->tv1.index == 0)
			{				
				//执行链表迁移
				//printf("cascade_timers tv2 index = %d\n",	hctx->tv2.index);
				cascade_timers(hctx, &(hctx->tv2.vec[hctx->tv2.index]));
				hctx->tv2.index = (hctx->tv2.index+1) & 0x000000ff;
				
				if(hctx->tv2.index == 1)
				{
					//printf("cascade_timers tv3 index = %d\n",	hctx->tv3.index);
					cascade_timers(hctx, &(hctx->tv3.vec[hctx->tv3.index]));
					hctx->tv3.index = (hctx->tv3.index+1) & 0x000000ff;
					
					if(hctx->tv3.index == 1)
					{
						cascade_timers(hctx, &(hctx->tv4.vec[hctx->tv4.index]));
						hctx->tv4.index = (hctx->tv4.index+1) & 0x000000ff;
						
						if(hctx->tv4.index == 1)
						{
							cascade_timers(hctx, &(hctx->tv5.vec[hctx->tv5.index]));
							hctx->tv5.index = (hctx->tv5.index+1) & 0x000000ff;
							
							//假定 tv5不会被 溢出	
							if(hctx->tv5.index == 1)	
							{
								hctx->tv5.index	= 1;
							}			
						}				
					}					
				}
			}
			//printf("step3\n");			
		}
		
		//printf("to index %d\n", hctx->tv1.index);
		
		os_mutex_unlock(&hctx->mutex_timers);
		
		//printf("step4\n");
		
		gettimeofday(&ttime, NULL);
		
		sleeptime = (ttime.tv_sec - time.tv_sec)*1000000 + (ttime.tv_usec - time.tv_usec);
		sleeptime = hctx->interval - sleeptime;
		
		if(sleeptime > 0)
		{
			os_sleep(hctx->interval/1000);
		}			
		
		//printf("step5\n");
	}	
		
	
	return 0;
}

/* ---------------------------------------------------------------------------- 
* Name     : malloc_timer(
* Comments : 维护 timer 的	内存 分配
* ----------------------------------------------------------------------------
* Parameters:
* Return    : timer结构的指针
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static wtimer*	malloc_timer()
{
	wtimer *item;	
	
	os_mutex_lock(&hctx->mutex_pool);
	//printf("malloc_timer\n");
	item = (wtimer*)mempool_alloc(hctx->hpool);
	os_mutex_unlock(&hctx->mutex_pool);
	if(item	== NULL)
	{
		return NULL;
	}
	
	return item;
}

/* ---------------------------------------------------------------------------- 
* Name     : free_timer
* Comments : 和malloc_timer相对应的释放
* ----------------------------------------------------------------------------
* Parameters:
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static void free_timer(wtimer*	timer)
{	
	
	//printf("free_timer key = %d\n", timer->key);	
	
	mempool_free(hctx->hpool, timer);	
}

/* ---------------------------------------------------------------------------- 
* Name     : get_handle
* Comments : 将wtimer* 转换为TIMER,仅仅是句柄的转换
* ----------------------------------------------------------------------------
* Parameters:
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static TIMER get_handle(wtimer* timer)
{
	return (TIMER)timer;	 
}

/* ---------------------------------------------------------------------------- 
* Name     : get_timer
* Comments : 对应get_handle的逆过程
* ----------------------------------------------------------------------------
* Parameters:
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static wtimer*	get_timer(TIMER	handle)
{
	return (wtimer*)handle;
}

/* ---------------------------------------------------------------------------- 
* Name     : cascade_timers
* Comments : 将一个wlist中的所有timer重新插入到维护timer的数组中
* ----------------------------------------------------------------------------
* Parameters: [in]hctx
*			   [in]head
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static void cascade_timers(HWTIMERCONTEXT hctx, wlist *head)
{
	wlist *curr, *temp;
	
	curr = head->next;
	
	//printf("cascade_timers 1 curr:%x head:%x head->pre:%x\n", curr, head, head->pre);
	
	while(curr != head)
	{
		//printf("cascade_timers 2 curr:%x next:%x\n", curr, curr->next);
		
		temp = curr;
		curr->pre->next	= curr->next;
		curr->next->pre	= curr->pre;
		curr = curr->next;		
		//printf("cascade_timers 3 \n");
		wtimer_add_internal(hctx, (wtimer*)temp);
		//printf("cascade_timers 4 \n");		
	}
}

/* ---------------------------------------------------------------------------- 
* Name     : wlist_add
* Comments : 将 timer 添加到list的尾部
* ----------------------------------------------------------------------------
* Parameters:
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static void wlist_add(wlist *list,	wtimer *timer)
{
	timer->list.next = list;
	timer->list.pre	= list->pre;
	list->pre->next	= (wlist*)timer;
	list->pre = (wlist*)timer;
}

/**/
/* ---------------------------------------------------------------------------- 
* Name     : wtimer_add_internal
* Comments : 相当重要的一个函数,将timer添加到 五个数组的恰当位置
* ----------------------------------------------------------------------------
* Parameters:
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
static int wtimer_add_internal(HWTIMERCONTEXT hctx, wtimer	*timer)
{
	unsigned long exp_ms;
	unsigned long exp_s;
	unsigned int index;
	
	//计算时间差
	if(timer->time.tv_usec < hctx->timer_val.tv_usec)
	{
		//当 有	进位情况 发生
		exp_ms = (timer->time.tv_usec - hctx->timer_val.tv_usec	+ 1000000) / 1000;
		exp_s = timer->time.tv_sec - hctx->timer_val.tv_sec	- 1;		
	}
	else
	{
		exp_ms = (timer->time.tv_usec - hctx->timer_val.tv_usec) / 1000;
		exp_s = timer->time.tv_sec - hctx->timer_val.tv_sec;
	}
	
	//printf("wtimer_add_internal exp_ms = %d exp_s = %d key = %p\n", (int)exp_ms, (int)exp_s, timer->key);
	
	//tv1
	if(exp_ms >=0 && exp_s == 0)
	{
		index = timer->time.tv_usec /10000;
		wlist_add(&(hctx->tv1.vec[index]), timer);	
		//printf("wtimer_add_internal tv1	index = %d key = %d\n", index, timer->key);	
	}
	//t2
	else if(exp_s > 0 && exp_s < 0x00000100)
	{
		index = (( timer->time.tv_sec ) & 0x000000ff); 
		wlist_add(&(hctx->tv2.vec[index]), timer);
		//printf("wtimer_add_internal tv2 index = %d key = %d\n", index, timer->key);
	}
	//t3
	else if(exp_s < 0x00010000)	
	{
		index = ( timer->time.tv_sec >> 8 ) & 0x000000ff;
		wlist_add(&(hctx->tv3.vec[index]), timer);
		//printf("wtimer_add_internal tv3 index = %d key = %d\n", index, timer->key);		
	}
	//t4
	else if(exp_s < 0x01000000)
	{
		index = ( timer->time.tv_sec >> (8 * 2) ) & 0x000000ff;
		wlist_add(&(hctx->tv4.vec[index]), timer);	
		//printf("wtimer_add_internal tv4 index = %d key = %d\n", index, timer->key);	
	}
	//当出现负值,这是有可能的
	else if((signed long)exp_s < 0 || (signed long)exp_ms < 0)
	{
		//when timer passed
		wlist_add(&(hctx->tv1.vec[hctx->tv1.index]), timer);
		//printf("passed timer add key = %d", timer->key);
	}	
	//t5
	else if(exp_s <= 0xffffffff)
	{
		index = ( timer->time.tv_sec	>> (8 * 3) ) & 0x000000ff;
		wlist_add(&(hctx->tv5.vec[index]), timer);	
		//printf("wtimer_add_internal tv5 index = %d key = %d\n", index, timer->key);		
	}
		
	return 0;
}

/* ---------------------------------------------------------------------------- 
* Name     : timerthread_event
* Comments : threadpool的事件回调函数,参看threadpool定义
* ----------------------------------------------------------------------------
* Parameters: [in]msgid
*			   [in]arg1
*			   [in]arg2
* Return    :
* Remarks   :
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/  
void timerthread_event(void* arg)
{
	wtimer *timer;

	timer =	(wtimer*)arg;
	
	timer->exec_thread_id = threadpool_getindex();
	
	//printf("timerthread_event event=%p start\n", timer->event);
	timer->event(get_handle(timer),	timer->key);
	//printf("timerthread_event event=%p end\n", timer->event);
	
	timer->exec_thread_id = -1;
	
	//set flag
	if(timer->period)
	{
		if(timer->flag_remove_by_exec_thtead == 1)
		{
			timer->flag = TFLAG_TERMINATE;
		}
		else
		{
			timer->flag = TFLAG_IDLE;
		}
	}
	else
	{
		timer->flag = TFLAG_IDLE;
		
		free_timer(timer);
	}

}

