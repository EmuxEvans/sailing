#include <string.h>
#include <assert.h>

#include "../inc/skates.h"
#include "../inc/octopus.h"

struct GAMEROOM {
	HASHMAP_ITEM			item;
	const GAMEROOM_CLASS*	gc;
	void*					fud;
	OBJECT_ID				objid;
	char					strid[GAMEROOM_STRID_LEN+1];
	GAMEROOM_MEMBER**		members;
	unsigned int			member_count;
	unsigned int			flags;

	struct {
		unsigned int	timer_id;
		int				state_id;
		os_time_t		duetime;
		int				period;
	}						timer_list[GAMEROOM_TIMER_COUNT];
};

struct GAMEROOM_MEMBER {
	GAMEROOM*				gm;
	unsigned int			seq;
	char					strid[GAMEROOM_STRID_LEN+1];
	unsigned int			state_flags;
	unsigned int			mask;
	OBJECT_ID				objid;
	RPCNET_GROUP*			group;
};

static struct {
	GAMEROOM*	room;
	os_mutex_t	mtx;
	os_time_t	duetime;
} room_list[GAMEROOM_MAX_COUNT];
static os_mutex_t		room_list_mtx;
static TIMER			room_list_timer;
static HASHMAP_HANDLE	room_strid_map;
static os_mutex_t		room_strid_mtx;
static unsigned int		member_seq = 0;
static int				room_seq = 0;

static GAMEROOM* gameroom_lock_byindex(int index);
static void gameroom_free(GAMEROOM* room, unsigned int state);
static void timer_proc(TIMER handle, void* key);
static void calc_duetime(GAMEROOM* room);

void gameroom_init()
{
	int i;
	os_mutex_init(&room_list_mtx);
	os_mutex_init(&room_strid_mtx);
	memset(&room_list, 0, sizeof(room_list));
	for(i=0; i<sizeof(room_list)/sizeof(room_list[0]); i++) {
		os_mutex_init(&room_list[i].mtx);
	}

	room_strid_map = hashmap_create(8, NULL, 0);
	assert(room_strid_map!=NULL);

	room_list_timer = timer_add(50, GAMEROOM_TIMER_INTERVAL, timer_proc, NULL);
	assert(room_list_timer!=NULL);
}

void gameroom_final()
{
	int i;

	assert(room_list_timer!=NULL);
	timer_force_remove(room_list_timer);

	assert(room_strid_map!=NULL);
	hashmap_destroy(room_strid_map);

	for(i=0; i<sizeof(room_list)/sizeof(room_list[0]); i++) {
		os_mutex_destroy(&room_list[i].mtx);
	}

	os_mutex_destroy(&room_strid_mtx);
	os_mutex_destroy(&room_list_mtx);
}

int gameroom_class_init(GAMEROOM_CLASS* gc)
{
	unsigned int i;

	assert(gc->state_count>0 && gc->state_count<=GAMEROOM_STATE_COUNT);
	if(gc->state_count<=0 || gc->state_count>GAMEROOM_STATE_COUNT) return ERR_INVALID_DATA;

	gc->room_size = gc->memb_size = 0;
	memset(gc->state_map, 0, sizeof(gc->state_map));
	for(i=0; i<gc->state_count; i++) {
		assert(gc->states[i].id<GAMEROOM_STATE_MAX);
		assert(gc->state_map[gc->states[i].id]==NULL);
		if(gc->states[i].id>=GAMEROOM_STATE_MAX)	return ERR_INVALID_DATA;
		if(gc->state_map[gc->states[i].id]!=NULL)	return ERR_INVALID_DATA;

		assert(gc->states[i].OnTimer!=NULL);
		assert(gc->states[i].OnNotify!=NULL);
		if(gc->states[i].OnTimer==NULL)				return ERR_INVALID_DATA;
		if(gc->states[i].OnNotify==NULL)		return ERR_INVALID_DATA;

		gc->states[i].index = i;
		gc->states[i].state_offset = gc->room_size;
		gc->room_size += gc->states[i].state_size;
		gc->states[i].member_offset = gc->memb_size;
		gc->memb_size += gc->states[i].member_size;

		gc->state_map[gc->states[i].id] = &gc->states[i];
	}

	gc->room_pool = mempool_create("GAMEROOM_ROOM", sizeof(GAMEROOM)+gc->room_size+sizeof(GAMEROOM_MEMBER*)*gc->member_max, 0);
	gc->memb_pool = mempool_create("GAMEROOM_MEMB", sizeof(GAMEROOM_MEMBER)+gc->memb_size, 0);
	assert(gc->room_pool);
	assert(gc->memb_pool);
	if(gc->room_pool==NULL || gc->memb_pool==NULL) {
		if(gc->room_pool!=NULL) mempool_destroy(gc->room_pool);
		if(gc->memb_pool!=NULL) mempool_destroy(gc->memb_pool);
		gc->room_pool = NULL;
		gc->memb_pool = NULL;
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int gameroom_class_destroy(GAMEROOM_CLASS* gc)
{
	mempool_destroy(gc->room_pool);
	mempool_destroy(gc->memb_pool);
	gc->room_pool = NULL;
	gc->memb_pool = NULL;
	return ERR_NOERROR;
}

int gameroom_class_get_count(const GAMEROOM_CLASS* gc)
{
	return gc->room_count;
}

int gameroom_class_shutdown(GAMEROOM_CLASS* gc)
{
	unsigned int i;
	GAMEROOM* room;

	os_mutex_lock(&room_list_mtx);
	for(i=0; i<sizeof(room_list)/sizeof(room_list[0]); i++) {
		if(room_list[i].room==NULL) continue;
		if(room_list[i].room->gc!=gc) continue;

		room = gameroom_lock_byindex(i);
		gameroom_notify(room, GAMEROOM_NOTIFY_SHUTDOWN, NULL);
		gameroom_destroy(room);
		gameroom_unlock(room);
	}
	os_mutex_unlock(&room_list_mtx);

	return ERR_NOERROR;
}

GAMEROOM* gameroom_create(GAMEROOM_CLASS* gc, const char* strid, void* fud)
{
	GAMEROOM* room;
	unsigned int index, i;

	if(strid!=NULL) {
		if(strlen(strid)>GAMEROOM_STRID_LEN || strid[0]=='\0') {
			assert(0);
			return NULL;
		}
	}

	room = NULL;
	os_mutex_lock(&room_list_mtx);

	os_mutex_lock(&room_strid_mtx);
	if(hashmap_get(room_strid_map, strid, (unsigned int)strlen(strid))!=NULL) {
		os_mutex_unlock(&room_list_mtx);
		os_mutex_unlock(&room_strid_mtx);
		return NULL;
	}
	os_mutex_unlock(&room_strid_mtx);

	for(index=0; index<sizeof(room_list)/sizeof(room_list[0]); index++) {
		if(room_list[index].room==NULL) {
			room = (GAMEROOM*)mempool_alloc(gc->room_pool);
			if(room==NULL) break;

			room->gc = gc;
			room->objid.index = index;
			room->objid.seq = room_seq++;
			rpcnet_group_get_endpoint(rpcnet_getgroup(NULL), &room->objid.address);
			room->members = (GAMEROOM_MEMBER**)((unsigned char*)(room+1) + gc->room_size);
			memset(room->members, 0, sizeof(room->members[0])*gc->member_max);
			room->member_count = 0;
			room->flags = 0;
			memset(room->timer_list, 0, sizeof(room->timer_list));

			memset(room->strid, 0, sizeof(room->strid));
			if(strid!=NULL) {
				strcpy(room->strid, strid);
				HASHMAP_ITEM_SETPTR(&room->item, room);
				os_mutex_lock(&room_strid_mtx);
				hashmap_add_unsafe(room_strid_map, &room->item, strid, (unsigned int)strlen(strid));
				os_mutex_unlock(&room_strid_mtx);
			}

			room_list[index].room = room;
			os_mutex_lock(&room_list[index].mtx);
			break;
		}
	}
	os_mutex_unlock(&room_list_mtx);

	for(i=0; i<room->gc->state_count; i++) {
		if(room->gc->states[i].OnNotify==NULL) continue;

		if(room->gc->states[i].OnNotify(room, GAMEROOM_NOTIFY_INIT, room->gc->states[i].id, NULL)!=ERR_NOERROR) {
			break;
		}
	}
	if(i!=room->gc->state_count) {
		gameroom_free(room, i);
		os_mutex_unlock(&room_list[index].mtx);
		return NULL;
	}

	return room;
}

void gameroom_destroy(GAMEROOM* room)
{
	room->flags |= GAMEROOM_DELETE;
}

void gameroom_free(GAMEROOM* room, unsigned int state)
{
	unsigned int index, i;

	index = room->objid.index;

	if(room->strid[0]!='\0') {
		os_mutex_lock(&room_strid_mtx);
		hashmap_erase(room_strid_map, &room->item);
		os_mutex_unlock(&room_strid_mtx);
	}

	for(i=0; i<room->gc->member_max; i++) {
		if(room->members[i]==NULL) continue;
		gameroom_member_delete(room, room->members[i]);
	}

	for(;;) {
		if(state==0) break;
		state--;

		if(room->gc->states[state].OnNotify==NULL) continue;
		room->gc->states[state].OnNotify(room, GAMEROOM_NOTIFY_INIT, room->gc->states[state].id, NULL);
	}

	mempool_free(room->gc->room_pool, room);
	room_list[index].room = NULL;
}

const char* gameroom_get_strid(GAMEROOM* room)
{
	return room->strid;
}

void* gameroom_get_fud(GAMEROOM* room)
{
	return room->fud;
}

void gameroom_set_fud(GAMEROOM* room, void* fud)
{
	room->fud = fud;
}

const GAMEROOM_CLASS* gameroom_get_class(GAMEROOM* room)
{
	return room->gc;
}

int gameroom_find(const char* strid, OBJECT_ID* objid)
{
	HASHMAP_ITEM* item;
	GAMEROOM* room;

	os_mutex_lock(&room_strid_mtx);
	item = hashmap_get(room_strid_map, strid, (unsigned int)strlen(strid));
	if(item!=NULL) {
		room = HASHMAP_ITEM_GETPTR(item);
		memcpy(objid, &room->objid, sizeof(*objid));
	}
	os_mutex_unlock(&room_strid_mtx);

	return item!=NULL?ERR_NOERROR:ERR_NOT_FOUND;
}

const OBJECT_ID* gameroom_objid(GAMEROOM* room, OBJECT_ID* objid)
{
	if(objid!=NULL) {
		memcpy(objid, &room->objid, sizeof(*objid));
	}
	return &room->objid;
}

GAMEROOM* gameroom_lock_byindex(int index)
{
	if(index<0 && index>=sizeof(room_list)/sizeof(room_list[0]))
		return NULL;

	os_mutex_lock(&room_list[index].mtx);
	if(room_list[index].room==NULL) {
		os_mutex_unlock(&room_list[index].mtx);
		return NULL;
	}

	return room_list[index].room;
}

GAMEROOM* gameroom_lock(const OBJECT_ID* objid)
{
	if(objid->index<0 && objid->index>=sizeof(room_list)/sizeof(room_list[0]))
		return NULL;

	os_mutex_lock(&room_list[objid->index].mtx);
	if(room_list[objid->index].room==NULL || memcmp(&room_list[objid->index].room->objid, objid, sizeof(OBJECT_ID))!=0) {
		os_mutex_unlock(&room_list[objid->index].mtx);
		return NULL;
	}

	return room_list[objid->index].room;
}

int gameroom_unlock(GAMEROOM* room)
{
	unsigned int index;

	if(room->member_count==0 && (room->gc->flags&GAMEROOM_AUTO_FREE)!=0) {
		gameroom_destroy(room);
	}

	index = room->objid.index;
	if((room_list[index].room->flags&GAMEROOM_DELETE)!=0) {
		gameroom_free(room, room->gc->state_count);
	}

	os_mutex_unlock(&room_list[index].mtx);
	return ERR_NOERROR;
}

// time
int gameroom_timer_reset(GAMEROOM* room, unsigned int id, unsigned int state_id)
{
	int i;
	for(i=0; i<sizeof(room->timer_list)/sizeof(room->timer_list[0]); i++) {
		if(room->timer_list[i].timer_id==id && room->timer_list[i].state_id==state_id) {
			room->timer_list[i].timer_id = 0;
			calc_duetime(room);
			return ERR_NOERROR;
		}
	}
	return ERR_NOT_FOUND;
}

int gameroom_timer_set(GAMEROOM* room, unsigned int id, unsigned int state_id, unsigned int duetime, unsigned int period)
{
	unsigned int index, i;

	assert(state_id<GAMEROOM_STATE_MAX);
	assert(room->gc->state_map[state_id]!=NULL);
	if(state_id>=GAMEROOM_STATE_MAX)		return ERR_INVALID_PARAMETER;
	if(room->gc->state_map[state_id]==NULL)	return ERR_INVALID_PARAMETER;

	assert(duetime!=0 || period!=0);
	if(duetime==0 && period==0) return ERR_INVALID_PARAMETER;

	index = 0xffff;
	for(i=0; i<sizeof(room->timer_list)/sizeof(room->timer_list[0]); i++) {
		if(room->timer_list[i].timer_id==0) {
			if(index==-1) index = i;
			continue;
		}
		if(room->timer_list[i].timer_id==id && room->timer_list[i].state_id==state_id) {
			if(room->timer_list[index].period!=0 || period!=0) {
				return ERR_EXISTED;
			} else {
				index = i;
				break;
			}
		}
	}
	if(index==0xffff) return ERR_FULL;

	if(duetime==0) duetime = period;
	os_time_get(&room->timer_list[index].duetime);

	room->timer_list[index].timer_id = id;
	room->timer_list[index].state_id = state_id;
	room->timer_list[index].duetime += duetime;
	room->timer_list[index].period = period;

	calc_duetime(room);
	return ERR_NOERROR;
}

//
void* gameroom_get_statedata(GAMEROOM* room, unsigned int state_id)
{
	assert(state_id<GAMEROOM_STATE_MAX);
	assert(room->gc->state_map[state_id]!=NULL);
	if(state_id>=GAMEROOM_STATE_MAX)		return NULL;
	if(room->gc->state_map[state_id]==NULL)	return NULL;

	return (void*)((unsigned char*)(room+1) + room->gc->state_map[state_id]->state_offset);
}

void* gameroom_get_membdata(GAMEROOM* room, unsigned int state_id, GAMEROOM_MEMBER* memb)
{
	assert(state_id<GAMEROOM_STATE_MAX);
	assert(room->gc->state_map[state_id]!=NULL);
	if(state_id>=GAMEROOM_STATE_MAX)		return NULL;
	if(room->gc->state_map[state_id]==NULL)	return NULL;

	return (void*)((unsigned char*)(memb+1) + room->gc->state_map[state_id]->member_offset);
}

GAMEROOM_MEMBER* gameroom_member_create(GAMEROOM* room, const char* strid, const OBJECT_ID* oid)
{
	unsigned int index, i;

	for(index=0; index<room->gc->member_max; index++) {
		if(room->members[index]==NULL) break;
	}
	if(index==room->gc->member_max) return NULL;

	room->members[index] = (GAMEROOM_MEMBER*)mempool_alloc(room->gc->memb_pool);
	if(room->members[index]==NULL) return NULL;

	room->members[index]->seq = (((member_seq++) & 0xffff) << 16) | ((unsigned int)index);
	room->members[index]->gm = room;
	room->members[index]->state_flags = 0;
	room->members[index]->mask = 0;
	memcpy(&room->members[index]->objid, oid, sizeof(room->members[index]->objid));
	room->members[index]->group = rpcnet_getgroup(&oid->address);

	for(i=0; i<room->gc->state_count; i++) {
		if((room->gc->states[i].flags&GAMEROOM_AUTO_ATTACH)==0) continue;
		room->members[index]->state_flags |= (1<<i);
		room->gc->states[i].OnNotify(room, GAMEROOM_NOTIFY_ATTACH, room->gc->states[i].id, room->members[index]);
	}

	room->member_count++;
	return room->members[index];
}

int gameroom_member_release(GAMEROOM* room, GAMEROOM_MEMBER* member)
{
	if(member->state_flags!=0) return ERR_UNKNOWN;
	room->gc->RpcLeave(room, member);
	return gameroom_member_delete(room, member);
}

int gameroom_member_delete(GAMEROOM* room, GAMEROOM_MEMBER* member)
{
	unsigned int i, index;

	index = (member->seq&0xffff);
	assert(index<room->gc->member_max && room->members[index]==member);
	if(index>=room->gc->member_max || room->members[index]!=member) return ERR_INVALID_PARAMETER;

	if(member->state_flags!=0) {
		for(i=0; i<room->gc->state_count; i++) {
			if((member->state_flags&(1<<i))==0) continue;

			room->gc->states[i].OnNotify(room, GAMEROOM_NOTIFY_DETACH, room->gc->states[i].id, member);
		}
	}

	mempool_free(room->gc->memb_pool, room->members[index]);
	room->member_count--;
	room->members[index] = NULL;
	return ERR_NOERROR;
}

const char* gameroom_member_get_strid(GAMEROOM_MEMBER* member)
{
	return member->strid;
}

int gameroom_member_set_strid(GAMEROOM* room, GAMEROOM_MEMBER* member, const char* strid)
{
	unsigned int i;

	if(strcmp(strid, member->strid)==0) return ERR_NOERROR;

	for(i=0; i<room->member_count; i++) {
		if(room->members[i]==NULL) continue;
		if(strcmp(room->members[i]->strid, strid)==0) return ERR_EXISTED;
	}

	strcpy(member->strid, strid);
	return ERR_NOERROR;
}

GAMEROOM_MEMBER* gameroom_member_getbystrid(GAMEROOM* room, const char* strid)
{
	unsigned int i;

	for(i=0; i<room->member_count; i++) {
		if(room->members[i]==NULL) continue;
		if(strcmp(room->members[i]->strid, strid)==0) return room->members[i];
	}

	return NULL;
}

int gameroom_member_connect(GAMEROOM* room, GAMEROOM_MEMBER* member, const OBJECT_ID* oid)
{
	assert(member->group==NULL);
	if(member->group!=NULL) return ERR_ALREADY;

	member->group = rpcnet_getgroup(&oid->address);
	assert(member->group!=NULL);
	if(member->group==NULL) return ERR_INVALID_PARAMETER;
	memcpy(&member->objid, oid, sizeof(member->objid));

	gameroom_notify(room, GAMEROOM_NOTIFY_CONNECT, member);

	return ERR_NOERROR;
}

int gameroom_member_disconnect(GAMEROOM* room, GAMEROOM_MEMBER* member)
{
	assert(member->group!=NULL);
	if(member->group==NULL) return ERR_ALREADY;

	gameroom_notify(room, GAMEROOM_NOTIFY_DISCONNECT, member);

	member->group = NULL;
	memset(&member->objid, 0, sizeof(member->objid));

	return ERR_NOERROR;
}

int gameroom_member_isonline(GAMEROOM_MEMBER* member)
{
	return member->group!=NULL;
}

void gameroom_notify(GAMEROOM* room, int code, GAMEROOM_MEMBER* memb)
{
	unsigned int i;
	for(i=0; i<room->gc->state_count; i++) {
		if(room->gc->states[i].OnNotify==NULL) continue;

		room->gc->states[i].OnNotify(room, code, room->gc->states[i].id, memb);
	}
}

void gameroom_notify_state(GAMEROOM* room, int code, unsigned int state_id, GAMEROOM_MEMBER* memb)
{
	assert(state_id<GAMEROOM_STATE_MAX);
	assert(room->gc->state_map[state_id]!=NULL);
	if(state_id>=GAMEROOM_STATE_MAX)		return;
	if(room->gc->state_map[state_id]==NULL)	return;

	room->gc->state_map[state_id]->OnNotify(room, code, state_id, memb);
}

unsigned int gameroom_member_getseq(GAMEROOM* room, GAMEROOM_MEMBER* memb)
{
	return memb->seq;
}

int gameroom_member_getindex(GAMEROOM* room, GAMEROOM_MEMBER* memb)
{
	return (int)(memb->seq&0xffff);
}

GAMEROOM_MEMBER* gameroom_member_getbyseq(GAMEROOM* room, unsigned int seq)
{
	unsigned int index;
	index = (seq & 0xffff);
	if(index<0 || index>=room->gc->member_max) return NULL;
	if(room->members[index]==NULL || room->members[index]->seq!=seq) return NULL;
	return room->members[index];
}

GAMEROOM_MEMBER* gameroom_member_getbyindex(GAMEROOM* room, unsigned int index)
{
	if(index<0 || index>=room->gc->member_max) return NULL;

	return room->members[index];
}

void gameroom_member_attach(GAMEROOM* room, unsigned int state_id, GAMEROOM_MEMBER* memb)
{
	assert(state_id<GAMEROOM_STATE_MAX);
	assert(room->gc->state_map[state_id]!=NULL);
	if(state_id>=GAMEROOM_STATE_MAX)		return;
	if(room->gc->state_map[state_id]==NULL)	return;

	assert((memb->state_flags&(1<<(room->gc->state_map[state_id]->index)))==0);
	if((memb->state_flags&(1<<(room->gc->state_map[state_id]->index)))!=0) return;

	memb->state_flags |= (1<<(room->gc->state_map[state_id]->index));
	room->gc->states[state_id].OnNotify(room, GAMEROOM_NOTIFY_ATTACH, state_id, memb);
}

void gameroom_member_detach(GAMEROOM* room, unsigned int state_id, GAMEROOM_MEMBER* memb)
{
	assert(state_id<GAMEROOM_STATE_MAX);
	assert(room->gc->state_map[state_id]!=NULL);
	if(state_id>=GAMEROOM_STATE_MAX)		return;
	if(room->gc->state_map[state_id]==NULL)	return;

	if((memb->state_flags&(1<<(room->gc->state_map[state_id]->index)))==0) {
		assert(0);
		return;
	}

	memb->state_flags &= ~(1<<((room->gc->state_map[state_id]->index)));
	room->gc->state_map[state_id]->OnNotify(room, GAMEROOM_NOTIFY_DETACH, state_id, memb);
}

const OBJECT_ID* gameroom_member_get_objid(GAMEROOM_MEMBER* memb, OBJECT_ID* objid)
{
	if(objid!=NULL) {
		memcpy(objid, &memb->objid, sizeof(*objid));
	}
	return &memb->objid;
}

RPCNET_GROUP* gameroom_member_get_group(GAMEROOM_MEMBER* memb)
{
	return memb->group;
}

void gameroom_member_set_mask(GAMEROOM* room, GAMEROOM_MEMBER* memb, unsigned int mask)
{
	memb->mask = mask;
}

unsigned int gameroom_member_get_mask(GAMEROOM* room, GAMEROOM_MEMBER* memb)
{
	return memb->mask;
}

int gameroom_sendex(GAMEROOM* room, GAMEROOM_MEMBER* memb, unsigned int mask, unsigned int value, unsigned int flags, unsigned char* data, unsigned int len)
{
	unsigned int i;

	for(i=0; i<room->gc->member_max; i++) {
		if(room->members[i]==NULL) continue;
		if(room->members[i]->state_flags==0) continue;
		if((flags&GAMEROOM_EXCEPT)!=0 && room->members[i]==memb) continue;
		if(!((flags&GAMEROOM_SELF)!=0 && room->members[i]==memb)) {
			if(!((flags&GAMEROOM_MATCH)!=0 && (room->members[i]->mask&mask)==value)) {
				if(!((flags&GAMEROOM_NOTMATCH)!=0 && (room->members[i]->mask&mask)!=value)) {
					continue;
				}
			}
		}

		room->gc->RpcSend(room, memb, data, len);
	}

	return ERR_NOERROR;
}

static void timer_proc(TIMER handle, void* key)
{
	os_time_t curtime;
	int index, i;
	GAMEROOM* room;

	os_time_get(&curtime);

	for(index=0; index<sizeof(room_list)/sizeof(room_list[0]); index++) {
		if(room_list[index].duetime==0 || room_list[index].duetime>curtime) continue;

		room = gameroom_lock_byindex(index);
		if(room==NULL) continue;

		for(i=0; i<sizeof(room->timer_list)/sizeof(room->timer_list[0]); i++) {
			if(room->timer_list[i].timer_id==0) continue;
			if(room->timer_list[i].duetime>curtime) continue;

			room->gc->states[room->timer_list[i].state_id].OnTimer(room, room->timer_list[i].state_id, room->timer_list[i].timer_id);

			if(room->timer_list[i].period==0) {
				room->timer_list[i].timer_id = 0;
			} else {
				room->timer_list[i].duetime += room->timer_list[i].period;
			}
		}

		calc_duetime(room);
		gameroom_unlock(room);
	}
}

void calc_duetime(GAMEROOM* room)
{
	int i;
	os_time_t duetime = 0;
	for(i=0; i<sizeof(room->timer_list)/sizeof(room->timer_list[0]); i++) {
		if(room->timer_list[i].timer_id==0) continue;
		if(duetime==0 && duetime>=room->timer_list[i].duetime) continue;
		duetime = room->timer_list[i].duetime;
	}
	room_list[room->objid.index].duetime = duetime;
}
