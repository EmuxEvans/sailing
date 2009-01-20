#pragma once

class CCubeUser;
class CCubeRoom;
class CCubeMember;

typedef IGameUserController<CCubeUser> ICubeUserController;
typedef IGameChannel<CCubeUser> ICubeChannel;
typedef IGameRoomController<CCubeUser, CCubeRoom, CCubeMember> ICubeRoomController;

#include "ServerLoginChannel.h"
#include "CubeUser.h"
#include "CubeRoom.h"
