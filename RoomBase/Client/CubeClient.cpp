#include <assert.h>

#include <skates/skates.h>

#include "../common/GameUser.h"
#include "../common/GameRoom.h"
#include "../common/GameUser.inl"
#include "../common/GameRoom.inl"

#include "../Cube.proto.h"
#include "../Cube.proto.h"
#include "../Cube.net.proto.h"
#include "../Cube.net.clt.hpp"

#include "CubeClient.h"

CCubeClient::CCubeClient() : m_Login(this, TCP_TEXTMODE)
{
}

CCubeClient::~CCubeClient()
{
}
