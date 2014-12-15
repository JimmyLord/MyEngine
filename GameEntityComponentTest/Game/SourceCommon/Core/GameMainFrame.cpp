//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

#include "../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "GameMainFrame.h"

enum GameMenuIDs
{
    myIDGame_LoadScene = myID_NumIDs,
    myIDGame_SaveScene,
};

GameMainFrame* g_pGameMainFrame = 0;

GameMainFrame::GameMainFrame()
: MainFrame(0)
{
    g_pGameMainFrame = this;

    m_File->Insert( 0, myIDGame_LoadScene, wxT("&Load Scene") );
    m_File->Insert( 1, myIDGame_SaveScene, wxT("&Save Scene") );

    Connect( myIDGame_LoadScene, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveScene, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
}

void GameMainFrame::AddPanes()
{
    MainFrame::AddPanes();

    // create the editor opengl canvas
    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    m_pGLCanvasEditor = MyNew MainGLCanvas( (wxFrame*)this, args, 1, false );
    //m_pGLCanvasEditor->SetSize( 600, 600 );

    m_AUIManager.AddPane( m_pGLCanvasEditor, wxAuiPaneInfo().Name("GLCanvasEditor").Bottom().Caption("GLCanvasEditor") );//.CaptionVisible(false) );
}

void GameMainFrame::OnGameMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myIDGame_SaveScene:
        ((GameEntityComponentTest*)g_pGameCore)->SaveScene();
        break;

    case myIDGame_LoadScene:
        ((GameEntityComponentTest*)g_pGameCore)->LoadScene();
        ResizeViewport();
        break;
    }
}

void GameMainFrame::ResizeViewport()
{
    MainFrame::ResizeViewport();

    m_pGLCanvasEditor->ResizeViewport();
}
