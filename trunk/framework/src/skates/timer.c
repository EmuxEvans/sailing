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

/*һ���򵥵�������*/
typedef	struct wlist
{
	struct wlist* pre;
	struct wlist* next;
}wlist;

/*timer �ṹ����*/
typedef	struct wtimer
{
	wlist  list;							//	����
	void*  key;								//	�û� ���key
	struct timeval time;					//	����ʱ��
	unsigned long period;					//	������ʱ����
	TIMER_EVENT	event;						//	callback����
	int flag;								//	������ʶtimer��ǰ״̬
	short flag_remove_by_exec_thtead;       //  ��timerevent ����ִ��ʱ����δ����remove
	int exec_thread_id;
}wtimer, *HWTIMER;		

/*��һ��timer ����*/
typedef	struct TVR
{
	int	index;
	wlist vec[TVR_SIZE];
}TVR;

/*�� 2 3 4 5 ��timer����Ķ���*/
typedef	struct TVN
{
	int	index;
	wlist vec[TVN_SIZE];
}TVN;					

/*timer�����ֳ�*/
typedef	struct wtimer_context
{	
	TVR	tv1;						//	0			-	1000 ms		
	TVN	tv2;						//	0x0			-	0xff s
	TVN	tv3;						//	0x100		-	0xffff s 
	TVN	tv4;						//	0x10000		-	0xffffff s
	TVN	tv5;						//	0x1000000	-	0xffffff s
	
	struct timeval timer_val;		//	timer	�Լ��ļ�ʱ����timer�Ĵ�����̲�����
									//	�� curr_val�Ĳ�����Ҫ��	�����ʱ���	
	os_mutex_t	mutex_timers;	//	����ͬ����timer�������ݽṹ�Ĵ���	
	
	os_thread_t hthread;				//	ɨ���߳�
	unsigned short exitflag;		//	�̵߳��˳���־ ��0 �˳�
	
	int	interval;					//	�߳�ɨ���ʱ�侫�� 10	ms - 1000	ms�ȽϺ�
	
	MEMPOOL_HANDLE	hpool;			//	���ӵľ��
	os_mutex_t	mutex_pool;		//  pool��ͬ������
	
}wtimer_context, *HWTIMERCONTEXT;

/*---------------------------------------------------------------------------*/
/*ȫ�ֱ�������*/
wtimer_context context,	*hctx;	// timer ���ݽṹ���ֳ�
/*---------------------------------------------------------------------------*/
/*�ֲ���������*/
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
* Comments : ��ʼ��timer�����ֳ�
* ----------------------------------------------------------------------------
* Parameters: [in]init_size timer�ĳ�ʼ������,����mempool_create��Ϊ�ڶ�������
*			   [in]interval timer��ɨ��ʱ����,msΪ��λ,�Ƽ�ȡֵ10-1000һ��100
* Return    : ERR_NOERROR �ɹ� 
*			   ERR_REPPOOL_CREATE ERR_THREAD_CREATE
* Remarks   : ������mempool��ʼ��֮�����
* ----------------------------------------------------------------------------	
* Function flow:
* ----------------------------------------------------------------------------
*/
int	timer_init(int interval)
{
	int	irv;
	int	loopi;
	
	hctx = &context;
	
	//ɨ�辫�Ⱥ����΢���ת��
	hctx->interval = interval *	1000;
	
	//��ʼ�� timer�������ݽṹ				
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
		//�̴߳���ʧ��
		return ERR_UNKNOWN;
	}
	return(ERR_NOERROR);
}

/* ---------------------------------------------------------------------------- 
* Name     : timer_uninitialize
* Comments : ����������timer�ֳ�
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
* Comments : ע��һ��timer
* ----------------------------------------------------------------------------
* Parameters: [in]duetime ����ʱ��,timerע�ᵽ��һ�δ�����ʱ����
*			   [in]period  ����,��TIMER_MULTSHOOT��Ч,timer�Ĵ�������
*			   [in]flags   TIMER_ONESHOOT �� TIMER_MULTSHOOT,ָ��timer��һ����
*						   ����ѭ��ʹ��
*			   [in]event   timer����ʱ�Ļص�����
*			   [in]key 	   ��ΪTIMER_EVENT�Ĳ���
* Return    : timer�ľ��,����NULLʧ��
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
	
	//��duetimeΪ0 ȡ	period��ֵ
	duetime	= duetime == 0 ? period	: duetime;
	
	//���� ��	��ʼ�� һ��	timer�ṹ
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
* Comments : ȡ��һ��timer
* ----------------------------------------------------------------------------
* Parameters: handle timer�ľ��
* Return    : ERR_NOERROR �ɹ�
* Remarks   : TIMER_ONESHOOT��timer�ڼ���֮����Զ��Ƴ�,������ʾ�ĵ���timer_remove
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
		//��ʱ������timerִ���߳��бض���TFLAG_INPROCESS״̬
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
		//�� one shut timer �Ѿ����ͷ�
		return ERR_NOERROR;
	}
}

/* ---------------------------------------------------------------------------- 
* Name     : wtimer_thread
* Comments : timer	ɨ�� �߳�
* ----------------------------------------------------------------------------
* Parameters: [in]param timer�������ֳ�
* Return    : 
* Remarks   : HWTIMERCONTEXT�������������timer��5������,��wtimer_threadͨ��
*			   ɨ��������ȷ���Ѿ�������Ҫ������timer
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
		//��ȡ ��ǰʱ��
		gettimeofday(&time,	NULL);
		
		os_mutex_lock(&hctx->mutex_timers);
		
		//printf("wtimer_thread currtime:	%d %d, timertime: %d %d\n",	time.tv_sec, time.tv_usec, hctx->timer_val.tv_sec, hctx->timer_val.tv_usec);
		//printf("tv1	from index = %d\n", hctx->tv1.index);
		
		//�����Ѿ����ڵ� timer
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
					//����Ƿ�oneshoot timer
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
				//ִ������Ǩ��
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
							
							//�ٶ� tv5���ᱻ ���	
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
* Comments : ά�� timer ��	�ڴ� ����
* ----------------------------------------------------------------------------
* Parameters:
* Return    : timer�ṹ��ָ��
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
* Comments : ��malloc_timer���Ӧ���ͷ�
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
* Comments : ��wtimer* ת��ΪTIMER,�����Ǿ����ת��
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
* Comments : ��Ӧget_handle�������
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
* Comments : ��һ��wlist�е�����timer���²��뵽ά��timer��������
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
* Comments : �� timer ��ӵ�list��β��
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
* Comments : �൱��Ҫ��һ������,��timer��ӵ� ��������ǡ��λ��
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
	
	//����ʱ���
	if(timer->time.tv_usec < hctx->timer_val.tv_usec)
	{
		//�� ��	��λ��� ����
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
	//�����ָ�ֵ,�����п��ܵ�
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
* Comments : threadpool���¼��ص�����,�ο�threadpool����
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

