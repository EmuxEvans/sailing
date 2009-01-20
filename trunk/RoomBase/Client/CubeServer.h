#pragma once

extern MEMPOOL_HANDLE	user_pool;
extern MEMPOOL_HANDLE	room_pool;
extern MEMPOOL_HANDLE	memb_pool;

int cubeserver_init();
int cubeserver_final();
