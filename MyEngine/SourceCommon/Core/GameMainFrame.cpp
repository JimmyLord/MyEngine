//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "GameMainFrame.h"

GameMainFrame* g_pGameMainFrame = 0;

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

void GameMainFrame_MessageLog(int logtype, const char* tag, const char* message)
{
    if( logtype == 1 )
        g_pGameMainFrame->m_pLogPane->AppendText( "ERROR: " );

    //g_pGameMainFrame->m_pLogPane->AppendText( tag );
    //g_pGameMainFrame->m_pLogPane->AppendText( " " );

    g_pGameMainFrame->m_pLogPane->AppendText( message );
}

GameMainFrame::GameMainFrame()
: MainFrame(0)
{
    m_pGLCanvasEditor = 0;
    m_pLogPane = 0;

    m_EditorPerspectives = 0;
    m_GameplayPerspectives = 0;

    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i] = 0;
        m_GameplayPerspectiveOptions[i] = 0;
    }

    m_Data = 0;
    m_Hackery = 0;
    m_Debug = 0;

    m_pEditorPrefs = 0;
}

void GameMainFrame::InitFrame()
{
    g_pGameMainFrame = this;

    FILE* file = 0;
    fopen_s( &file, "EditorPrefs.ini", "rb" );
    if( file )
    {
        char* string = MyNew char[10000];
        int len = fread( string, 1, 10000, file );
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

    m_pGLCanvasEditor = 0;
    m_pLogPane = 0;

    m_CurrentSceneName[0] = 0;

    g_pMessageLogCallbackFunction = GameMainFrame_MessageLog;

    m_File->Insert( 0, myIDGame_LoadScene, wxT("&Load Scene") );
    m_File->Insert( 1, myIDGame_SaveScene, wxT("&Save Scene\tCtrl-S") );
    m_File->Insert( 2, myIDGame_SaveSceneAs, wxT("Save Scene &As") );

    m_Data = MyNew wxMenu;
    m_MenuBar->Append( m_Data, wxT("&Data") );
    m_Data->Append( myIDGame_AddDatafile, wxT("&Load Datafiles") );

    m_Hackery = MyNew wxMenu;
    m_MenuBar->Append( m_Hackery, wxT("&Hackery") );
    m_Hackery->Append( myIDGame_RecordMacro, wxT("&Record\tCtrl-R") );
    m_Hackery->Append( myIDGame_ExecuteMacro, wxT("Stop recording and &Execute\tCtrl-E") );

    m_Debug = MyNew wxMenu;
    m_MenuBar->Append( m_Debug, wxT("&Debug views") );
    m_Debug->Append( myIDGame_DebugShowMousePickerFBO, wxT("&Show Mouse Picker FBO\tF9") );
    m_Debug->Append( myIDGame_DebugShowSelectedAnimatedMesh, wxT("Show &Animated Debug View for Selection\tF8") );

    m_Hackery_Record_StackDepth = -1;

    m_EditorPerspectives = MyNew wxMenu;
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i] = m_EditorPerspectives->AppendCheckItem( myIDGame_EditorPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
        Connect( myIDGame_EditorPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    }

    m_GameplayPerspectives = MyNew wxMenu;
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_GameplayPerspectiveOptions[i] = m_GameplayPerspectives->AppendCheckItem( myIDGame_GameplayPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
        Connect( myIDGame_GameplayPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    }

    m_EditorPerspectiveOptions[0]->Check();
    m_GameplayPerspectiveOptions[0]->Check();

    m_View->Append( myIDGame_EditorPerspectives, "Editor Layouts", m_EditorPerspectives );
    m_View->Append( myIDGame_GameplayPerspectives, "Gameplay Layouts", m_GameplayPerspectives );

    Connect( myIDGame_LoadScene,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveScene,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveSceneAs,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );

    Connect( myIDGame_AddDatafile,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );

    Connect( myIDGame_RecordMacro,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_ExecuteMacro, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );

    Connect( myIDGame_DebugShowMousePickerFBO,       wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_DebugShowSelectedAnimatedMesh, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
}

GameMainFrame::~GameMainFrame()
{
    SAFE_DELETE( m_pCommandStack );
    SAFE_DELETE( m_pGLCanvasEditor );

    g_pMessageLogCallbackFunction = 0;
}

void GameMainFrame::AddPanes()
{
    m_pCommandStack = MyNew GameCommandStack();

    MainFrame::AddPanes();

    // create the editor opengl canvas
    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    m_pGLCanvasEditor = MyNew MainGLCanvas( (wxFrame*)this, args, 1, false );
    m_pGLCanvasEditor->SetSize( 600, 600 );

    m_AUIManager.AddPane( m_pGLCanvasEditor, wxAuiPaneInfo().Name("GLCanvasEditor").Bottom().Caption("GLCanvasEditor") );//.CaptionVisible(false) );

    m_pLogPane = new wxTextCtrl( this, -1, wxEmptyString,
                                 wxDefaultPosition, wxSize(200,150),
                                 wxTE_READONLY | wxNO_BORDER | wxTE_MULTILINE );
    m_AUIManager.AddPane( m_pLogPane, wxAuiPaneInfo().Name("Log").Bottom().Caption("Log") );//.CaptionVisible(false) );
}

bool GameMainFrame::UpdateAUIManagerAndLoadPerspective()
{
    if( MainFrame::UpdateAUIManagerAndLoadPerspective() )
        return true;

    // layout.ini file not found, use the default editor layout.
    int currentperspective = GetDefaultEditorPerspectiveIndex();
    m_AUIManager.LoadPerspective( g_DefaultPerspectives[currentperspective] );

    // say a valid layout was set.
    return true;
}

void GameMainFrame::OnPostInit()
{
    if( m_pEditorPrefs )
    {
        cJSON* obj;

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "LastSceneLoaded" );
        if( obj )
            LoadScene( obj->valuestring );

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "EditorCam" );
        if( obj )
            ((GameEntityComponentTest*)g_pGameCore)->m_pEditorState->GetEditorCamera()->m_pComponentTransform->ImportFromJSONObject( obj, 1 );

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "EditorLayout" );
        if( obj )
            SetDefaultEditorPerspectiveIndex( obj->valueint );

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "GameplayLayout" );
        if( obj )
            SetDefaultGameplayPerspectiveIndex( obj->valueint );

        extern GLViewTypes g_CurrentGLViewType;
        cJSONExt_GetInt( m_pEditorPrefs, "GameAspectRatio", (int*)&g_CurrentGLViewType );

        cJSON_Delete( m_pEditorPrefs );
        m_pEditorPrefs = 0;
    }

    m_pGLCanvas->ResizeViewport();
}

void GameMainFrame::OnClose()
{
    FILE* file = 0;
    fopen_s( &file, "EditorPrefs.ini", "wb" );
    if( file )
    {
        cJSON* pPrefs = cJSON_CreateObject();

        cJSON_AddNumberToObject( pPrefs, "WindowX", m_WindowX );
        cJSON_AddNumberToObject( pPrefs, "WindowY", m_WindowY );
        cJSON_AddNumberToObject( pPrefs, "ClientWidth", m_ClientWidth );
        cJSON_AddNumberToObject( pPrefs, "ClientHeight", m_ClientHeight );
        cJSON_AddNumberToObject( pPrefs, "IsMaximized", m_Maximized );

        cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", m_CurrentSceneName );
        cJSON_AddItemToObject( pPrefs, "EditorCam", ((GameEntityComponentTest*)g_pGameCore)->m_pEditorState->GetEditorCamera()->m_pComponentTransform->ExportAsJSONObject() );
        cJSON_AddNumberToObject( pPrefs, "EditorLayout", GetDefaultEditorPerspectiveIndex() );
        cJSON_AddNumberToObject( pPrefs, "GameplayLayout", GetDefaultGameplayPerspectiveIndex() );
        extern GLViewTypes g_CurrentGLViewType;
        cJSON_AddNumberToObject( pPrefs, "GameAspectRatio", g_CurrentGLViewType );

        char* string = cJSON_Print( pPrefs );

        fprintf( file, string );
        fclose( file );

        free( string );
    }
}

void GameMainFrame::OnGameMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myIDGame_LoadScene:
        LoadSceneDialog();
        ResizeViewport();
        break;

    case myIDGame_SaveScene:
        SaveScene();
        break;

    case myIDGame_SaveSceneAs:
        SaveSceneAs();
        break;

    case myIDGame_AddDatafile:
        AddDatafileToScene();
        break;

    case myIDGame_RecordMacro:
        m_Hackery_Record_StackDepth = m_pCommandStack->m_UndoStack.size();
        break;

    case myIDGame_ExecuteMacro:
        if( m_Hackery_Record_StackDepth != -1 )
        {
            int topdepth = m_pCommandStack->m_UndoStack.size();
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
        ((GameEntityComponentTest*)g_pGameCore)->m_Debug_DrawMousePickerFBO = !((GameEntityComponentTest*)g_pGameCore)->m_Debug_DrawMousePickerFBO;
        break;

    case myIDGame_DebugShowSelectedAnimatedMesh:
        ((GameEntityComponentTest*)g_pGameCore)->m_Debug_DrawSelectedAnimatedMesh = !((GameEntityComponentTest*)g_pGameCore)->m_Debug_DrawSelectedAnimatedMesh;
        break;
    }
}

void GameMainFrame::ResizeViewport()
{
    MainFrame::ResizeViewport();

    m_pGLCanvasEditor->ResizeViewport();
}

void GameMainFrame::SetWindowPerspectiveToDefault(bool forceswitch)
{
    int currentperspective;
    int editor = GetDefaultEditorPerspectiveIndex();
    int gameplay = GetDefaultGameplayPerspectiveIndex();

    // don't touch anything if both perspectives are the same.
    if( forceswitch == false && editor == gameplay )
        return;

    if( ((GameEntityComponentTest*)g_pGameCore)->m_EditorMode )
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

int GameMainFrame::GetDefaultEditorPerspectiveIndex()
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        if( m_EditorPerspectiveOptions[i]->IsChecked() )
            return i;
    }

    assert( false );
    return -1;
}

int GameMainFrame::GetDefaultGameplayPerspectiveIndex()
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        if( m_GameplayPerspectiveOptions[i]->IsChecked() )
            return i;
    }

    assert( false );
    return -1;
}

void GameMainFrame::SetDefaultEditorPerspectiveIndex(int index)
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i]->Check( false );
    }
    m_EditorPerspectiveOptions[index]->Check( true );
}

void GameMainFrame::SetDefaultGameplayPerspectiveIndex(int index)
{
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_GameplayPerspectiveOptions[i]->Check( false );
    }
    m_GameplayPerspectiveOptions[index]->Check( true );
}

void GameMainFrame::SaveScene()
{
    if( m_CurrentSceneName[0] == 0 )
    {
        SaveSceneAs();
    }
    else
    {
        if( ((GameEntityComponentTest*)g_pGameCore)->m_EditorMode == false )
        {
            m_pLogPane->AppendText( "Can't save when gameplay is active... use \"Save As\"\n" );
        }
        else
        {
            m_pLogPane->AppendText( "Saving scene...\n" );
            ((GameEntityComponentTest*)g_pGameCore)->SaveScene( m_CurrentSceneName );
        }
    }
}

void GameMainFrame::SaveSceneAs()
{
    wxFileDialog FileDialog( this, _("Save Scene file"), "./Data/Scenes", "", "Scene files (*.scene)|*.scene", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // save the current scene
    // TODO: typecasting will likely cause issues with multibyte names
    wxString wxpath = FileDialog.GetPath();
    sprintf_s( m_CurrentSceneName, 260, "%s", (const char*)wxpath );

    ((GameEntityComponentTest*)g_pGameCore)->SaveScene( m_CurrentSceneName );

    this->SetTitle( m_CurrentSceneName );
}

void GameMainFrame::LoadSceneDialog()
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
    LoadScene( wxpath );
}

void GameMainFrame::LoadScene(const char* scenename)
{
    assert( scenename != 0 );

    strcpy_s( m_CurrentSceneName, 260, scenename );

    // clear out the old scene before loading
    // TODO: make this optional, so we can load multiple scenes at once, also change the '1' in LoadScene to the next available scene id
    ((GameEntityComponentTest*)g_pGameCore)->UnloadScene();
    ((GameEntityComponentTest*)g_pGameCore)->LoadSceneFromFile( m_CurrentSceneName, 1 );

    this->SetTitle( m_CurrentSceneName );
}

void GameMainFrame::AddDatafileToScene()
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
        unsigned int fullpathlen = strlen( fullpath );

        char dirpath[MAX_PATH];
        GetCurrentDirectoryA( MAX_PATH, dirpath );

        // if the datafile is in our working directory, then load it... otherwise TODO: copy it in?
        size_t dirpathlen = strlen(dirpath);
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
            //assert( false );
            return;
        }

        // fullpath is actually a relative path at this point.
        ((GameEntityComponentTest*)g_pGameCore)->m_pComponentSystemManager->LoadDatafile( fullpath, 1 );
    }
}
