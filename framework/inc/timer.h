#ifndef _TIMER_INCLUDE_
#define _TIMER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* TIMER;

typedef void (*TIMER_EVENT)(TIMER handle, void* key);

ZION_API int timer_init(int	interval);
ZION_API int timer_final();

ZION_API TIMER timer_add(unsigned long duetime, unsigned long period, TIMER_EVENT event, void* key);
ZION_API int timer_remove(TIMER handle);

#define	timer_force_remove(h)	while(timer_remove(h)==ERR_BLOCK)

#ifdef __cplusplus
}
#endif

#endif

