#include <string.h>
#include <assert.h>

#include <skates/skates.h>

#include "GameRoom.h"
#include "GameRoom.inl"
#include "CubeRoom.proto.h"
#include "GameUser.h"

CCubeUser User(NULL);
CCubeRoom Room(NULL);
CCubeMember Member(NULL, NULL, 0);
