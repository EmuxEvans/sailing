#include <stdio.h>
#include <stdlib.h>

#include "../inc/skates.h"
#include "../inc/octopus.h"

static void ontimer(GAMEROOM* room, unsigned int state_id, int timer_id);
static int onnotify(GAMEROOM* room, int code, unsigned int state_id, GAMEROOM_MEMBER* memb);
static void rpcsend(GAMEROOM* room, GAMEROOM_MEMBER* memb, void* data, int data_len);
static void rpcleave(GAMEROOM* room, GAMEROOM_MEMBER* member);

static
GAMEROOM_CLASS_BEGIN(demoroom)
	GAMEROOM_CLASS_CALLBACK(rpcsend, rpcleave)
	GAMEROOM_CLASS_OPTION(GAMEROOM_AUTO_FREE, 100)
	GAMEROOM_CLASS_STATE_BEGIN(1)
		GAMEROOM_CLASS_STATE(100, GAMEROOM_AUTO_ATTACH|GAMEROOM_AUTO_DETACH, 0, 0, ontimer, onnotify)
	GAMEROOM_CLASS_STATE_END()
GAMEROOM_CLASS_END(classname)

void doit()
{
	GAMEROOM* room;
	OBJECT_ID objid;
	GAMEROOM_MEMBER* member;

	os_time_t a, b, c;
	os_time_get(&a);
	os_sleep(100);
	os_time_get(&b);
	c = b - a;

	room = gameroom_create(&demoroom, NULL, NULL);
	member = gameroom_member_create(room, NULL, &objid);
	gameroom_objid(room, &objid);

	gameroom_timer_set(room, 9808, 100, 1000, 200);

	gameroom_unlock(room);

	getchar();

	room = gameroom_lock(&objid);
	gameroom_member_detach(room, 100, gameroom_member_getbyindex(room, 0));
	gameroom_member_release(room, gameroom_member_getbyindex(room, 0));
	gameroom_unlock(room);
}

int main(int argc, char* argv[])
{
	mempool_init();
	threadpool_init(10);
	timer_init(10);
	gameroom_init();
	gameroom_class_init(&demoroom);

	doit();

	gameroom_class_destroy(&demoroom);
	gameroom_final();
	timer_final();
	threadpool_final();
	mempool_final();
	return(0);
}

os_time_t a = 0;

void ontimer(GAMEROOM* room, unsigned int state_id, int timer_id)
{
	os_time_t b, c;

	return;

	os_time_get(&b);
	c = b - a;
	a = b;

	printf("ontimer %d\n", (int)c);
}

int onnotify(GAMEROOM* room, int code, unsigned int state_id, GAMEROOM_MEMBER* memb)
{
	return ERR_NOERROR;
}

void rpcsend(GAMEROOM* room, GAMEROOM_MEMBER* memb, void* data, int data_len)
{
}

void rpcleave(GAMEROOM* room, GAMEROOM_MEMBER* member)
{
}
