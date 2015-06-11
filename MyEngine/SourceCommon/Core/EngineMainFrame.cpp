//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX

#include "../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "EngineMainFrame.h"

EngineMainFrame* g_pEngineMainFrame = 0;

const char* g_DefaultPerspectiveMenuLabels[Perspective_NumPerspectives] =
{
    "Center &Editor",
    "Center &Game",
    "&Side by Side",
    "&Full Frame Game",
};

const char* g_DefaultPerspectives[Perspective_NumPerspectives] =
{
    "layout2|name=GLCanvas;caption=GLCanvas;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-253;floaty=231;floatw=680;floath=748|name=PanelWatch;caption=Watch;state=2099196;dir=2;layer=3;row=0;pos=0;prop=130560;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099196;dir=2;layer=3;row=0;pos=1;prop=69440;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=GLCanvasEditor;state=2099196;dir=3;layer=0;row=0;pos=0;prop=100000;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Log;caption=Log;state=2099196;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(4,1,0)=302|dock_size(3,0,0)=1560|dock_size(3,2,0)=140|dock_size(2,3,0)=321|",
    "layout2|name=GLCanvas;caption=GLCanvas;state=2099196;dir=3;layer=0;row=0;pos=0;prop=97635;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=451;floaty=186;floatw=616;floath=632|name=PanelWatch;caption=Watch;state=2099196;dir=2;layer=3;row=0;pos=0;prop=113200;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099196;dir=2;layer=3;row=0;pos=1;prop=86800;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=GLCanvasEditor;state=2099196;dir=4;layer=1;row=0;pos=1;prop=102365;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-7;floaty=330;floatw=616;floath=632|name=Log;caption=Log;state=2099196;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(4,1,0)=253|dock_size(3,0,0)=1560|dock_size(3,2,0)=140|dock_size(2,3,0)=307|",
    "layout2|name=GLCanvas;caption=GLCanvas;state=2099196;dir=3;layer=0;row=0;pos=0;prop=97635;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=451;floaty=186;floatw=616;floath=632|name=PanelWatch;caption=Watch;state=2099196;dir=2;layer=3;row=0;pos=0;prop=113200;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099196;dir=2;layer=3;row=0;pos=1;prop=86800;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=GLCanvasEditor;state=2099196;dir=3;layer=0;row=0;pos=1;prop=102365;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Log;caption=Log;state=2099196;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(4,1,0)=201|dock_size(3,0,0)=1560|dock_size(3,2,0)=140|dock_size(2,3,0)=307|",
    "layout2|name=GLCanvas;caption=GLCanvas;state=2099196;dir=3;layer=0;row=0;pos=0;prop=97635;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=451;floaty=186;floatw=616;floath=632|name=PanelWatch;caption=Watch;state=2099198;dir=2;layer=3;row=0;pos=0;prop=113200;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099198;dir=2;layer=3;row=0;pos=0;prop=86800;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099198;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=GLCanvasEditor;state=2099198;dir=4;layer=1;row=0;pos=0;prop=102365;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-7;floaty=330;floatw=616;floath=632|name=Log;caption=Log;state=2099198;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(3,0,0)=1560|",
};

void EngineMainFrame_MessageLog(int logtype, const char* tag, const char* message)
{
    if( logtype == 1 )
        g_pEngineMainFrame->m_pLogPane->AppendText( "ERROR: " );

    //g_pEngineMainFrame->m_pLogPane->AppendText( tag );
    //g_pEngineMainFrame->m_pLogPane->AppendText( " " );

    g_pEngineMainFrame->m_pLogPane->AppendText( message );
}

EngineMainFrame::EngineMainFrame()
: MainFrame(0)
{
    m_pCommandStack = 0;
    m_pGLCanvasEditor = 0;
    m_pLogPane = 0;

    m_StackDepthAtLastSave = 0;

    m_EditorPerspectives = 0;
    m_GameplayPerspectives = 0;

    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i] = 0;
        m_GameplayPerspectiveOptions[i] = 0;
    }

    m_Grid = 0;
    m_PlayPauseStop = 0;
    m_Data = 0;
    m_Hackery = 0;
    m_Debug = 0;

    m_MenuItem_GridSnapEnabled = 0;

    m_pEditorPrefs = 0;

    m_GridSettings.snapenabled = false;
    m_GridSettings.stepsize.Set( 5, 5, 5 );
}

void EngineMainFrame::InitFrame()
{
    g_pEngineMainFrame = this;

    FILE* file = 0;
#if MYFW_WINDOWS
    fopen_s( &file, "EditorPrefs.ini", "rb" );
#else
    file = fopen( "EditorPrefs.ini", "rb" );
#endif
    if( file )
    {
        char* string = MyNew char[10000];
        size_t len = fread( string, 1, 10000, file );
        string[len] = 0;
        fclose( file );

        m_pEditorPrefs = cJSON_Parse( string );
        delete[] string;

        if( m_pEditorPrefs )
        {
            int windowx = -9999;
            int windowy = -9999;
            int clientwidth = 0;
            int clientheight = 0;
            bool maximized = false;
            cJSONExt_GetInt( m_pEditorPrefs, "WindowX", &windowx );
            cJSONExt_GetInt( m_pEditorPrefs, "WindowY", &windowy );
            cJSONExt_GetInt( m_pEditorPrefs, "ClientWidth", &clientwidth );
            cJSONExt_GetInt( m_pEditorPrefs, "ClientHeight", &clientheight );
            cJSONExt_GetBool( m_pEditorPrefs, "IsMaximized", &maximized );

            if( clientwidth != 0 )
                m_ClientWidth = clientwidth;
            if( clientheight != 0 )
                m_ClientHeight = clientheight;

            if( windowx != -9999 )
                m_WindowX = windowx;
                
            if( windowy != -9999 )
                m_WindowY = windowy;

            m_Maximized = maximized;
        }
    }

    MainFrame::InitFrame();

    m_pCommandStack = 0;
    m_pGLCanvasEditor = 0;
    m_pLogPane = 0;

    //m_CurrentSceneName[0] = 0;

    g_pMessageLogCallbackFunction = EngineMainFrame_MessageLog;

    m_File->Insert( 0, myIDGame_NewScene, wxT("&New Scene") );
    m_File->Insert( 1, myIDGame_LoadScene, wxT("&Load Scene...") );
    m_File->Insert( 2, myIDGame_LoadAdditionalScene, wxT("&Load Additional Scene...") );
    m_File->Insert( 3, myIDGame_SaveScene, wxT("&Save Scene\tCtrl-S") );
    m_File->Insert( 4, myIDGame_SaveSceneAs, wxT("Save Scene &As...") );

    m_Grid = MyNew wxMenu;
    m_MenuBar->Append( m_Grid, wxT("&Grid") );
    m_MenuItem_GridSnapEnabled = m_Grid->AppendCheckItem( myIDGame_Grid_SnapOnOff, wxT("Grid Snap &On/Off\tCtrl-G") );
    m_Grid->Append( myIDGame_Grid_Settings, wxT("Grid &Settings\tCtrl-Shift-G") );

    m_PlayPauseStop = MyNew wxMenu;
    m_MenuBar->Append( m_PlayPauseStop, wxT("&Mode") );
    m_PlayPauseStop->Append( myIDGame_Mode_PlayStop, wxT("&Play/Stop\tCtrl-SPACE") );
    m_PlayPauseStop->Append( myIDGame_Mode_Pause, wxT("Pause\tCtrl-.") );
    m_PlayPauseStop->Append( myIDGame_Mode_Advance1Frame, wxT("Advance 1 Frame\tCtrl-]") );
    m_PlayPauseStop->Append( myIDGame_Mode_Advance1Second, wxT("Advance 1 Second\tCtrl-[") );
    //m_PlayPauseStop->Append( myIDGame_Mode_Stop, wxT("&Stop\tCtrl-SPACE") );

    m_Data = MyNew wxMenu;
    m_MenuBar->Append( m_Data, wxT("&Data") );
    m_Data->Append( myIDGame_AddDatafile, wxT("&Load Datafiles") );

    m_Hackery = MyNew wxMenu;
    m_MenuBar->Append( m_Hackery, wxT("&Hackery") );
    m_Hackery->Append( myIDGame_RecordMacro, wxT("&Record\tCtrl-R") );
    m_Hackery->Append( myIDGame_ExecuteMacro, wxT("Stop recording and &Execute\tCtrl-E") );

    m_Debug = MyNew wxMenu;
    m_MenuBar->Append( m_Debug, wxT("&Debug views") );
    m_Debug->AppendCheckItem( myIDGame_DebugShowMousePickerFBO, wxT("&Show Mouse Picker FBO\tF9") );
    m_Debug->AppendCheckItem( myIDGame_DebugShowSelectedAnimatedMesh, wxT("Show &Animated Debug View for Selection\tF8") );

    m_Hackery_Record_StackDepth = -1;

    m_EditorPerspectives = MyNew wxMenu;
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i] = m_EditorPerspectives->AppendCheckItem( myIDGame_EditorPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
        Connect( myIDGame_EditorPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    }

    m_GameplayPerspectives = MyNew wxMenu;
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_GameplayPerspectiveOptions[i] = m_GameplayPerspectives->AppendCheckItem( myIDGame_GameplayPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
        Connect( myIDGame_GameplayPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    }

    m_EditorPerspectiveOptions[0]->Check();
    m_GameplayPerspectiveOptions[0]->Check();

    m_View->Append( myIDGame_EditorPerspectives, "Editor Layouts", m_EditorPerspectives );
    m_View->Append( myIDGame_GameplayPerspectives, "Gameplay Layouts", m_GameplayPerspectives );

    Connect( myIDGame_NewScene,     wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_LoadScene,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_LoadAdditionalScene,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveScene,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveSceneAs,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );

    Connect( myIDGame_AddDatafile,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );

    Connect( myIDGame_Grid_SnapOnOff, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_Grid_Settings,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );

    Connect( myIDGame_Mode_PlayStop,       wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_Mode_Pause,          wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_Mode_Advance1Frame,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_Mode_Advance1Second, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    //Connect( myIDGame_Mode_Stop,           wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );

    Connect( myIDGame_RecordMacro,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_ExecuteMacro, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );

    Connect( myIDGame_DebugShowMousePickerFBO,       wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
    Connect( myIDGame_DebugShowSelectedAnimatedMesh, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnGameMenu) );
}

EngineMainFrame::~EngineMainFrame()
{
    SAFE_DELETE( m_pCommandStack );
    SAFE_DELETE( m_pGLCanvasEditor );

    g_pMessageLogCallbackFunction = 0;
}

void EngineMainFrame::AddPanes()
{
    m_pCommandStack = MyNew EngineCommandStack();

    MainFrame::AddPanes();

    // create the editor opengl canvas
    {
        int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
        m_pGLCanvasEditor = MyNew MainGLCanvas( (wxFrame*)this, args, 1, false );
        m_pGLCanvasEditor->SetSize( 600, 600 );

        PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
        pDropTarget->m_pCallbackObj = this;
        pDropTarget->m_pCallbackFunc = StaticOnDrop;

        m_pGLCanvasEditor->SetDropTarget( pDropTarget );
    }

    m_AUIManager.AddPane( m_pGLCanvasEditor, wxAuiPaneInfo().Name("GLCanvasEditor").Bottom().Caption("GLCanvasEditor") );//.CaptionVisible(false) );

    m_pLogPane = new wxTextCtrl( this, -1, wxEmptyString,
                                 wxDefaultPosition, wxSize(200,150),
                                 wxTE_READONLY | wxNO_BORDER | wxTE_MULTILINE );
    m_AUIManager.AddPane( m_pLogPane, wxAuiPaneInfo().Name("Log").Bottom().Caption("Log") );//.CaptionVisible(false) );
}

bool EngineMainFrame::UpdateAUIManagerAndLoadPerspective()
{
    if( MainFrame::UpdateAUIManagerAndLoadPerspective() )
        return true;

    // layout.ini file not found, use the default editor layout.
    int currentperspective = GetDefaultEditorPerspectiveIndex();
    m_AUIManager.LoadPerspective( g_DefaultPerspectives[currentperspective] );

    // say a valid layout was set.
    return true;
}

void EngineMainFrame::OnPostInit()
{
    MainFrame::OnPostInit();

    if( m_pEditorPrefs )
    {
        cJSON* obj;

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "LastSceneLoaded" );
        if( obj )
            LoadScene( obj->valuestring, true );

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "EditorCam" );
        if( obj )
            g_pEngineCore->m_pEditorState->GetEditorCamera()->m_pComponentTransform->ImportFromJSONObject( obj, 1 );

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "EditorLayout" );
        if( obj )
            SetDefaultEditorPerspectiveIndex( obj->valueint );

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "GameplayLayout" );
        if( obj )
            SetDefaultGameplayPerspectiveIndex( obj->valueint );

        extern GLViewTypes g_CurrentGLViewType;
        cJSONExt_GetInt( m_pEditorPrefs, "GameAspectRatio", (int*)&g_CurrentGLViewType );

        cJSONExt_GetBool( m_pEditorPrefs, "GridSnapEnabled", &m_GridSettings.snapenabled );
        cJSONExt_GetFloatArray( m_pEditorPrefs, "GridStepSize", &m_GridSettings.stepsize.x, 3 );

        cJSON_Delete( m_pEditorPrefs );
        m_pEditorPrefs = 0;
    }

    m_pGLCanvas->ResizeViewport();

    UpdateMenuItemStates();
}

bool EngineMainFrame::OnClose()
{
    int answer = wxYES;

    if( m_pCommandStack->m_UndoStack.size() != m_StackDepthAtLastSave )
    {
        answer = wxMessageBox( "Some changes aren't saved.\nQuit anyway?", "Confirm", wxYES_NO, this );
    }

    if( answer == wxYES )
    {
        bool parentwantstoclose = MainFrame::OnClose();

        if( g_pEngineCore == 0 )
            return parentwantstoclose;

        g_pEngineCore->m_pEditorState->ClearKeyAndActionStates();
        g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

        FILE* file = 0;
    #if MYFW_WINDOWS
        fopen_s( &file, "EditorPrefs.ini", "wb" );
    #else
        file = fopen( "EditorPrefs.ini", "wb" );
    #endif
        if( file )
        {
            cJSON* pPrefs = cJSON_CreateObject();

            cJSON_AddNumberToObject( pPrefs, "WindowX", m_WindowX );
            cJSON_AddNumberToObject( pPrefs, "WindowY", m_WindowY );
            cJSON_AddNumberToObject( pPrefs, "ClientWidth", m_ClientWidth );
            cJSON_AddNumberToObject( pPrefs, "ClientHeight", m_ClientHeight );
            cJSON_AddNumberToObject( pPrefs, "IsMaximized", m_Maximized );

            cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", g_pComponentSystemManager->GetSceneInfo( 1 )->fullpath );
            cJSON_AddItemToObject( pPrefs, "EditorCam", g_pEngineCore->m_pEditorState->GetEditorCamera()->m_pComponentTransform->ExportAsJSONObject( false ) );
            cJSON_AddNumberToObject( pPrefs, "EditorLayout", GetDefaultEditorPerspectiveIndex() );
            cJSON_AddNumberToObject( pPrefs, "GameplayLayout", GetDefaultGameplayPerspectiveIndex() );
            extern GLViewTypes g_CurrentGLViewType;
            cJSON_AddNumberToObject( pPrefs, "GameAspectRatio", g_CurrentGLViewType );

            cJSON_AddNumberToObject( pPrefs, "GridSnapEnabled", m_GridSettings.snapenabled );
            cJSONExt_AddFloatArrayToObject( pPrefs, "GridStepSize", &m_GridSettings.stepsize.x, 3 );

            char* string = cJSON_Print( pPrefs );
            cJSON_Delete( pPrefs );

            fprintf( file, "%s", string );
            fclose( file );

            cJSONExt_free( string );
        }

        return parentwantstoclose;
    }

    return false;
}

void EngineMainFrame::ResizeViewport()
{
    MainFrame::ResizeViewport();

    m_pGLCanvasEditor->ResizeViewport();
}

void EngineMainFrame::UpdateMenuItemStates()
{
    MainFrame::UpdateMenuItemStates();

    if( m_GridSettings.snapenabled )
        m_MenuItem_GridSnapEnabled->Check( m_GridSettings.snapenabled );
}

void EngineMainFrame::OnGameMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myIDGame_NewScene:
        {
            int answer = wxYES;

            if( m_pCommandStack->m_UndoStack.size() != m_StackDepthAtLastSave )
            {
                answer = wxMessageBox( "Some changes aren't saved.\nCreate a new scene?", "Confirm", wxYES_NO, this );
            }

            if( answer == wxYES )
            {
                this->SetTitle( "New scene" );
                g_pEngineCore->UnloadScene( UINT_MAX, false );
                g_pComponentSystemManager->GetSceneInfo( 1 )->fullpath[0] = 0;
                g_pEngineCore->CreateDefaultSceneObjects( false );
                ResizeViewport();
            }
        }
        break;

    case myIDGame_LoadScene:
        {
            int answer = wxYES;

            if( m_pCommandStack->m_UndoStack.size() != m_StackDepthAtLastSave )
            {
                answer = wxMessageBox( "Some changes aren't saved.\nLoad anyway?", "Confirm", wxYES_NO, this );
            }

            if( answer == wxYES )
            {
                LoadSceneDialog( true );
                ResizeViewport();
            }
        }
        break;

    case myIDGame_LoadAdditionalScene:
        {
            LoadSceneDialog( false );
            ResizeViewport();
        }
        break;

    case myIDGame_SaveScene:
        m_StackDepthAtLastSave = (unsigned int)m_pCommandStack->m_UndoStack.size();
        g_pMaterialManager->SaveAllMaterials();
        g_pComponentSystemManager->AddAllMaterialsToFilesList();
        SaveScene();
        break;

    case myIDGame_SaveSceneAs:
        m_StackDepthAtLastSave = (unsigned int)m_pCommandStack->m_UndoStack.size();
        g_pMaterialManager->SaveAllMaterials();
        g_pComponentSystemManager->AddAllMaterialsToFilesList();
        SaveSceneAs();
        break;

    case myIDGame_AddDatafile:
        AddDatafileToScene();
        break;

    case myIDGame_Grid_SnapOnOff:
        m_GridSettings.snapenabled = !m_GridSettings.snapenabled;
        break;

    case myIDGame_Grid_Settings:
        {
            // should be in an "gl frame lost focus" state handling.
            g_pEngineCore->m_pEditorState->ClearKeyAndActionStates();

            DialogGridSettings dialog( this, -1, _("Grid Settings"), GetPosition() + wxPoint(60,60), wxSize(200, 200) );
            dialog.ShowModal();
            //if( dialog.ShowModal() != wxID_OK )
            //    LOGInfo( LOGTag, "Cancel Pressed.\n" );
            //else
            //    LOGInfo( LOGTag, "OK pressed.\n" );
        }
        break;

    case myIDGame_Mode_PlayStop:
        g_pEngineCore->OnModeTogglePlayStop();
        break;

    case myIDGame_Mode_Pause:
        g_pEngineCore->OnModePause();
        break;

    case myIDGame_Mode_Advance1Frame:
        g_pEngineCore->OnModeAdvanceTime( 1/60.0f );
        break;

    case myIDGame_Mode_Advance1Second:
        g_pEngineCore->OnModeAdvanceTime( 1.0f );
        break;

    //case myIDGame_Mode_Stop:
    //    g_pEngineCore->OnModeTogglePlayStop();
    //    break;

    case myIDGame_RecordMacro:
        m_Hackery_Record_StackDepth = (int)m_pCommandStack->m_UndoStack.size();
        break;

    case myIDGame_ExecuteMacro:
        if( m_Hackery_Record_StackDepth != -1 )
        {
            int topdepth = (int)m_pCommandStack->m_UndoStack.size();
            for( int i=m_Hackery_Record_StackDepth; i<topdepth; i++ )
            {
                // need to copy the command.
                EditorCommand* pCommand = m_pCommandStack->m_UndoStack[i]->Repeat();
                if( pCommand )
                    m_pCommandStack->Add( pCommand );
            }
            m_Hackery_Record_StackDepth = topdepth;
        }
        break;

    case myIDGame_EditorPerspective + 0:
    case myIDGame_EditorPerspective + 1:
    case myIDGame_EditorPerspective + 2:
    case myIDGame_EditorPerspective + 3:
        SetDefaultEditorPerspectiveIndex( id - myIDGame_EditorPerspective );
        SetWindowPerspectiveToDefault( true );
        break;

    case myIDGame_GameplayPerspective + 0:
    case myIDGame_GameplayPerspective + 1:
    case myIDGame_GameplayPerspective + 2:
    case myIDGame_GameplayPerspective + 3:
        SetDefaultGameplayPerspectiveIndex( id - myIDGame_GameplayPerspective );
        SetWindowPerspectiveToDefault( true );
        break;

    case myIDGame_DebugShowMousePickerFBO:
        g_pEngineCore->m_Debug_DrawMousePickerFBO = !g_pEngineCore->m_Debug_DrawMousePickerFBO;
        break;

    case myIDGame_DebugShowSelectedAnimatedMesh:
        g_pEngineCore->m_Debug_DrawSelectedAnimatedMesh = !g_pEngineCore->m_Debug_DrawSelectedAnimatedMesh;
        break;
    }
}

void EngineMainFrame::SetWindowPerspectiveToDefault(bool forceswitch)
{
    int currentperspective;
    int editor = GetDefaultEditorPerspectiveIndex();
    int gameplay = GetDefaultGameplayPerspectiveIndex();

    // don't touch anything if both perspectives are the same.
    if( forceswitch == false && editor == gameplay )
        return;

    if( g_pEngineCore->m_EditorMode )
    {
        currentperspective = editor;
    }
    else
    {
        currentperspective = gameplay;
    }

    if( currentperspective != -1 )
        m_AUIManager.LoadPerspective( g_DefaultPerspectives[currentperspective] );
}

int EngineMainFrame::GetDefaultEditorPerspectiveIndex()
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        if( m_EditorPerspectiveOptions[i]->IsChecked() )
            return i;
    }

    MyAssert( false );
    return -1;
}

int EngineMainFrame::GetDefaultGameplayPerspectiveIndex()
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        if( m_GameplayPerspectiveOptions[i]->IsChecked() )
            return i;
    }

    MyAssert( false );
    return -1;
}

void EngineMainFrame::SetDefaultEditorPerspectiveIndex(int index)
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i]->Check( false );
    }
    m_EditorPerspectiveOptions[index]->Check( true );
}

void EngineMainFrame::SetDefaultGameplayPerspectiveIndex(int index)
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_GameplayPerspectiveOptions[i]->Check( false );
    }
    m_GameplayPerspectiveOptions[index]->Check( true );
}

void EngineMainFrame::SaveScene()
{
    if( g_pComponentSystemManager->GetSceneInfo( 1 )->fullpath[0] == 0 )
    {
        SaveSceneAs();
    }
    else
    {
        if( g_pEngineCore->m_EditorMode == false )
        {
            LOGInfo( LOGTag, "Can't save when gameplay is active... use \"Save As\"\n" );
        }
        else
        {
            typedef std::map<int, SceneInfo>::iterator it_type;
            for( it_type iterator = g_pComponentSystemManager->m_pSceneIDToSceneTreeIDMap.begin(); iterator != g_pComponentSystemManager->m_pSceneIDToSceneTreeIDMap.end(); iterator++ )
            {
                unsigned int sceneid = iterator->first;
                SceneInfo* pSceneInfo = &iterator->second;

                LOGInfo( LOGTag, "Saving scene... %s\n", pSceneInfo->fullpath );
                g_pEngineCore->SaveScene( pSceneInfo->fullpath, sceneid );
            }
        }
    }
}

void EngineMainFrame::SaveSceneAs()
{
    wxFileDialog FileDialog( this, _("Save Scene file"), "./Data/Scenes", "", "Scene files (*.scene)|*.scene", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // save the current scene
    // TODO: typecasting will likely cause issues with multibyte names
    wxString wxpath = FileDialog.GetPath();
    sprintf_s( g_pComponentSystemManager->GetSceneInfo( 1 )->fullpath, 260, "%s", (const char*)wxpath );

    g_pMaterialManager->SaveAllMaterials();
    g_pComponentSystemManager->AddAllMaterialsToFilesList();
    g_pEngineCore->SaveScene( g_pComponentSystemManager->GetSceneInfo( 1 )->fullpath, 1 );

    this->SetTitle( g_pComponentSystemManager->GetSceneInfo( 1 )->fullpath );
}

void EngineMainFrame::LoadSceneDialog(bool unloadscenes)
{
    //if( scene is dirty )
    //{
    //    if (wxMessageBox(_("Current content has not been saved! Proceed?"), _("Please confirm"),
    //        wxICON_QUESTION | wxYES_NO, this) == wxNO )
    //    return;
    //}

    wxFileDialog FileDialog( this, _("Open Scene"), "./Data/Scenes", "", "Scene files (*.scene)|*.scene", wxFD_OPEN|wxFD_FILE_MUST_EXIST );
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;

    // load the file chosen by the user
    // TODO: typecasting will likely cause issues with multibyte names
    wxString wxpath = FileDialog.GetPath();
    LoadScene( wxpath, unloadscenes );
}

void EngineMainFrame::LoadScene(const char* scenename, bool unloadscenes)
{
    MyAssert( scenename != 0 );

    //strcpy_s( m_CurrentSceneName, 260, scenename );

    // clear out the old scene before loading
    // TODO: make this optional, so we can load multiple scenes at once, also change the '1' in LoadScene to the next available scene id
    if( unloadscenes )
        g_pEngineCore->UnloadScene( UINT_MAX, false ); // don't unload editor objects.
    unsigned int sceneid = g_pComponentSystemManager->GetNextSceneID();
    g_pEngineCore->LoadSceneFromFile( scenename, sceneid );

    this->SetTitle( g_pComponentSystemManager->GetSceneInfo( sceneid )->fullpath );
}

void EngineMainFrame::AddDatafileToScene()
{
    // multiple select file open dialog
    wxFileDialog FileDialog( this, _("Open Datafile"), "./Data", "", "All files(*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE );
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // load the files chosen by the user
    // TODO: typecasting will likely cause issues with multibyte names
    wxArrayString patharray;
    FileDialog.GetPaths( patharray );

    char fullpath[MAX_PATH];
    for( unsigned int filenum=0; filenum<patharray.Count(); filenum++ )
    {
        sprintf_s( fullpath, MAX_PATH, "%s", (const char*)patharray[filenum] );
        unsigned int fullpathlen = (unsigned int)strlen( fullpath );

        char dirpath[MAX_PATH];
#if MYFW_WINDOWS
        GetCurrentDirectoryA( MAX_PATH, dirpath );
#else
        dirpath[0] = 0;
#endif

        // if the datafile is in our working directory, then load it... otherwise TODO: copy it in?
        unsigned int dirpathlen = (unsigned int)strlen(dirpath);
        if( strncmp( dirpath, fullpath, dirpathlen ) == 0 )
        {
            for( unsigned int i=dirpathlen; i<fullpathlen-1; i++ )
            {
                fullpath[i-dirpathlen] = fullpath[i+1];
                if( fullpath[i-dirpathlen] == '\\' )
                    fullpath[i-dirpathlen] = '/';
                fullpath[i-dirpathlen+1] = 0;
            }
        }
        else
        {
            // File is not in our working directory.
            // TODO: copy the file into our data folder?
            LOGError( LOGTag, "file must be in working directory\n" );
            //MyAssert( false );
            return;
        }

        // fullpath is actually a relative path at this point.
        g_pEngineCore->m_pComponentSystemManager->LoadDatafile( fullpath, 1 );
    }
}

void EngineMainFrame::OnDrop(int controlid, wxCoord x, wxCoord y)
{
    // get the GameObject the mouse was hovering over.
    ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();
    y = pCamera->m_WindowHeight - y; // prefer 0,0 at bottom left.
    GameObject* pObject = g_pEngineCore->GetObjectAtPixel( x, y, true );

    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;

        if( pMaterial && pObject )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeAllMaterialsOnGameObject( pObject, pMaterial ) );
        }
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        TextureDefinition* pTexture = (TextureDefinition*)g_DragAndDropStruct.m_Value;

        if( pTexture && pObject && pObject->GetMaterial() )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeTextureOnMaterial( pObject->GetMaterial(), pTexture ) );
        }
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShader = (ShaderGroup*)g_DragAndDropStruct.m_Value;

        if( pShader && pObject && pObject->GetMaterial() )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeShaderOnMaterial( pObject->GetMaterial(), pShader ) );
        }
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );

        if( pFile && strcmp( pFile->m_ExtensionWithDot, ".lua" ) == 0 )
        {
            if( pObject )
            {
                g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeAllScriptsOnGameObject( pObject, pFile ) );
            }
        }

        if( pFile && strcmp( pFile->m_ExtensionWithDot, ".glsl" ) == 0 )
        {
            if( pObject && pObject->GetMaterial() )
            {
                ShaderGroup* pShader = g_pShaderGroupManager->FindShaderGroupByFile( pFile );
                g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeShaderOnMaterial( pObject->GetMaterial(), pShader ) );
            }
        }

        if( pFile &&
            ( strcmp( pFile->m_ExtensionWithDot, ".obj" ) == 0 || strcmp( pFile->m_ExtensionWithDot, ".mymesh" ) == 0 )
          )
        {
            // TODO: undo/redo

            // create a new gameobject using this obj.
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );

            GameObject* pGameObject = g_pComponentSystemManager->CreateGameObject( true, 1 );
            //pGameObject->SetSceneID( 1 );
            pGameObject->SetName( "New mesh" );
            ComponentMeshOBJ* pComponentMeshOBJ = (ComponentMeshOBJ*)pGameObject->AddNewComponent( ComponentType_MeshOBJ, 1 );
            pComponentMeshOBJ->SetSceneID( 1 );
            pComponentMeshOBJ->SetMaterial( (MaterialDefinition*)g_pMaterialManager->m_Materials.GetHead(), 0 );
            pComponentMeshOBJ->SetMesh( pMesh );
            pComponentMeshOBJ->m_LayersThisExistsOn = Layer_MainScene;

            if( pObject && pObject->GetMaterial() )
            {
                // place it just above of the object selected otherwise place it at 0,0,0... for now.
                pGameObject->m_pComponentTransform->SetPosition( pObject->m_pComponentTransform->GetPosition() );
            }
        }
    }

    //if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    //{
    //    GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;
    //    MyAssert( pGameObject );

    //    int id = g_DragAndDropStruct.m_ID - m_ControlIDOfFirstExtern;
    //    
    //    // TODO: this will make a mess of memory if different types of objects can be dragged in...
    //    m_ExposedVars[id]->pointer = pGameObject;

    //    // update the panel so new gameobject name shows up.
    //    g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pGameObject->GetName();
    //}
}

#endif //MYFW_USING_WX
