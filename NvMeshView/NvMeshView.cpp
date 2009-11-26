// test01.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Resource.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <io.h>
#include <string.h>
#include <assert.h>
#include <windows.h>
#include <commdlg.h>
#include <gl\glew.h>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIControls.h"
#include "XUIDevice.h"
#include "XUIApp.h"

#include <vector>
#include "MeshLoaderObj.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourStatNavMesh.h"
#include "DetourStatNavMeshBuilder.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return XUI_AppMain();
}

class NvMeshExport {
public:
	NvMeshExport() {
		m_triflags = NULL;
		m_solid = NULL;
		m_chf = NULL;
		m_cset = NULL;
		m_pmesh = NULL;
		m_dmesh = NULL;

		m_keepInterResults = true;
	}

	~NvMeshExport() {
	}

	void KeepInterResults(bool bKeep) {
		m_keepInterResults = bKeep;
	}

	void Clear() {
		delete [] m_triflags;
		m_triflags = NULL;
		delete m_solid;
		m_solid = NULL;
		delete m_chf;
		m_chf = NULL;
		delete m_cset;
		m_cset = NULL;
		delete m_pmesh;
		m_pmesh = NULL;
		delete m_dmesh;
		m_dmesh = NULL;
	}

	static const int NAVMESH_POLYS = 0x01;
	static const int NAVMESH_BVTREE = 0x02;
	static const int NAVMESH_TOOLS = 0x04;

	void Build(rcConfig& m_cfg, const float* m_verts, int m_nverts, const int* m_tris, const float* m_trinorms, int m_ntris)
	{
		// step 2:
		m_solid = new rcHeightfield;
		rcCreateHeightfield(*m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch);
		m_triflags = new unsigned char[m_ntris];
		memset(m_triflags, 0, m_ntris*sizeof(unsigned char));
		rcMarkWalkableTriangles(m_cfg.walkableSlopeAngle, m_verts, m_nverts, m_tris, m_ntris, m_triflags);
		rcRasterizeTriangles(m_verts, m_nverts, m_tris, m_triflags, m_ntris, *m_solid);
		if(!m_keepInterResults) {
			delete [] m_triflags;
			m_triflags = 0;
		}

		// step 3:
		rcFilterLedgeSpans(m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
		rcFilterWalkableLowHeightSpans(m_cfg.walkableHeight, *m_solid);
		m_chf = new rcCompactHeightfield;
		rcBuildCompactHeightfield(m_cfg.walkableHeight, m_cfg.walkableClimb, RC_WALKABLE, *m_solid, *m_chf);
		if (!m_keepInterResults) {
			delete m_solid;
			m_solid = 0;
		}

		// step 4:
		rcBuildDistanceField(*m_chf);
		rcBuildRegions(*m_chf, m_cfg.walkableRadius, m_cfg.borderSize, m_cfg.minRegionSize, m_cfg.mergeRegionSize);
		
		// Step 5. Trace and simplify region contours.
		m_cset = new rcContourSet;
		rcBuildContours(*m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset);

		// Step 6. Build polygons mesh from contours.
		m_pmesh = new rcPolyMesh;
		rcBuildPolyMesh(*m_cset, m_cfg.maxVertsPerPoly, *m_pmesh);
		
		// Step 7. Create detail mesh which allows to access approximate height on each polygon.
		m_dmesh = new rcPolyMeshDetail;
		if(!rcBuildPolyMeshDetail(*m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
		if(!m_keepInterResults) {
			delete m_chf;
			m_chf = 0;
			delete m_cset;
			m_cset = 0;
		}
	}

	bool MakeData(rcConfig& m_cfg, const char* pOutFile) {
		unsigned char* navData = 0;
		int navDataSize = 0;
		if (!dtCreateNavMeshData(m_pmesh->verts, m_pmesh->nverts,
								 m_pmesh->polys, m_pmesh->npolys, m_pmesh->nvp,
								 m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch,
								 m_dmesh->meshes, m_dmesh->verts, m_dmesh->nverts, m_dmesh->tris, m_dmesh->ntris, 
								 &navData, &navDataSize))
		{
			return false;
		}

		FILE* fp = fopen(pOutFile, "wb");
		if(!fp) {
			delete [] navData;
			return false;
		}
		fwrite(navData, navDataSize, 1, fp);
		fclose(fp);
		delete [] navData;
		return true;
	}

	unsigned char* m_triflags;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcPolyMeshDetail* m_dmesh;
	bool m_keepInterResults;
};

#define MAX_POLYS 256

class XUIApp {
public:
	XUIApp() {
		m_bMesh = false;
		m_bBuild = false;
		m_navMesh = NULL;
	}

	void AppInit();
	void AppFinal();
	void DoTick(unsigned int nDeltaTime);
	void DrawScene();

	bool m_bMesh;
	MeshLoaderObj m_Mesh;
	float meshBMin[3], meshBMax[3];
	float camr, camx, camy, camz, rx, ry;
	bool m_bBuild;
	rcConfig m_Config;
	NvMeshExport m_Export;
	dtStatNavMesh* m_navMesh;

	dtStatPolyRef m_startRef;
	dtStatPolyRef m_endRef;
	dtStatPolyRef m_polys[MAX_POLYS];
	dtStatPolyRef m_parent[MAX_POLYS];
	int m_npolys;
	float m_straightPath[MAX_POLYS*3];
	int m_nstraightPath;
	float m_polyPickExt[3];

public:
	void OnWelcome_Close(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_pWelcome->SetVisable(!m_pWelcome->IsVisable());
	}

	void OnFileOpen_Select(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_pFileList->SetVisable(!m_pFileList->IsVisable());
		if(m_pFileList->IsVisable()) {
			m_pFileList->BringToTop();
			scanDirectory("meshes", ".obj");
		}
	}

	void OnFileListClick(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_pFileList->SetVisable(false);
		m_pFileName->SetText(dynamic_cast<XUIButton*>(pWidget)->GetText());
	}

	void OnFileOpen_Open(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_nPointCount = 0;
		m_Export.Clear();
		delete m_navMesh;
		m_navMesh = NULL;

		char szFileName[300];
		sprintf(szFileName, "meshes/%s", m_pFileName->GetText());
		if(dynamic_cast<XUICheckBox*>(m_pFileOpen->GetWidget("LARGE"))->GetCheck()) {
			m_Mesh.setScale(1/1000.0f);
		} else {
			m_Mesh.setScale(1.0f);
		}
		if(!m_Mesh.load(szFileName)) {
			m_bMesh = false;
			MessageBox(NULL, "", "ERROR", MB_OK);
			return;
		}
		m_bMesh = true;

		rcCalcBounds(m_Mesh.getVerts(), m_Mesh.getVertCount(), meshBMin, meshBMax);
		camr = sqrtf(rcSqr(meshBMax[0]-meshBMin[0]) +
		rcSqr(meshBMax[1]-meshBMin[1]) +
		rcSqr(meshBMax[2]-meshBMin[2])) / 2;
		camx = (meshBMax[0] + meshBMin[0]) / 2 + camr;
		camy = (meshBMax[1] + meshBMin[1]) / 2 + camr;
		camz = (meshBMax[2] + meshBMin[2]) / 2 + camr;
		camr *= 3;
		rx = 45;
		ry = -45;
		glFogf(GL_FOG_START, camr*0.2f);
		glFogf(GL_FOG_END, camr*1.25f);

		if(!m_pBuildTool->IsVisable()) {
			m_pBuildTool->SetVisable(true);
			m_pFileOpen->SetWidgetSize(m_pFileOpen->GetWidgetWidth(), m_pFileOpen->GetWidgetHeight()+260);
			m_pFileOpen->GetWidget("BUILD")->SetVisable(true);
			m_pFileOpen->GetWidget("SAVE")->SetVisable(true);
			m_pFileOpen->GetWidget("TOOL")->SetVisable(true);
			m_pFileOpen->GetWidget("LOG")->SetVisable(true);
		}

		OnFileOpen_ToolBox(NULL, XUIPoint(0, 0), 0);
	}

	void OnFileOpen_ToolBox(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		if(m_bMesh) {
			m_pToolBox->SetVisable(dynamic_cast<XUICheckBox*>(m_pFileOpen->GetWidget("TOOL"))->GetCheck());
			m_pLogView->SetVisable(dynamic_cast<XUICheckBox*>(m_pFileOpen->GetWidget("LOG"))->GetCheck());
		} else {
			m_pToolBox->SetVisable(false);
			m_pLogView->SetVisable(false);
		}
	}

	void OnScene_Click(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId);

	float GetValue(const char* pName) {
		return ((XUISlider*)(m_pBuildTool->GetWidget(pName)))->GetValue();
	}
	void SetValue(const char* pName, float fValue) {
		((XUISlider*)(m_pBuildTool->GetWidget(pName)))->SetValue(fValue);
	}

	void OnFileOpen_Build(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		memset(&m_Config, 0, sizeof(m_Config));
		m_Config.cs = GetValue("CELLSIZE");
		m_Config.ch = GetValue("CELLHEIGHT");
		m_Config.walkableSlopeAngle	= GetValue("SLOPE");
		m_Config.walkableHeight		= ceilf(GetValue("HEIGHT")/m_Config.ch);
		m_Config.walkableClimb		= ceilf(GetValue("CLIMB")/m_Config.ch);
		m_Config.walkableRadius		= ceilf(GetValue("RADIUS")/m_Config.cs);
		m_Config.maxEdgeLen = (int)(GetValue("MAXEDGELENGTH")/ m_Config.cs);
		m_Config.maxSimplificationError = GetValue("MAXEDGEERROR");
		m_Config.minRegionSize = (int)rcSqr(GetValue("MAXREGIONSIZE"));
		m_Config.mergeRegionSize = (int)rcSqr(GetValue("MERGEDREGIONSIZE"));
		m_Config.maxVertsPerPoly = (int)GetValue("VERTSPERPOLY");
		float m_detailSampleDist = GetValue("SAMPLEDISTANCE");
		m_Config.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_Config.cs * m_detailSampleDist;
		m_Config.detailSampleMaxError = m_Config.ch * GetValue("MAXSAMPLEERROR");
		rcCalcBounds(m_Mesh.getVerts(), m_Mesh.getVertCount(), m_Config.bmin, m_Config.bmax);
		rcCalcGridSize(m_Config.bmin, m_Config.bmax, m_Config.cs, &m_Config.width, &m_Config.height);
		m_Export.Build(m_Config, m_Mesh.getVerts(), m_Mesh.getVertCount(), m_Mesh.getTris(), m_Mesh.getNormals(), m_Mesh.getTriangleCount());
		m_bBuild = true;

		m_Export.MakeData(m_Config, "out.tmp");

		FILE* fp = fopen("out.tmp", "rb");
		fseek(fp, 0, SEEK_END);
		int navDataSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		unsigned char* navData = new unsigned char[navDataSize];
		fread(navData, navDataSize, 1, fp);
		fclose(fp);

		m_navMesh = new dtStatNavMesh;
		m_navMesh->init(navData, navDataSize, true);
		ComputPath();
		return;

	}

	void OnToolBox_ViewMode(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		for(m_nViewMode=0; m_nViewMode<sizeof(m_ViewModeWidget)/sizeof(m_ViewModeWidget[0]); m_nViewMode++) {
			if(m_ViewModeWidget[m_nViewMode]==pWidget) break;
		}
		if(m_nViewMode==sizeof(m_ViewModeWidget)/sizeof(m_ViewModeWidget[0])) return;
		for(int i=0; i<sizeof(m_ViewModeWidget)/sizeof(m_ViewModeWidget[0]); i++) {
			if(m_ViewModeWidget[i]==NULL) continue;
			m_ViewModeWidget[i]->SetCheck(i==m_nViewMode);
		}
	}

	void OnWidgetMove(XUIWidget* pWidget, int nLeft, int nTop)
	{
		m_pFileList->SetWidgetPosition(m_pFileOpen->GetWidgetLeft()+m_nFileListOffsetX, m_pFileOpen->GetWidgetTop()+m_nFileListOffsetY);
		m_pFileList->BringToTop();
	}

	void scanDirectory(const char* pPath, const char* ext) {
		clearFiles();

		_finddata_t dir;
		char pathWithExt[MAX_PATH];
		long fh;
		strcpy(pathWithExt, pPath);
		strcat(pathWithExt, "/*");
		strcat(pathWithExt, ext);
		fh = _findfirst(pathWithExt, &dir);
		if (fh == -1L) return;
		do {
			eventMouseButtonClickImpl<XUIApp>* pEvent = new eventMouseButtonClickImpl<XUIApp>();
			XUIButton* pButton = new XUIButton("", dir.name, 0, 0, 100, 15);
			m_pFileList->AddWidget(pButton);
			pButton->_eventMouseButtonClick.Register(pEvent->R(this, &XUIApp::OnFileListClick));
			events.push_back(new eventMouseButtonClickImpl<XUIApp>);
		} while (_findnext(fh, &dir) == 0);
		_findclose(fh);
	}

	void clearFiles() {
		for(size_t i=0; i<events.size(); i++) {
			delete events[i];
		}
		m_pFileList->ClearWidgets();
		events.clear();
	}

	void ComputPath()
	{
		m_npolys = 0;
		m_nstraightPath = 0;
		if(m_nPointCount<2 || !m_navMesh) return;

		m_startRef	= m_navMesh->findNearestPoly(&m_Points[0].x, m_polyPickExt);
		m_endRef	= m_navMesh->findNearestPoly(&m_Points[1].x, m_polyPickExt);
		if(!m_startRef || !m_endRef) return;

		m_npolys = m_navMesh->findPath(m_startRef, m_endRef, &m_Points[0].x, &m_Points[1].x, m_polys, MAX_POLYS);
		if(m_npolys) {
			m_nstraightPath = m_navMesh->findStraightPath(&m_Points[0].x, &m_Points[1].x, m_polys, m_npolys, m_straightPath, MAX_POLYS);
		}
	}

private:
	XUIDialog* m_pWelcome;
	XUIDialog* m_pFileOpen;
	XUIScrollPanel* m_pBuildTool;
	XUILabel* m_pFileName;
	XUIScrollPanel* m_pFileList;
	XUIDialog* m_pToolBox;
	XUIDialog* m_pLogView;

	int m_nFileListOffsetX, m_nFileListOffsetY;
	eventMouseButtonClickImpl<XUIApp> m_eventWelcome_Close;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Select;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Open;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Welcome;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Build;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_ToolBox;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_LogView;
	eventWidgetMoveImpl<XUIApp> m_eventFileListMove;

	GLdouble proj[16];
	GLdouble model[16];
	GLint view[4];
	eventMouseButtonClickImpl<XUIApp> m_eventSceneClick;
	int m_nPointCount;
	struct {
		float x, y, z;
	} m_Points[2];

	int									m_nViewMode;
	XUICheckBox*						m_ViewModeWidget[6];
	eventMouseButtonClickImpl<XUIApp>	m_eventViewMode[6];

	std::vector<eventMouseButtonClickImpl<XUIApp>*> events;
};

void XUIApp::AppInit()
{
	m_pWelcome = new XUIDialog("WELCOME", "Welcome", 0, 0, 200, 100);
	XUI_GetXUI().GetRoot()->AddChild(m_pWelcome);
	m_pWelcome->CenterWidget();
	m_pWelcome->AddChild(new XUILabel("", "I'm Mr.XUI. Who are you?", XUIALIGN_CENTER, 0, 0, m_pWelcome->GetClientWidth(), 20));
	m_pWelcome->AddChild(new XUIButton("CLOSE", "Close", 0, 35, m_pWelcome->GetClientWidth(), 20));
	m_pWelcome->GetWidget("CLOSE")->_eventMouseButtonClick.Register(m_eventWelcome_Close.R(this, &XUIApp::OnWelcome_Close));

	m_pFileOpen = new XUIDialog("", "Main Tool", 10, 10, 200, 101);
	m_pFileName = new XUILabel("FILE", "", XUIALIGN_CENTER, 0, 0, m_pFileOpen->GetClientWidth()-20, 15);
	m_pFileOpen->AddChild(m_pFileName);
	m_pFileOpen->AddChild(new XUIButton("SELECT", "...", m_pFileOpen->GetClientWidth()-20, 0, 20, 15));
	m_pFileOpen->AddChild(new XUICheckBox("LARGE", "Large Mode", false, 0, 20, m_pFileOpen->GetClientWidth(), 20));
	m_pFileOpen->AddChild(new XUIButton("OPEN",		"Open", 0, 45, m_pFileOpen->GetClientWidth()/2-3, 20));
	m_pFileOpen->AddChild(new XUIButton("SHOW",		"Show", m_pFileOpen->GetClientWidth()/2+3, 45, m_pFileOpen->GetClientWidth()/2-3, 20));
	m_pFileOpen->GetWidget("SELECT")->_eventMouseButtonClick.Register(m_eventFileOpen_Select.R(this, &XUIApp::OnFileOpen_Select));
	m_pFileOpen->GetWidget("OPEN")->_eventMouseButtonClick.Register(m_eventFileOpen_Open.R(this, &XUIApp::OnFileOpen_Open));
	m_pFileOpen->GetWidget("SHOW")->_eventMouseButtonClick.Register(m_eventFileOpen_Welcome.R(this, &XUIApp::OnWelcome_Close));
	XUI_GetXUI().GetRoot()->AddChild(m_pFileOpen);

	m_pFileList = new XUIScrollPanel("", 0, 0, 200, 400);
	m_pFileList->m_bShowBoard = true;
	m_pFileList->SetVisable(false);
	XUI_GetXUI().GetRoot()->AddChild(m_pFileList);
	m_pFileList->SetClientArea(
		m_pFileList->GetClientLeft()+5,
		m_pFileList->GetClientTop()+5,
		m_pFileList->GetClientRight()+5,
		m_pFileList->GetClientBottom()+5);

	XUIWidget* pSelect = m_pFileOpen->GetWidget("SELECT");
	XUIPoint P(pSelect->GetWidgetWidth() + 10, -5);
	pSelect->WidgetToScreen(P, P);
	m_pFileList->GetParent()->ScreenToWidget(P, P);
	m_pFileList->SetWidgetPosition(P.x, P.y);
	m_nFileListOffsetX = P.x - m_pFileOpen->GetWidgetLeft();
	m_nFileListOffsetY = P.y - m_pFileOpen->GetWidgetTop();
	m_pFileOpen->_eventWidgetMove.Register(m_eventFileListMove.R(this, &XUIApp::OnWidgetMove));

	m_pBuildTool = new XUIScrollPanel("", 0, 75, m_pFileOpen->GetClientWidth(), 200);
	m_pBuildTool->AddWidget(new XUILabel("", "Rasterization", XUIALIGN_LEFT, 0, 0, 0, 20));
	m_pBuildTool->AddWidget(new XUISlider("CELLSIZE",			"Cell Size",			0, 0, 0, 20, 0.1f, 1, 0.01f));
	m_pBuildTool->AddWidget(new XUISlider("CELLHEIGHT",			"Cell Height",			0, 0, 0, 20, 0.1f, 1, 0.01f));
	m_pBuildTool->AddWidget(new XUILabel("", "Agent", XUIALIGN_LEFT, 0, 0, 0, 20));
	m_pBuildTool->AddWidget(new XUISlider("HEIGHT",				"Height",				0, 0, 0, 20, 0.1f, 5, 0.1f));
	m_pBuildTool->AddWidget(new XUISlider("RADIUS",				"Radius",				0, 0, 0, 20, 0.1f, 5, 0.1f));
	m_pBuildTool->AddWidget(new XUISlider("CLIMB",				"Max Climb",			0, 0, 0, 20, 0.1f, 5, 0.1f));
	m_pBuildTool->AddWidget(new XUISlider("SLOPE",				"Max Slope",			0, 0, 0, 20, 0, 90, 1));
	m_pBuildTool->AddWidget(new XUILabel("", "Region", XUIALIGN_LEFT, 0, 0, 0, 20));
	m_pBuildTool->AddWidget(new XUISlider("MAXREGIONSIZE",		"Max Region Size",		0, 0, 0, 20, 0, 150, 1));
	m_pBuildTool->AddWidget(new XUISlider("MERGEDREGIONSIZE",	"Merged Region Size",	0, 0, 0, 20, 0, 150, 1));
	m_pBuildTool->AddWidget(new XUILabel("", "Polygonization", XUIALIGN_LEFT, 0, 0, 0, 20));
	m_pBuildTool->AddWidget(new XUISlider("MAXEDGELENGTH",		"Max Edge Length",		0, 0, 0, 20, 0, 150, 1));
	m_pBuildTool->AddWidget(new XUISlider("MAXEDGEERROR",		"Max Edge Error",		0, 0, 0, 20, 0.1f, 3, 0.1f));
	m_pBuildTool->AddWidget(new XUISlider("VERTSPERPOLY",		"Verts Per Poly",		0, 0, 0, 20, 3, 6, 1));
	m_pBuildTool->AddWidget(new XUILabel("", "Detail Mesh", XUIALIGN_LEFT, 0, 0, 0, 20));
	m_pBuildTool->AddWidget(new XUISlider("SAMPLEDISTANCE",		"Sample Distance",		0, 0, 0, 20, 0, 16, 1));
	m_pBuildTool->AddWidget(new XUISlider("MAXSAMPLEERROR",		"Max Sample Error",		0, 0, 0, 20, 0, 16, 1));

	SetValue("CELLSIZE", 0.3f);
	SetValue("CELLHEIGHT", 0.2f);
	SetValue("HEIGHT", 2.0f);
	SetValue("RADIUS", 0.6f);
	SetValue("CLIMB", 5.0f);
	SetValue("SLOPE", 45);
	SetValue("MAXREGIONSIZE", 50);
	SetValue("MERGEDREGIONSIZE", 20);
	SetValue("MAXEDGELENGTH", 12);
	SetValue("MAXEDGEERROR", 1.3f);
	SetValue("VERTSPERPOLY", 6);
	SetValue("SAMPLEDISTANCE", 6);
	SetValue("MAXSAMPLEERROR", 1);

	m_pBuildTool->SetVisable(false);
	m_pFileOpen->AddChild(m_pBuildTool);
	m_pFileOpen->AddChild(new XUIButton("BUILD", "Build", 0, 280, m_pFileOpen->GetClientWidth()/2-3, 20));
	m_pFileOpen->AddChild(new XUIButton("SAVE", "Save", m_pFileOpen->GetClientWidth()/2+3, 280, m_pFileOpen->GetClientWidth()/2-3, 20));
	m_pFileOpen->AddChild(new XUICheckBox("TOOL", "Tool Box", true, 0, 305, m_pFileOpen->GetClientWidth()/2-3, 20));
	m_pFileOpen->AddChild(new XUICheckBox("LOG", "Log View" , true, m_pFileOpen->GetClientWidth()/2+3, 305, m_pFileOpen->GetClientWidth()/2-3, 20));
	m_pFileOpen->GetWidget("BUILD")->SetVisable(false);
	m_pFileOpen->GetWidget("SAVE")->SetVisable(false);
	m_pFileOpen->GetWidget("TOOL")->SetVisable(false);
	m_pFileOpen->GetWidget("LOG")->SetVisable(false);
	m_pFileOpen->GetWidget("BUILD")->_eventMouseButtonClick.Register(m_eventFileOpen_Build.R(this, &XUIApp::OnFileOpen_Build));
	m_pFileOpen->GetWidget("TOOL")->_eventMouseButtonClick.Register(m_eventFileOpen_ToolBox.R(this, &XUIApp::OnFileOpen_ToolBox));
	m_pFileOpen->GetWidget("LOG")->_eventMouseButtonClick.Register(m_eventFileOpen_LogView.R(this, &XUIApp::OnFileOpen_ToolBox));

	m_pToolBox = new XUIDialog("", "Tool Box", 10, 380, 200, 200);
	m_pToolBox->SetVisable(false);
	XUI_GetXUI().GetRoot()->AddChild(m_pToolBox);
	m_pToolBox->AddChild(new XUILabel("", "OOXX Mode", XUIALIGN_LEFT, 0, 0, m_pToolBox->GetWidgetWidth(), 20));
	m_pToolBox->AddChild(new XUICheckBox("", "Path find", true, 0,									20, m_pFileOpen->GetClientWidth()/2-5, 20));
	m_pToolBox->AddChild(new XUICheckBox("", "Raycast",   true, m_pFileOpen->GetClientWidth()/2,	20, m_pFileOpen->GetClientWidth()/2-5, 20));
	m_pToolBox->AddChild(new XUILabel("", "View Mode", XUIALIGN_LEFT, 0, 45, m_pToolBox->GetWidgetWidth(), 20));

	memset(m_ViewModeWidget, 0, sizeof(m_ViewModeWidget));
	m_ViewModeWidget[0] = new XUICheckBox("", "Poly Mesh",        false, 0, 65, m_pFileOpen->GetClientWidth(), 20);
	m_ViewModeWidget[0]->_eventMouseButtonClick.Register(m_eventViewMode[0].R(this, &XUIApp::OnToolBox_ViewMode));
	m_pToolBox->AddChild(m_ViewModeWidget[0]);
	m_ViewModeWidget[1] = new XUICheckBox("", "Poly Mesh Detail", false, 0, 85, m_pFileOpen->GetClientWidth(), 20);
	m_ViewModeWidget[1]->_eventMouseButtonClick.Register(m_eventViewMode[1].R(this, &XUIApp::OnToolBox_ViewMode));
	m_pToolBox->AddChild(m_ViewModeWidget[1]);
	OnToolBox_ViewMode(m_ViewModeWidget[0], XUIPoint(0, 0), 0);

	m_pLogView = new XUIDialog("", "Log view", 220, 380, 500, 200);
	m_pLogView->SetVisable(false);
	XUI_GetXUI().GetRoot()->AddChild(m_pLogView);

	XUI_GetXUI().GetRoot()->_eventMouseButtonClick.Register(m_eventSceneClick.R(this, &XUIApp::OnScene_Click));
}

void XUIApp::AppFinal()
{
}

void XUIApp::DrawScene()
{
	if(!m_bMesh) return;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0f, (float)XUI_GetXUI().GetRoot()->GetWidgetWidth()/(float)XUI_GetXUI().GetRoot()->GetWidgetHeight(), 1.0f, camr);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(rx,1,0,0);
	glRotatef(ry,0,1,0);
	glTranslatef(-camx, -camy, -camz);

	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, view);


	float col[4];
	col[0] = 1; col[1] = 1; col[2] = 1; col[3] = 0.5f;
	rcDebugDrawBoxWire(meshBMin[0], meshBMin[1], meshBMin[2], meshBMax[0], meshBMax[1], meshBMax[2], col);

	// rcDebugDrawMesh(m_Mesh.m_verts, m_Mesh.m_vertCount, m_Mesh.m_tris, m_Mesh.m_normals, m_Mesh.m_triCount, 0);
	rcDebugDrawMeshSlope(m_Mesh.m_verts, m_Mesh.m_vertCount, m_Mesh.m_tris, m_Mesh.m_normals, m_Mesh.m_triCount, 45);

	for(int i=0; i<m_nPointCount; i++) {
		float m_distanceToWall = GetValue("RADIUS");
		float m_agentHeight = GetValue("HEIGHT");
		rcDebugDrawCylinderWire(m_Points[i].x-m_distanceToWall, m_Points[i].y+0.02f, m_Points[i].z-m_distanceToWall,
								m_Points[i].x+m_distanceToWall, m_Points[i].y+m_agentHeight, m_Points[i].z+m_distanceToWall,
								col);
	}

	if(m_bBuild) {
		switch(m_nViewMode) {
		case 0:
			glDepthMask(GL_FALSE);
			rcDebugDrawPolyMesh(*m_Export.m_pmesh);
			glDepthMask(GL_TRUE);
			break;
		case 1:
			glDepthMask(GL_FALSE);
			rcDebugDrawPolyMeshDetail(*m_Export.m_dmesh);
			glDepthMask(GL_TRUE);
			break;
		}
	}

	//dtDebugDrawStatNavMeshPoly(m_navMesh, m_startRef, startCol);
	//dtDebugDrawStatNavMeshPoly(m_navMesh, m_endRef, endCol);
	//	
	//if (m_npolys)
	//{
	//	for (int i = 1; i < m_npolys-1; ++i)
	//		dtDebugDrawStatNavMeshPoly(m_navMesh, m_polys[i], pathCol);
	//}

	if(m_nstraightPath) {
		glColor4ub(64,16,0,220);
		glLineWidth(3.0f);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < m_nstraightPath; ++i)
			glVertex3f(m_straightPath[i*3], m_straightPath[i*3+1]+0.4f, m_straightPath[i*3+2]);
		glEnd();
		glLineWidth(1.0f);
		glPointSize(6.0f);
		glBegin(GL_POINTS);
		for (int i = 0; i < m_nstraightPath; ++i)
			glVertex3f(m_straightPath[i*3], m_straightPath[i*3+1]+0.4f, m_straightPath[i*3+2]);
		glEnd();
		glPointSize(1.0f);
	}
}

void XUIApp::DoTick(unsigned int nDeltaTime)
{
	if(m_bMesh) {
		BYTE KeyState[256];
		memset(KeyState, 0, sizeof(KeyState));
		GetKeyboardState(KeyState);

		float dt = (float)nDeltaTime / 1000.0f;
		float moveW = 0, moveS = 0, moveA = 0, moveD = 0;
		moveW = rcClamp(moveW + dt * 4 * ((KeyState['W']&0x80) ? 1 : -1), 0.0f, 1.0f);
		moveS = rcClamp(moveS + dt * 4 * ((KeyState['S']&0x80) ? 1 : -1), 0.0f, 1.0f);
		moveA = rcClamp(moveA + dt * 4 * ((KeyState['A']&0x80) ? 1 : -1), 0.0f, 1.0f);
		moveD = rcClamp(moveD + dt * 4 * ((KeyState['D']&0x80) ? 1 : -1), 0.0f, 1.0f);

	//	float keybSpeed = 22.0f;
		float keybSpeed = 1.0f;

		if((KeyState[VK_LSHIFT]&0x80) && (KeyState[VK_RSHIFT]&0x80)) keybSpeed *= 4.0f;

		float movex = (moveD - moveA) * keybSpeed * nDeltaTime;
		float movey = (moveS - moveW) * keybSpeed * nDeltaTime;

		camx += movex * (float)model[0];
		camy += movex * (float)model[4];
		camz += movex * (float)model[8];

		camx += movey * (float)model[2];
		camy += movey * (float)model[6];
		camz += movey * (float)model[10];
	}

	XUI_GetXUI().BeginFrame();
	XUI_GetXUI().EndFrame();
}

static bool intersectSegmentTriangle(const float* sp, const float* sq,
							  const float* a, const float* b, const float* c,
							  float &t)
{
	float v, w;
	float ab[3], ac[3], qp[3], ap[3], norm[3], e[3];
	vsub(ab, b, a);
	vsub(ac, c, a);
	vsub(qp, sp, sq);
	
	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	vcross(norm, ab, ac);
	
	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = vdot(qp, norm);
	if (d <= 0.0f) return false;
	
	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	vsub(ap, sp, a);
	t = vdot(ap, norm);
	if (t < 0.0f) return false;
	if (t > d) return false; // For segment; exclude this code line for a ray test
	
	// Compute barycentric coordinate components and test if within bounds
	vcross(e, qp, ap);
	v = vdot(ac, e);
	if (v < 0.0f || v > d) return false;
	w = -vdot(ab, e);
	if (w < 0.0f || v + w > d) return false;
	
	// Segment/ray intersects triangle. Perform delayed division
	t /= d;
	
	return true;
}

static bool raycast(MeshLoaderObj& mesh, float* src, float* dst, float& tmin)
{
	float dir[3];
	vsub(dir, dst, src);
	
	int nt = mesh.getTriCount();
	const float* verts = mesh.getVerts();
	const float* normals = mesh.getNormals();
	const int* tris = mesh.getTris();
	tmin = 1.0f;
	bool hit = false;
	
	for (int i = 0; i < nt*3; i += 3)
	{
		const float* n = &normals[i];
		if (vdot(dir, n) > 0)
			continue;
		
		float t = 1;
		if (intersectSegmentTriangle(src, dst,
									 &verts[tris[i]*3],
									 &verts[tris[i+1]*3],
									 &verts[tris[i+2]*3], t))
		{
			if (t < tmin)
				tmin = t;
			hit = true;
		}
	}
	
	return hit;
}

void XUIApp::OnScene_Click(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
	GLdouble x, y, z;
	float rays[3], raye[3];
	gluUnProject(Point.x, XUI_GetXUI().GetRoot()->GetWidgetHeight()-Point.y, 0.001f, model, proj, view, &x, &y, &z);
	rays[0] = (float)x; rays[1] = (float)y; rays[2] = (float)z;
	gluUnProject(Point.x, XUI_GetXUI().GetRoot()->GetWidgetHeight()-Point.y, 0.999f, model, proj, view, &x, &y, &z);
	raye[0] = (float)x; raye[1] = (float)y; raye[2] = (float)z;

	float t;
	if(raycast(m_Mesh, rays, raye, t)) {
		if(m_nPointCount<sizeof(m_Points)/sizeof(m_Points[0])) {
			m_nPointCount++;
		} else {
			memmove(m_Points+0, m_Points+1, sizeof(m_Points)-sizeof(m_Points[0]));
		}
		m_Points[m_nPointCount-1].x = rays[0] + (raye[0] - rays[0])*t;
		m_Points[m_nPointCount-1].y = rays[1] + (raye[1] - rays[1])*t;
		m_Points[m_nPointCount-1].z = rays[2] + (raye[2] - rays[2])*t;

		ComputPath();
	}
}

static XUIApp _App;

void XUI_AppInit()
{
	_App.AppInit();
}

void XUI_AppFinal()
{
	_App.AppFinal();
}

void XUI_DrawScene()
{
	_App.DrawScene();
}

void XUI_DoTick(unsigned int nDeltaTime)
{
	_App.DoTick(nDeltaTime);
}
