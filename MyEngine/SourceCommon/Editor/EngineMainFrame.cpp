//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX

#include "../../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "EngineMainFrame.h"

EngineMainFrame* g_pEngineMainFrame = 0;

const char* g_DefaultPerspectiveMenuLabels[Perspective_NumPerspectives] =
{
    "Center &Editor",
    "Center &Game",
    "&Side by Side",
    "&Full Frame Game",
};

const char* g_DefaultEngineEditorWindowTypeMenuLabels[EngineEditorWindow_NumTypes] =
{
    "&Editor View",
    "&Log Panel",
};

char* g_DefaultPerspectives[Perspective_NumPerspectives] =
{
    "layout2|name=GLCanvas;caption=Game;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-253;floaty=231;floatw=680;floath=748|name=PanelWatch;caption=Watch;state=2099196;dir=2;layer=3;row=0;pos=0;prop=130560;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099196;dir=2;layer=3;row=0;pos=1;prop=69440;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=Editor;state=2099196;dir=3;layer=0;row=0;pos=0;prop=100000;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Log;caption=Log;state=2099196;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(4,1,0)=302|dock_size(3,0,0)=1560|dock_size(3,2,0)=140|dock_size(2,3,0)=321|",
    "layout2|name=GLCanvas;caption=Game;state=2099196;dir=3;layer=0;row=0;pos=0;prop=97635;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-253;floaty=231;floatw=680;floath=748|name=PanelWatch;caption=Watch;state=2099196;dir=2;layer=3;row=0;pos=0;prop=113200;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099196;dir=2;layer=3;row=0;pos=1;prop=86800;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=Editor;state=2099196;dir=4;layer=1;row=0;pos=1;prop=102365;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-7;floaty=330;floatw=616;floath=632|name=Log;caption=Log;state=2099196;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(4,1,0)=253|dock_size(3,0,0)=1560|dock_size(3,2,0)=140|dock_size(2,3,0)=307|",
    "layout2|name=GLCanvas;caption=Game;state=2099196;dir=3;layer=0;row=0;pos=0;prop=97635;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-253;floaty=231;floatw=680;floath=748|name=PanelWatch;caption=Watch;state=2099196;dir=2;layer=3;row=0;pos=0;prop=113200;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099196;dir=2;layer=3;row=0;pos=1;prop=86800;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=Editor;state=2099196;dir=3;layer=0;row=0;pos=1;prop=102365;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Log;caption=Log;state=2099196;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(4,1,0)=201|dock_size(3,0,0)=1560|dock_size(3,2,0)=140|dock_size(2,3,0)=307|",
    "layout2|name=GLCanvas;caption=Game;state=2099196;dir=3;layer=0;row=0;pos=0;prop=97635;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-253;floaty=231;floatw=680;floath=748|name=PanelWatch;caption=Watch;state=2099198;dir=2;layer=3;row=0;pos=0;prop=113200;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1274;floaty=143;floatw=316;floath=632|name=PanelMemory;caption=Memory;state=2099198;dir=2;layer=3;row=0;pos=0;prop=86800;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1023;floaty=335;floatw=316;floath=632|name=PanelObjectList;caption=Objects;state=2099198;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GLCanvasEditor;caption=Editor;state=2099198;dir=4;layer=1;row=0;pos=0;prop=102365;bestw=600;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-7;floaty=330;floatw=616;floath=632|name=Log;caption=Log;state=2099198;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=183;besth=150;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=543;floaty=588;floatw=199;floath=182|dock_size(3,0,0)=1560|",
};

char* g_SavedPerspectives[Perspective_NumPerspectives] =
{
    g_DefaultPerspectives[0],
    g_DefaultPerspectives[1],
    g_DefaultPerspectives[2],
    g_DefaultPerspectives[3],
};

struct MessageLog
{
    int logtype;
    wxString tag;
    wxString message;
};

std::vector<MessageLog> g_LoggedMessages;
pthread_mutex_t g_MessageLogMutex;

void EngineMainFrame_MessageLog(int logtype, const char* tag, const char* message)
{
    MessageLog logentry;
    logentry.logtype = logtype;
    logentry.tag = tag;
    logentry.message = message;

    pthread_mutex_lock( &g_MessageLogMutex );

    g_LoggedMessages.push_back( logentry );

    pthread_mutex_unlock( &g_MessageLogMutex );
}

void EngineMainFrame_MessageLogForReal(int logtype, const char* tag, const char* message)
{
    // TODO: writing to "g_pEngineMainFrame->m_pLogPane" only works from main thread, will assert otherwise.  fix me. 

    // Info and Error (not debug) messages go into the "All" log.
    if( logtype != 2 )
    {
        if( logtype == 1 )
            g_pEngineMainFrame->m_pLogMain->AppendText( "ERROR: " );
        g_pEngineMainFrame->m_pLogMain->AppendText( message );
    }

    // All warnings go into the "Info" log.
    if( logtype == 0 )
        g_pEngineMainFrame->m_pLogInfo->AppendText( message );

    // All errors go into the "Errors" log.
    if( logtype == 1 )
        g_pEngineMainFrame->m_pLogErrors->AppendText( message );

    // If the default tag wasn't used, find or create an extra log window for that tag.
    wxTextCtrl* pCustomLogTextCtrl = 0;
    if( tag != LOGTag )
    {
        // Find custom box if it exists.
        for( unsigned int i=0; i<g_pEngineMainFrame->m_pLogPane->GetPageCount(); i++ )
        {
            if( g_pEngineMainFrame->m_pLogPane->GetPageText( i ) == tag )
            {
                pCustomLogTextCtrl = (wxTextCtrl*)g_pEngineMainFrame->m_pLogPane->GetPage( i );
                break;
            }
        }

        // if not, create it.
        if( pCustomLogTextCtrl == 0 )
        {
            pCustomLogTextCtrl = new wxTextCtrl( g_pEngineMainFrame->m_pLogPane, -1, wxEmptyString,
                                         wxDefaultPosition, wxDefaultSize,
                                         wxTE_READONLY | wxNO_BORDER | wxTE_MULTILINE );
            pCustomLogTextCtrl->Bind( wxEVT_LEFT_DCLICK, &EngineMainFrame::OnTextCtrlLeftDoubleClick, g_pEngineMainFrame );
            g_pEngineMainFrame->m_pLogPane->AddPage( pCustomLogTextCtrl, tag );
        }

        // write the message to the custom box, error check shouldn't be needed.
        if( pCustomLogTextCtrl )
        {
            if( logtype == 1 )
                pCustomLogTextCtrl->AppendText( "ERROR: " );
            pCustomLogTextCtrl->AppendText( message );
        }
    }
}

void EngineMainFrame_DumpCachedMessagesToLogPane()
{
    pthread_mutex_lock( &g_MessageLogMutex );

    for( unsigned int i=0; i<g_LoggedMessages.size(); i++ )
    {
        EngineMainFrame_MessageLogForReal( g_LoggedMessages[i].logtype, g_LoggedMessages[i].tag, g_LoggedMessages[i].message );
    }
    g_LoggedMessages.clear();

    pthread_mutex_unlock( &g_MessageLogMutex );
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
    m_EditorCameraLayers = 0;

    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i] = 0;
        m_GameplayPerspectiveOptions[i] = 0;
    }

    for( int i=0; i<g_NumberOfVisibilityLayers; i++ )
    {
        m_EditorCameraLayerOptions[i] = 0;
    }

    m_Grid = 0;
    m_PlayPauseStop = 0;
    m_Data = 0;
    m_Hackery = 0;
    m_Debug = 0;

    m_MenuItem_GridVisible = 0;
    m_MenuItem_GridSnapEnabled = 0;
    m_MenuItem_ShowEditorIcons = 0;
    m_MenuItem_SelectedObjects_ShowWireframe = 0;
    m_MenuItem_SelectedObjects_ShowEffect = 0;
    m_MenuItem_Mode_SwitchFocusOnPlayStop = 0;

    m_MenuItem_Debug_DrawMousePickerFBO = 0;
    m_MenuItem_Debug_DrawSelectedAnimatedMesh = 0;
    m_MenuItem_Debug_DrawGLStats = 0;
    m_MenuItem_Debug_DrawPhysicsDebugShapes = 0;
    m_MenuItem_Debug_ShowProfilingInfo = 0;

    m_pEditorPrefs = 0;

    m_ShowEditorIcons = true;
    m_SelectedObjects_ShowWireframe = true;
    m_SelectedObjects_ShowEffect = true;
    m_Mode_SwitchFocusOnPlayStop = true;

    m_GridSettings.visible = true;
    m_GridSettings.snapenabled = false;
    m_GridSettings.stepsize.Set( 5, 5, 5 );
}

EngineMainFrame::~EngineMainFrame()
{
    SAFE_DELETE( m_pCommandStack );
    SAFE_DELETE( m_pGLCanvasEditor );

    for( int i=0; i<4; i++ )
    {
        if( g_SavedPerspectives[i] != g_DefaultPerspectives[i] )
            delete[] g_SavedPerspectives[i];
    }

    g_pMessageLogCallbackFunction = 0;

    pthread_mutex_destroy( &g_MessageLogMutex );
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
            // load any customized layouts.
            for( int i=0; i<4; i++ )
            {
                MyAssert( g_SavedPerspectives[i] == g_DefaultPerspectives[i] );

                char name[10];
                sprintf_s( name, 10, "Layout%d", i );
                cJSON* layout = cJSON_GetObjectItem( m_pEditorPrefs, name );
                if( layout )
                {
                    int len = strlen( layout->valuestring );
                    g_SavedPerspectives[i] = MyNew char[len+1];
                    strcpy_s( g_SavedPerspectives[i], len+1, layout->valuestring );
                }
            }

            int windowx = -9999;
            int windowy = -9999;
            int clientwidth = 0;
            int clientheight = 0;
            bool maximized = false;
            bool showicons = false;
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

    pthread_mutex_init( &g_MessageLogMutex, 0 );
    g_pMessageLogCallbackFunction = EngineMainFrame_MessageLog;

    m_File->Insert( 0, myIDEngine_NewScene, wxT("&New Scene") );
    m_File->Insert( 1, myIDEngine_LoadScene, wxT("&Load Scene...") );
    m_File->Insert( 2, myIDEngine_CreateAdditionalScene, wxT("&Create Additional Scene") );
    m_File->Insert( 3, myIDEngine_LoadAdditionalScene, wxT("&Load Additional Scene...") );
    m_File->Insert( 4, myIDEngine_SaveScene, wxT("&Save Scene\tCtrl-S") );
    m_File->Insert( 5, myIDEngine_SaveSceneAs, wxT("Save Scene &As...") );

    wxMenu* menuexport = MyNew wxMenu;
    m_File->Insert( 6, -1, "E&xport", menuexport );
    menuexport->Append( myIDEngine_ExportBox2DScene, wxT("Box2D Scene...\tCtrl-Shift-E") );

    m_Grid = MyNew wxMenu;
    m_MenuBar->Append( m_Grid, wxT("&Grid") );
    m_MenuItem_GridVisible = m_Grid->AppendCheckItem( myIDEngine_Grid_VisibleOnOff, wxT("Grid &On/Off\tCtrl-Shift-V") );
    m_MenuItem_GridSnapEnabled = m_Grid->AppendCheckItem( myIDEngine_Grid_SnapOnOff, wxT("Grid Snap &On/Off\tCtrl-G") );
    m_Grid->Append( myIDEngine_Grid_Settings, wxT("Grid &Settings\tCtrl-Shift-G") );

    m_PlayPauseStop = MyNew wxMenu;
    m_MenuBar->Append( m_PlayPauseStop, wxT("&Mode") );
    m_MenuItem_Mode_SwitchFocusOnPlayStop = m_PlayPauseStop->AppendCheckItem( myIDEngine_Mode_SwitchFocusOnPlayStop, wxT("Switch &Focus on Play/Stop") );    
    m_PlayPauseStop->Append( myIDEngine_Mode_PlayStop, wxT("&Play/Stop\tCtrl-SPACE") );
    m_PlayPauseStop->Append( myIDEngine_Mode_Pause, wxT("Pause\tCtrl-.") );
    m_PlayPauseStop->Append( myIDEngine_Mode_Advance1Frame, wxT("Advance 1 Frame\tCtrl-]") );
    m_PlayPauseStop->Append( myIDEngine_Mode_Advance1Second, wxT("Advance 1 Second\tCtrl-[") );
    m_PlayPauseStop->Append( myIDEngine_Mode_LaunchGame, wxT("&Launch Game\tCtrl-F5") );

    m_Data = MyNew wxMenu;
    m_MenuBar->Append( m_Data, wxT("&Data") );
    m_Data->Append( myIDEngine_AddDatafile, wxT("&Load Datafiles") );

    m_Hackery = MyNew wxMenu;
    m_MenuBar->Append( m_Hackery, wxT("&Hackery") );
    m_Hackery->Append( myIDEngine_RecordMacro, wxT("&Record\tCtrl-R") );
    m_Hackery->Append( myIDEngine_ExecuteMacro, wxT("Stop recording and &Execute\tCtrl-E") );

    m_Debug = MyNew wxMenu;
    m_MenuBar->Append( m_Debug, wxT("&Debug views") );
    m_MenuItem_Debug_DrawMousePickerFBO       = m_Debug->AppendCheckItem( myIDEngine_DebugShowMousePickerFBO, wxT("Show &Mouse Picker FBO\tF9") );
    m_MenuItem_Debug_DrawSelectedAnimatedMesh = m_Debug->AppendCheckItem( myIDEngine_DebugShowSelectedAnimatedMesh, wxT("Show &Animated Debug View for Selection\tF8") );
    m_MenuItem_Debug_DrawGLStats              = m_Debug->AppendCheckItem( myIDEngine_DebugShowGLStats, wxT("Show &GL Stats\tShift-F9") );
    m_MenuItem_Debug_DrawWireframe            = m_Debug->AppendCheckItem( myIDEngine_DebugDrawWireframe, wxT("Draw &Wireframe\tCtrl-F9") );
    m_MenuItem_Debug_DrawPhysicsDebugShapes   = m_Debug->AppendCheckItem( myIDEngine_DebugShowPhysicsShapes, wxT("Show &Physics debug shapes\tShift-F8") );
    m_MenuItem_Debug_ShowProfilingInfo        = m_Debug->AppendCheckItem( myIDEngine_DebugShowProfilingInfo, wxT("Show profiling &Info\tCtrl-F8") );

    m_Hackery_Record_StackDepth = -1;

    this->DragAcceptFiles( true );
    Connect( wxEVT_DROP_FILES, wxDropFilesEventHandler(EngineMainFrame::OnDropFiles) );

    // Override these menu options from the main frame,
    Connect( myID_View_SavePerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myID_View_LoadPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myID_View_ResetPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    m_EditorPerspectives = MyNew wxMenu;
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_EditorPerspectiveOptions[i] = m_EditorPerspectives->AppendCheckItem( myIDEngine_View_EditorPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
        Connect( myIDEngine_View_EditorPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    }

    m_GameplayPerspectives = MyNew wxMenu;
    for( int i=0; i<Perspective_NumPerspectives; i++ )
    {
        m_GameplayPerspectiveOptions[i] = m_GameplayPerspectives->AppendCheckItem( myIDEngine_View_GameplayPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
        Connect( myIDEngine_View_GameplayPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    }

    m_EditorPerspectiveOptions[0]->Check();
    m_GameplayPerspectiveOptions[0]->Check();

    m_EditorCameraLayers = MyNew wxMenu;
    for( int i=0; i<g_NumberOfVisibilityLayers; i++ )
    {
        wxString label;
        label << "(&" << i+1 << ") " << g_pVisibilityLayerStrings[i] << "\tCtrl-Alt-" << i+1;
        m_EditorCameraLayerOptions[i] = m_EditorCameraLayers->AppendCheckItem( myIDEngine_View_EditorCameraLayer + i, label, wxEmptyString );
        Connect( myIDEngine_View_EditorCameraLayer + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    }

    m_EditorCameraLayerOptions[0]->Check();

    m_View->Append( myIDEngine_View_EditorPerspectives, "Editor Layouts", m_EditorPerspectives );
    m_View->Append( myIDEngine_View_GameplayPerspectives, "Gameplay Layouts", m_GameplayPerspectives );

    m_MenuItem_ShowEditorIcons = m_View->AppendCheckItem( myIDEngine_View_ShowEditorIcons, wxT("Show &Editor Icons\tShift-F7") );

    wxMenu* pMenuSelectedObjects = MyNew wxMenu;
    m_View->AppendSubMenu( pMenuSelectedObjects, "Selected Objects" );
    m_MenuItem_SelectedObjects_ShowWireframe = pMenuSelectedObjects->AppendCheckItem( myIDEngine_View_SelectedObjects_ShowWireframe, wxT("Show &Wireframe") );
    m_MenuItem_SelectedObjects_ShowEffect = pMenuSelectedObjects->AppendCheckItem( myIDEngine_View_SelectedObjects_ShowEffect, wxT("Show &Effect") );
    
    m_View->Append( myIDEngine_View_EditorCameraLayers, "Editor &Camera Layers", m_EditorCameraLayers );

    Connect( myIDEngine_NewScene,               wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_LoadScene,              wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_CreateAdditionalScene,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_LoadAdditionalScene,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_SaveScene,              wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_SaveSceneAs,            wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_ExportBox2DScene,       wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    Connect( myIDEngine_AddDatafile,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    Connect( myIDEngine_Grid_VisibleOnOff, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Grid_SnapOnOff,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Grid_Settings,     wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    Connect( myIDEngine_Mode_SwitchFocusOnPlayStop, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Mode_PlayStop,              wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Mode_Pause,                 wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Mode_Advance1Frame,         wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Mode_Advance1Second,        wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_Mode_LaunchGame,            wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );    

    Connect( myIDEngine_RecordMacro,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_ExecuteMacro, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    Connect( myIDEngine_View_ShowEditorIcons,               wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_View_SelectedObjects_ShowWireframe, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_View_SelectedObjects_ShowEffect,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    Connect( myIDEngine_DebugShowMousePickerFBO,       wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_DebugShowSelectedAnimatedMesh, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_DebugShowGLStats,              wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_DebugDrawWireframe,            wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_DebugShowPhysicsShapes,        wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    Connect( myIDEngine_DebugShowProfilingInfo,        wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );

    if( m_pEditorPrefs )
    {
        int layouteditor = 0;
        int layoutgameplay = 0;
        cJSONExt_GetInt( m_pEditorPrefs, "EditorLayout", &layouteditor );
        cJSONExt_GetInt( m_pEditorPrefs, "GameplayLayout", &layoutgameplay );
        SetDefaultEditorPerspectiveIndex( layouteditor );
        SetDefaultGameplayPerspectiveIndex( layoutgameplay );
    }

    UpdateMenuItemStates();
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

    m_AUIManager.AddPane( m_pGLCanvasEditor, wxAuiPaneInfo().Name("GLCanvasEditor").Bottom().Caption("Editor") );//.CaptionVisible(false) );

    m_pLogPane = MyNew wxNotebook( this, wxID_ANY, wxPoint(0,0), wxDefaultSize );
    m_AUIManager.AddPane( m_pLogPane, wxAuiPaneInfo().Name("Log").Bottom().Caption("Log") );//.CaptionVisible(false) );

    for( int i=0; i<EngineEditorWindow_NumTypes; i++ )
    {
        m_EngineEditorWindowOptions[i] = m_EditorWindows->Append( myIDEngine_EditorWindow_FirstWindow + i, g_DefaultEngineEditorWindowTypeMenuLabels[i], wxEmptyString );
        Connect( myIDEngine_EditorWindow_FirstWindow + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
    }

    m_pLogMain = new wxTextCtrl( m_pLogPane, -1, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_READONLY | wxNO_BORDER | wxTE_MULTILINE );
    m_pLogMain->Bind( wxEVT_LEFT_DCLICK, &EngineMainFrame::OnTextCtrlLeftDoubleClick, this );
    m_pLogPane->AddPage( m_pLogMain, "Main Log" );

    m_pLogInfo = new wxTextCtrl( m_pLogPane, -1, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_READONLY | wxNO_BORDER | wxTE_MULTILINE );
    m_pLogInfo->Bind( wxEVT_LEFT_DCLICK, &EngineMainFrame::OnTextCtrlLeftDoubleClick, this );
    m_pLogPane->AddPage( m_pLogInfo, "Info" );

    m_pLogErrors = new wxTextCtrl( m_pLogPane, -1, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_READONLY | wxNO_BORDER | wxTE_MULTILINE );
    m_pLogErrors->Bind( wxEVT_LEFT_DCLICK, &EngineMainFrame::OnTextCtrlLeftDoubleClick, this );
    m_pLogPane->AddPage( m_pLogErrors, "Errors" );
}

bool EngineMainFrame::UpdateAUIManagerAndLoadPerspective()
{
    // try to load current perspective from layout.ini
    //if( MainFrame::UpdateAUIManagerAndLoadPerspective() )
    //    return true;

    // layout.ini file not found, use the default editor layout.
    int currentperspective = GetDefaultEditorPerspectiveIndex();
    m_AUIManager.LoadPerspective( g_SavedPerspectives[currentperspective] );

    // say a valid layout was set.
    return true;
}

void EngineMainFrame::OnPostInit()
{
    if( g_pEngineCore == 0 )
        return;

    MainFrame::OnPostInit();

    bool sceneloaded = false;

    if( m_pEditorPrefs )
    {
        cJSON* obj;

        obj = cJSON_GetObjectItem( m_pEditorPrefs, "EditorCam" );
        if( obj )
            g_pEngineCore->m_pEditorState->GetEditorCamera()->m_pComponentTransform->ImportFromJSONObject( obj, EngineCore::ENGINE_SCENE_ID );

        extern GLViewTypes g_CurrentGLViewType;
        cJSONExt_GetInt( m_pEditorPrefs, "GameAspectRatio", (int*)&g_CurrentGLViewType );

        cJSONExt_GetBool( m_pEditorPrefs, "ShowIcons", &m_ShowEditorIcons );
        cJSONExt_GetBool( m_pEditorPrefs, "SelectedObjects_ShowWireframe", &m_SelectedObjects_ShowWireframe );
        cJSONExt_GetBool( m_pEditorPrefs, "SelectedObjects_ShowEffect", &m_SelectedObjects_ShowEffect );
        cJSONExt_GetBool( m_pEditorPrefs, "Mode_SwitchFocusOnPlayStop", &m_Mode_SwitchFocusOnPlayStop );

        cJSONExt_GetBool( m_pEditorPrefs, "GridVisible", &m_GridSettings.visible );
        g_pEngineCore->SetGridVisible( m_GridSettings.visible );

        cJSONExt_GetBool( m_pEditorPrefs, "GridSnapEnabled", &m_GridSettings.snapenabled );
        cJSONExt_GetFloatArray( m_pEditorPrefs, "GridStepSize", &m_GridSettings.stepsize.x, 3 );

        cJSON* jGameObjectFlagsArray = cJSON_GetObjectItem( m_pEditorPrefs, "GameObjectFlags" );
        g_pEngineCore->InitializeGameObjectFlagStrings( jGameObjectFlagsArray );

        // Load the scene at the end.
        obj = cJSON_GetObjectItem( m_pEditorPrefs, "LastSceneLoaded" );
        if( obj && obj->valuestring[0] != 0 )
        {
            LoadScene( obj->valuestring, false ); // this is only parsed on startup, so no need to unload scene.
            sceneloaded = true;
        }

        cJSON_Delete( m_pEditorPrefs );
        m_pEditorPrefs = 0;
    }
    else
    {
        g_pEngineCore->InitializeGameObjectFlagStrings( 0 );
    }

    if( sceneloaded == false )
    {
        g_pComponentSystemManager->CreateNewScene( "Unsaved.scene", 1 );
        g_pEngineCore->CreateDefaultSceneObjects();
    }

    m_pGLCanvas->ResizeViewport();

    UpdateMenuItemStates();
}

bool EngineMainFrame::OnClose()
{
    int answer = wxID_YES;

    if( m_pCommandStack->GetUndoStackSize() != m_StackDepthAtLastSave )
    {
        //answer = wxMessageBox( "Some changes aren't saved.\nQuit anyway?", "Confirm", wxYES_NO, this );
        wxMessageDialog dlg( this, "Some changes aren't saved.\nQuit anyway?", "Confirm", wxYES_NO | wxNO_DEFAULT );
        dlg.SetYesNoLabels( "Quit/Lose changes", "Return to editor" );
        answer = dlg.ShowModal();
    }

    if( answer == wxID_YES )
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

            // Save Layout strings.
            for( int i=0; i<4; i++ )
            {
                if( g_SavedPerspectives[i] != g_DefaultPerspectives[i] )
                {
                    char name[10];
                    sprintf_s( name, 10, "Layout%d", i );
                    cJSON_AddStringToObject( pPrefs, name, g_SavedPerspectives[i] );
                }
            }

            cJSON_AddNumberToObject( pPrefs, "WindowX", m_WindowX );
            cJSON_AddNumberToObject( pPrefs, "WindowY", m_WindowY );
            cJSON_AddNumberToObject( pPrefs, "ClientWidth", m_ClientWidth );
            cJSON_AddNumberToObject( pPrefs, "ClientHeight", m_ClientHeight );
            cJSON_AddNumberToObject( pPrefs, "IsMaximized", m_Maximized );
            cJSON_AddNumberToObject( pPrefs, "ShowIcons", m_ShowEditorIcons );
            cJSON_AddNumberToObject( pPrefs, "SelectedObjects_ShowWireframe", m_SelectedObjects_ShowWireframe );
            cJSON_AddNumberToObject( pPrefs, "SelectedObjects_ShowEffect", m_SelectedObjects_ShowEffect );
            cJSON_AddNumberToObject( pPrefs, "Mode_SwitchFocusOnPlayStop", m_Mode_SwitchFocusOnPlayStop );

            const char* relativepath = GetRelativePath( g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath );
            if( relativepath )
                cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", relativepath );
            else
                cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath );
            cJSON_AddItemToObject( pPrefs, "EditorCam", g_pEngineCore->m_pEditorState->GetEditorCamera()->m_pComponentTransform->ExportAsJSONObject( false, true ) );
            cJSON_AddNumberToObject( pPrefs, "EditorLayout", GetDefaultEditorPerspectiveIndex() );
            cJSON_AddNumberToObject( pPrefs, "GameplayLayout", GetDefaultGameplayPerspectiveIndex() );
            extern GLViewTypes g_CurrentGLViewType;
            cJSON_AddNumberToObject( pPrefs, "GameAspectRatio", g_CurrentGLViewType );

            cJSON_AddNumberToObject( pPrefs, "GridVisible", m_GridSettings.visible );
            cJSON_AddNumberToObject( pPrefs, "GridSnapEnabled", m_GridSettings.snapenabled );
            cJSONExt_AddFloatArrayToObject( pPrefs, "GridStepSize", &m_GridSettings.stepsize.x, 3 );

            cJSON* jGameObjectFlagsArray = cJSON_CreateStringArray( (const char**)g_pEngineCore->m_GameObjectFlagStrings, 32 );
            cJSON_AddItemToObject( pPrefs, "GameObjectFlags", jGameObjectFlagsArray );

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

bool EngineMainFrame::FilterGlobalEvents(wxEvent& event)
{
    wxWindow* pWindowInFocus = FindFocus();

    // ignore global events if gameplay is in focus.
    if( pWindowInFocus == m_pGLCanvas )
        return false;

    //if( event.GetEventType() == wxEVT_KEY_DOWN )
    //{
    //    int wxkeycode = ((wxKeyEvent&)event).GetKeyCode();

    //    if( g_pEngineCore->HandleEditorInput( 1, GCBA_Down, wxkeycode, -1, -1, -1, -1, -1 ) )
    //        return true;
    //}

    //if( event.GetEventType() == wxEVT_KEY_UP )
    //{
    //    int wxkeycode = ((wxKeyEvent&)event).GetKeyCode();

    //    if( g_pEngineCore->HandleEditorInput( 1, GCBA_Up, wxkeycode, -1, -1, -1, -1, -1 ) )
    //        return true;
    //}

    return false;
}

void EngineMainFrame::OnGLCanvasShownOrHidden(bool shown)
{
    if( shown )
    {
        m_pGLCanvasEditor->m_TickGameCore = false;
    }
    else
    {
        m_pGLCanvasEditor->m_TickGameCore = true;
    }
}

void EngineMainFrame::OnDropFiles(wxDropFilesEvent& event)
{
    wxString* pFileArray = event.GetFiles();

    for( int i=0; i<event.GetNumberOfFiles(); i++ )
    {
        LoadDatafile( pFileArray[i] );
    }
}

void EngineMainFrame::ResizeViewport()
{
    unsigned int activecanvas = g_GLCanvasIDActive;

    MainFrame::ResizeViewport();
    m_pGLCanvasEditor->ResizeViewport();

    g_GLCanvasIDActive = activecanvas;
}

void EngineMainFrame::UpdateMenuItemStates()
{
    MainFrame::UpdateMenuItemStates();

    if( m_MenuItem_GridVisible )
        m_MenuItem_GridVisible->Check( m_GridSettings.visible );

    if( m_MenuItem_GridSnapEnabled )
        m_MenuItem_GridSnapEnabled->Check( m_GridSettings.snapenabled );

    if( m_MenuItem_ShowEditorIcons )
        m_MenuItem_ShowEditorIcons->Check( m_ShowEditorIcons );

    if( m_MenuItem_SelectedObjects_ShowWireframe )
        m_MenuItem_SelectedObjects_ShowWireframe->Check( m_SelectedObjects_ShowWireframe );

    if( m_MenuItem_SelectedObjects_ShowEffect )
        m_MenuItem_SelectedObjects_ShowEffect->Check( m_SelectedObjects_ShowEffect );

    if( m_MenuItem_Mode_SwitchFocusOnPlayStop )
        m_MenuItem_Mode_SwitchFocusOnPlayStop->Check( m_Mode_SwitchFocusOnPlayStop );

    if( g_pEngineCore )
    {
        if( m_MenuItem_Debug_DrawMousePickerFBO )
            m_MenuItem_Debug_DrawMousePickerFBO->Check( g_pEngineCore->m_Debug_DrawMousePickerFBO );

        if( m_MenuItem_Debug_DrawSelectedAnimatedMesh )
            m_MenuItem_Debug_DrawSelectedAnimatedMesh->Check( g_pEngineCore->m_Debug_DrawSelectedAnimatedMesh );

        if( m_MenuItem_Debug_DrawGLStats )
            m_MenuItem_Debug_DrawGLStats->Check( g_pEngineCore->m_Debug_DrawGLStats );

        if( m_MenuItem_Debug_DrawPhysicsDebugShapes )
            m_MenuItem_Debug_DrawPhysicsDebugShapes->Check( g_pEngineCore->m_Debug_DrawPhysicsDebugShapes );

        if( m_MenuItem_Debug_ShowProfilingInfo )
            m_MenuItem_Debug_ShowProfilingInfo->Check( g_pEngineCore->m_Debug_ShowProfilingInfo );
    }
}

void EngineMainFrame::ProcessAllGLCanvasInputEventQueues()
{
    MainFrame::ProcessAllGLCanvasInputEventQueues();

    m_pGLCanvasEditor->ProcessInputEventQueue();
}

void EngineMainFrame::OnMenu_Engine(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myIDEngine_NewScene:
        {
            int answer = wxID_YES;

            if( m_pCommandStack->GetUndoStackSize() != m_StackDepthAtLastSave )
            {
                //answer = wxMessageBox( "Some changes aren't saved.\nCreate a new scene?", "Confirm", wxYES_NO, this );
                wxMessageDialog dlg( g_pEngineMainFrame, "Some changes aren't saved.\nCreate a new scene?", "Confirm", wxYES_NO | wxNO_DEFAULT );
                dlg.SetYesNoLabels( "New scene/Lose changes", "Return to editor" );
                answer = dlg.ShowModal();
            }

            if( answer == wxID_YES )
            {
                this->SetTitle( "New scene" );
                g_pEngineCore->UnloadScene( UINT_MAX, false );
                g_pComponentSystemManager->CreateNewScene( "Unsaved.scene", 1 );
                g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath[0] = 0;
                g_pEngineCore->CreateDefaultSceneObjects();
                ResizeViewport();
            }
        }
        break;

    case myIDEngine_LoadScene:
        {
            int answer = wxID_YES;

            if( m_pCommandStack->GetUndoStackSize() != m_StackDepthAtLastSave )
            {
                //answer = wxMessageBox( "Some changes aren't saved.\nLoad anyway?", "Confirm", wxYES_NO, this );
                wxMessageDialog dlg( g_pEngineMainFrame, "Some changes aren't saved.\nLoad anyway?", "Confirm", wxYES_NO | wxNO_DEFAULT );
                dlg.SetYesNoLabels( "Load/Lose changes", "Return to editor" );
                answer = dlg.ShowModal();
            }

            if( answer == wxID_YES )
            {
                LoadSceneDialog( true );
                ResizeViewport();
            }
        }
        break;

    case myIDEngine_CreateAdditionalScene:
        {
            unsigned int sceneid = g_pComponentSystemManager->GetNextSceneID();
            g_pComponentSystemManager->CreateNewScene( "Unsaved.scene", sceneid );
            g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath[0] = 0;
        }
        break;

    case myIDEngine_LoadAdditionalScene:
        {
            LoadSceneDialog( false );
            ResizeViewport();
        }
        break;

    case myIDEngine_SaveScene:
        m_StackDepthAtLastSave = m_pCommandStack->GetUndoStackSize();
        g_pMaterialManager->SaveAllMaterials();
        g_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs();
        g_pGameCore->m_pSoundManager->SaveAllCues();
        SaveScene();
        break;

    case myIDEngine_SaveSceneAs:
        m_StackDepthAtLastSave = m_pCommandStack->GetUndoStackSize();
        g_pMaterialManager->SaveAllMaterials();
        g_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs();
        g_pGameCore->m_pSoundManager->SaveAllCues();
        SaveSceneAs( 1 );
        break;

    case myIDEngine_ExportBox2DScene:
        ExportBox2DScene( 1 );
        break;

    case myIDEngine_AddDatafile:
        AddDatafilesToScene();
        break;

    case myIDEngine_Grid_VisibleOnOff:
        m_GridSettings.visible = !m_GridSettings.visible;
        g_pEngineCore->SetGridVisible( m_GridSettings.visible );
        break;

    case myIDEngine_Grid_SnapOnOff:
        m_GridSettings.snapenabled = !m_GridSettings.snapenabled;
        break;

    case myIDEngine_Grid_Settings:
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

    case myIDEngine_Mode_SwitchFocusOnPlayStop:
        m_Mode_SwitchFocusOnPlayStop = !m_Mode_SwitchFocusOnPlayStop;
        break;

    case myIDEngine_Mode_PlayStop:
        g_pEngineCore->OnModeTogglePlayStop();
        break;

    case myIDEngine_Mode_Pause:
        g_pEngineCore->OnModePause();
        break;

    case myIDEngine_Mode_Advance1Frame:
        g_pEngineCore->OnModeAdvanceTime( 1/60.0f );
        break;

    case myIDEngine_Mode_Advance1Second:
        g_pEngineCore->OnModeAdvanceTime( 1.0f );
        break;

    case myIDEngine_Mode_LaunchGame:
        LaunchApplication( "MyEngine_Game.exe", g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath );
        break;

    case myIDEngine_RecordMacro:
        m_Hackery_Record_StackDepth = (int)m_pCommandStack->GetUndoStackSize();
        break;

    case myIDEngine_ExecuteMacro:
        if( m_Hackery_Record_StackDepth != -1 )
        {
            int topdepth = (int)m_pCommandStack->GetUndoStackSize();
            for( int i=m_Hackery_Record_StackDepth; i<topdepth; i++ )
            {
                // need to copy the command.
                EditorCommand* pCommandCopy = m_pCommandStack->GetUndoCommandAtIndex(i)->Repeat();
                if( pCommandCopy )
                    m_pCommandStack->Add( pCommandCopy );
            }
            m_Hackery_Record_StackDepth = topdepth;
        }
        break;

    case myID_View_SavePerspective:
        {
            int currentperspective = GetCurrentPerspectiveIndex();

            if( g_SavedPerspectives[currentperspective] != g_DefaultPerspectives[currentperspective] )
                delete[] g_SavedPerspectives[currentperspective];

            wxString wxPerspective = m_AUIManager.SavePerspective();
            int len = wxPerspective.Length();
            g_SavedPerspectives[currentperspective] = MyNew char[len+1];
            strcpy_s( g_SavedPerspectives[currentperspective], len+1, wxPerspective.c_str() );
        }
        break;

    case myID_View_LoadPerspective:
        {
            int currentperspective = GetCurrentPerspectiveIndex();
            m_AUIManager.LoadPerspective( g_SavedPerspectives[currentperspective] );
        }
        break;

    case myID_View_ResetPerspective:
        {
            int currentperspective = GetCurrentPerspectiveIndex();

            if( g_SavedPerspectives[currentperspective] != g_DefaultPerspectives[currentperspective] )
                delete[] g_SavedPerspectives[currentperspective];

            g_SavedPerspectives[currentperspective] = g_DefaultPerspectives[currentperspective];
            m_AUIManager.LoadPerspective( g_SavedPerspectives[currentperspective] );
        }
        break;

    case myIDEngine_View_EditorPerspective + 0:
    case myIDEngine_View_EditorPerspective + 1:
    case myIDEngine_View_EditorPerspective + 2:
    case myIDEngine_View_EditorPerspective + 3:
        SetDefaultEditorPerspectiveIndex( id - myIDEngine_View_EditorPerspective );
        if( g_pEngineCore->m_EditorMode == true )
            SetWindowPerspectiveToDefault( true );
        break;

    case myIDEngine_View_GameplayPerspective + 0:
    case myIDEngine_View_GameplayPerspective + 1:
    case myIDEngine_View_GameplayPerspective + 2:
    case myIDEngine_View_GameplayPerspective + 3:
        SetDefaultGameplayPerspectiveIndex( id - myIDEngine_View_GameplayPerspective );
        if( g_pEngineCore->m_EditorMode == false )
            SetWindowPerspectiveToDefault( true );
        break;

    case myIDEngine_View_EditorCameraLayer + 0:
    case myIDEngine_View_EditorCameraLayer + 1:
    case myIDEngine_View_EditorCameraLayer + 2:
    case myIDEngine_View_EditorCameraLayer + 3:
    case myIDEngine_View_EditorCameraLayer + 4:
    case myIDEngine_View_EditorCameraLayer + 5:
    case myIDEngine_View_EditorCameraLayer + 6:
    case myIDEngine_View_EditorCameraLayer + 7:
        {
            int layerindex = id - myIDEngine_View_EditorCameraLayer;
            if( m_EditorCameraLayerOptions[layerindex]->IsChecked() )
                g_pEngineCore->m_pEditorState->GetEditorCamera()->m_LayersToRender |= (1 << layerindex);
            else
                g_pEngineCore->m_pEditorState->GetEditorCamera()->m_LayersToRender &= ~(1 << layerindex);
        }
        break;

    case myIDEngine_View_ShowEditorIcons:
        m_ShowEditorIcons = !m_ShowEditorIcons;
        break;

    case myIDEngine_View_SelectedObjects_ShowWireframe:
        m_SelectedObjects_ShowWireframe = !m_SelectedObjects_ShowWireframe;
        break;

    case myIDEngine_View_SelectedObjects_ShowEffect:
        m_SelectedObjects_ShowEffect = !m_SelectedObjects_ShowEffect;
        break;

    case myIDEngine_EditorWindow_Editor:
        {
            wxAuiPaneInfo& paneinfo = m_AUIManager.GetPane( m_pGLCanvasEditor );
            paneinfo.Show( !paneinfo.IsShown() );
            m_AUIManager.Update();
        }
        break;

    case myIDEngine_EditorWindow_LogPane:
        {
            wxAuiPaneInfo& paneinfo = m_AUIManager.GetPane( m_pLogPane );
            paneinfo.Show( !paneinfo.IsShown() );
            m_AUIManager.Update();
        }
        break;

    case myIDEngine_DebugShowMousePickerFBO:
        g_pEngineCore->m_Debug_DrawMousePickerFBO = !g_pEngineCore->m_Debug_DrawMousePickerFBO;
        break;

    case myIDEngine_DebugShowSelectedAnimatedMesh:
        g_pEngineCore->m_Debug_DrawSelectedAnimatedMesh = !g_pEngineCore->m_Debug_DrawSelectedAnimatedMesh;
        break;

    case myIDEngine_DebugShowGLStats:
        g_pEngineCore->m_Debug_DrawGLStats = !g_pEngineCore->m_Debug_DrawGLStats;
        break;

    case myIDEngine_DebugDrawWireframe:
        g_pEngineCore->m_Debug_DrawWireframe = !g_pEngineCore->m_Debug_DrawWireframe;
        break;

    case myIDEngine_DebugShowPhysicsShapes:
        g_pEngineCore->m_Debug_DrawPhysicsDebugShapes = !g_pEngineCore->m_Debug_DrawPhysicsDebugShapes;
        break;

    case myIDEngine_DebugShowProfilingInfo:
        g_pEngineCore->m_Debug_ShowProfilingInfo = !g_pEngineCore->m_Debug_ShowProfilingInfo;
        break;
    }
}

void EngineMainFrame::SetWindowPerspectiveToDefault(bool forceswitch)
{
    // change menubar color scheme when in gameplay mode
    {
        wxColour bgcolour = m_MenuBar->GetBackgroundColour();
        if( g_pEngineCore->m_EditorMode )
        {
            //m_pGLCanvasEditor->SetBackgroundColour( wxColour( 240, 240, 240, 255 ) );
            //m_pGLCanvasEditor->Refresh();
            this->SetBackgroundColour( wxColour( 240, 240, 240, 255 ) );
            this->Refresh();
            //m_MenuBar->SetBackgroundColour( wxColour( 240, 240, 240, 255 ) );
            //m_MenuBar->Refresh();            
        }
        else
        {
            //m_pGLCanvasEditor
            //m_pGLCanvasEditor->SetBackgroundColour( wxColour( 50, 50, 50, 255 ) );
            //m_pGLCanvasEditor->Refresh();
            this->SetBackgroundColour( wxColour( 150, 150, 150, 255 ) );
            this->Refresh();
            //m_MenuBar->SetBackgroundColour( wxColour( 50, 50, 50, 255 ) );
            //m_MenuBar->Refresh();
        }
    }

    // change the window layout
    int editor = GetDefaultEditorPerspectiveIndex();
    int gameplay = GetDefaultGameplayPerspectiveIndex();

    // don't touch anything if both perspectives are the same.
    if( forceswitch == false && editor == gameplay )
        return;

    int currentperspective = GetCurrentPerspectiveIndex();

    if( currentperspective != -1 )
        m_AUIManager.LoadPerspective( g_SavedPerspectives[currentperspective] );
}

int EngineMainFrame::GetCurrentPerspectiveIndex()
{
    int currentperspective;
    int editor = GetDefaultEditorPerspectiveIndex();
    int gameplay = GetDefaultGameplayPerspectiveIndex();

    if( g_pEngineCore->m_EditorMode )
    {
        currentperspective = editor;
    }
    else
    {
        currentperspective = gameplay;
    }

    return currentperspective;
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
    if( g_pEngineCore->m_EditorMode == false )
    {
        LOGInfo( LOGTag, "Can't save when gameplay is active... use \"Save As\"\n" );
    }
    else
    {
        //typedef std::map<int, SceneInfo>::iterator it_type;
        //for( it_type iterator = g_pComponentSystemManager->m_pSceneInfoMap.begin(); iterator != g_pComponentSystemManager->m_pSceneInfoMap.end(); iterator++ )
        //{
        //    unsigned int sceneid = iterator->first;
        //    SceneInfo* pSceneInfo = &iterator->second;
        for( unsigned int i=0; i<ComponentSystemManager::MAX_SCENES_LOADED; i++ )
        {
            if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                continue;

            unsigned int sceneid = i;
            SceneInfo* pSceneInfo = &g_pComponentSystemManager->m_pSceneInfoMap[i];

            if( sceneid != 0 && sceneid != EngineCore::ENGINE_SCENE_ID )
            {
                if( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath[0] == 0 )
                {
                    SaveSceneAs( sceneid );
                }
                else
                {
                    LOGInfo( LOGTag, "Saving scene... %s\n", pSceneInfo->m_FullPath );
                    g_pEngineCore->SaveScene( pSceneInfo->m_FullPath, sceneid );
                }
            }
        }
    }
}

void EngineMainFrame::SaveSceneAs(unsigned int sceneid)
{
    wxFileDialog FileDialog( this, _("Save Scene file"), "./Data/Scenes", "", "Scene files (*.scene)|*.scene", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // save the current scene
    // TODO: typecasting will likely cause issues with multibyte names
    wxString wxpath = FileDialog.GetPath();
    g_pComponentSystemManager->GetSceneInfo( sceneid )->ChangePath( (const char*)wxpath );
    //sprintf_s( g_pComponentSystemManager->GetSceneInfo( sceneid )->fullpath, 260, "%s", (const char*)wxpath );

    g_pMaterialManager->SaveAllMaterials();
    g_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs();
    g_pGameCore->m_pSoundManager->SaveAllCues();
    g_pEngineCore->SaveScene( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath, sceneid );

    this->SetTitle( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath );
}

void EngineMainFrame::ExportBox2DScene(unsigned int sceneid)
{
    wxFileDialog FileDialog( this, _("Export Box2D Scene file"), "", "", "Box2D Scene files (*.box2dscene)|*.box2dscene", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    wxString wxpath = FileDialog.GetPath();

    g_pEngineCore->ExportBox2DScene( wxpath, sceneid );
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
    if( g_pEngineCore == 0 )
        return;

    MyAssert( scenename != 0 );

    // clear out the old scene before loading.
    if( unloadscenes )
    {
        // Make sure gameplay is stopped before loading
        g_pEngineCore->OnModeStop();

        // Reset the scene counter, so the new "first" scene loaded will be 1.
        g_pComponentSystemManager->ResetSceneIDCounter();

        // if we're unloading the old scene(s), clear all selected items.
        g_pEngineCore->m_pEditorState->ClearKeyAndActionStates();
        g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );

        g_pEngineCore->UnloadScene( UINT_MAX, false ); // don't unload editor objects.
    }

    // Load the scene from file.
    // This might cause some "undo" actions, so wipe them out once the load is complete.
    unsigned int numItemsInUndoStack = g_pEngineMainFrame->m_pCommandStack->GetUndoStackSize();
    unsigned int sceneid = g_pEngineCore->LoadSceneFromFile( scenename );
    g_pEngineMainFrame->m_pCommandStack->ClearUndoStack( numItemsInUndoStack );

    this->SetTitle( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath );
}

void EngineMainFrame::AddDatafilesToScene()
{
    // multiple select file open dialog
    wxFileDialog FileDialog( this, _("Open Datafile"), "./Data", "", "All files(*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE );
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // load the files chosen by the user
    // TODO: typecasting will likely cause issues with multibyte names
    wxArrayString patharray;
    FileDialog.GetPaths( patharray );

    for( unsigned int filenum=0; filenum<patharray.Count(); filenum++ )
    {
        LoadDatafile( patharray[filenum] );
    }
}

void EngineMainFrame::LoadDatafile(wxString filename)
{
    char fullpath[MAX_PATH];

    sprintf_s( fullpath, MAX_PATH, "%s", (const char*)filename );

    // if the datafile is in our working directory, then load it... otherwise TODO: copy it in?
    const char* relativepath = GetRelativePath( fullpath );
    if( relativepath == 0 )
    {
        // File is not in our working directory.
        // TODO: copy the file into our data folder?
        LOGError( LOGTag, "file must be in working directory\n" );
        //MyAssert( false );
        return;
    }

    g_pEngineCore->m_pComponentSystemManager->LoadDataFile( relativepath, 1, filename, true );
}

void EngineMainFrame::OnDrop(int controlid, wxCoord x, wxCoord y)
{
    // get the GameObject the mouse was hovering over.
    ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();
    y = pCamera->m_WindowHeight - y; // prefer 0,0 at bottom left.
    GameObject* pObjectDroppedOn = g_pEngineCore->GetCurrentEditorInterface()->GetObjectAtPixel( x, y, true, false );

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;

        if( pMaterial && pObjectDroppedOn )
        {
            pObjectDroppedOn->Editor_SetMaterial( pMaterial );
            g_pPanelWatch->SetNeedsRefresh();
        }
    }

    if( pDropItem->m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        TextureDefinition* pTexture = (TextureDefinition*)pDropItem->m_Value;

        if( pTexture && pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeTextureOnMaterial( pObjectDroppedOn->GetMaterial(), pTexture ) );
        }
    }

    if( pDropItem->m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShader = (ShaderGroup*)pDropItem->m_Value;

        if( pShader && pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeShaderOnMaterial( pObjectDroppedOn->GetMaterial(), pShader ) );
        }
    }

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)pDropItem->m_Value;
        MyAssert( pFile );

        if( pFile && strcmp( pFile->GetExtensionWithDot(), ".lua" ) == 0 )
        {
            if( pObjectDroppedOn )
            {
                g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeAllScriptsOnGameObject( pObjectDroppedOn, pFile ) );
            }
        }

        if( pFile && strcmp( pFile->GetExtensionWithDot(), ".glsl" ) == 0 )
        {
            if( pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
            {
                ShaderGroup* pShader = g_pShaderGroupManager->FindShaderGroupByFile( pFile );
                g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeShaderOnMaterial( pObjectDroppedOn->GetMaterial(), pShader ) );
            }
        }

        if( pFile &&
            ( strcmp( pFile->GetExtensionWithDot(), ".obj" ) == 0 || strcmp( pFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
          )
        {
            // Create a new gameobject using this obj.
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );

            GameObject* pGameObject = g_pComponentSystemManager->CreateGameObject( true, 1 );
            pGameObject->SetName( "New mesh" );
            ComponentMeshOBJ* pComponentMeshOBJ = (ComponentMeshOBJ*)pGameObject->AddNewComponent( ComponentType_MeshOBJ, 1 );
            pComponentMeshOBJ->SetSceneID( 1 );
            pComponentMeshOBJ->SetMaterial( g_pMaterialManager->GetFirstMaterial(), 0 );
            pComponentMeshOBJ->SetMesh( pMesh );
            pComponentMeshOBJ->SetLayersThisExistsOn( Layer_MainScene );

            if( pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
            {
                // Place it just above of the object selected otherwise place it at 0,0,0... for now.
                Vector3 pos = pObjectDroppedOn->GetTransform()->GetWorldPosition();

                ComponentRenderable* pComponentMeshDroppedOn = (ComponentRenderable*)pObjectDroppedOn->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
                if( pComponentMeshDroppedOn )
                {
                    pos.y += pComponentMeshDroppedOn->GetBounds()->GetHalfSize().y;
                    pos.y += pMesh->GetBounds()->GetHalfSize().y;
                }

                pGameObject->GetTransform()->SetWorldPosition( pos );
                pGameObject->GetTransform()->UpdateTransform();
            }

            // Undo/redo
            g_pEngineMainFrame->m_pCommandStack->Add( MyNew EditorCommand_CreateGameObject( pGameObject ) );
        }
    }

    if( pDropItem->m_Type == DragAndDropTypeEngine_Prefab )
    {
        PrefabObject* pPrefab = (PrefabObject*)pDropItem->m_Value;

        // Default to drop into scene 1, but prefer putting in same scene as the object dropped on.
        unsigned int sceneid = 1;
        if( pObjectDroppedOn )
        {
            sceneid = pObjectDroppedOn->GetSceneID();
        }

        // Create the game object
        GameObject* pGameObjectCreated = g_pComponentSystemManager->CreateGameObjectFromPrefab( pPrefab, true, sceneid );

        if( pGameObjectCreated )
        {
            // Undo/Redo
            g_pEngineMainFrame->m_pCommandStack->Add( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );

            // Select the object dropped
            g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();
            g_pEngineCore->m_pEditorState->SelectGameObject( pGameObjectCreated );

            // Move the new object to the same spot as the one it was dropped on
            if( pObjectDroppedOn )
            {
                std::vector<GameObject*> selectedobjects;
                selectedobjects.push_back( pGameObjectCreated );
                Vector3 worldpos = pObjectDroppedOn->GetTransform()->GetWorldPosition();
                g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_MoveObjects( worldpos, selectedobjects ), true );
            }
        }
    }

    //if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    //{
    //    GameObject* pGameObject = (GameObject*)pDropItem->m_Value;
    //    MyAssert( pGameObject );

    //    int id = g_DragAndDropStruct.GetControlID() - m_ControlIDOfFirstExtern;
    //    
    //    // TODO: this will make a mess of memory if different types of objects can be dragged in...
    //    m_ExposedVars[id]->pointer = pGameObject;

    //    // update the panel so new gameobject name shows up.
    //    g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.GetControlID() )->m_Description = pGameObject->GetName();
    //}
}

// Internal event handling functions
void EngineMainFrame::OnTextCtrlLeftDoubleClick(wxMouseEvent& evt)
{
    wxTextCtrl* pTextCtrl = (wxTextCtrl*)evt.GetEventObject();

    wxPoint pos = evt.GetPosition();

    wxTextCoord col, row;
    pTextCtrl->HitTest( pos, &col, &row );

    wxString line = pTextCtrl->GetLineText( row );

    // Parse the line and select the gameobject/material.
    char scenename[100];
    char gameobjectname[100];
    char componentname[100];

    int num = sscanf( line.c_str(), " (GameObject) %99[^:] :: %99[^:] :: %99s", scenename, gameobjectname, componentname );
    if( num == 3 )
    {
        // Remove the space from the end of the scene/gameobject name.
        int scenenamelen = strlen(scenename);
        if( scenenamelen > 1 )
        {
            if( scenename[scenenamelen-1] == ' ' )
                scenename[scenenamelen-1] = 0;
        }

        int gameobjectnamelen = strlen(gameobjectname);
        if( gameobjectnamelen > 1 )
        {
            if( gameobjectname[gameobjectnamelen-1] == ' ' )
                gameobjectname[gameobjectnamelen-1] = 0;
        }

        int sceneid = g_pComponentSystemManager->FindSceneID( scenename );

        if( sceneid != UINT_MAX )
        {
            GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByNameInScene( sceneid, gameobjectname );

            if( pGameObject )
            {
                g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();
                g_pEngineCore->m_pEditorState->SelectGameObject( pGameObject );
            }
        }
    }
}

#endif //MYFW_USING_WX
