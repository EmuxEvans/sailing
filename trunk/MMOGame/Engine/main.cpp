#include <assert.h>
#include <list>

#include "Game.h"
#include "Math.h"
#include "VarCmd.h"

#include "GameArea.h"
#include "GameArea.inl"
#include "GameBuff.h"
#include "GameItem.h"
#include "GameSkill.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"

CSGArea g_Area;
CSGPlayer g_Player1(1);
CSGPlayer g_Player2(2);

int main(int argc, char* argv[])
{
	Vector p1 = { 10.0f, 10.0f, 0.0f };
	Vector p2 = { 20.0f, 20.0f, 0.0f };

	g_Player1.SetArea(&g_Area);
	g_Player2.SetArea(&g_Area);

	g_Player1.SetPosition(p1, 0.0f);
	g_Player2.SetPosition(p2, 0.0f);

	g_Player1.Move(NULL, &p2, 10000);
	g_Player2.Move(NULL, &p1, 10000);


	unsigned int nTime = 0;
	for(int i=0; i<1000; i++) {
		g_Area.Tick(nTime, 100);
		nTime += 100;
	}

	return 0;
}
