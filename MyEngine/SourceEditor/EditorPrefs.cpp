//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorPrefs.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "Core/EngineCore.h"
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Editor_ImGui/EditorLayoutManager_ImGui.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#include "../SourceEditor/Editor_ImGui/ImGuiStylePrefs.h"

EditorPrefs::EditorPrefs()
{
    m_pSaveFile = nullptr;

    m_jEditorPrefs = nullptr;

    // Set default values for prefs.
    m_WindowX = 0;
    m_WindowY = 0;
    m_WindowWidth = 1280;
    m_WindowHeight = 600;
    m_IsWindowMaximized = false;

    m_View_ShowEditorIcons = true;
    m_View_EditorCamDeferred = false;
    m_View_SelectedObjects_ShowWireframe = true;
    m_View_SelectedObjects_ShowEffect = true;

    m_Aspect_CurrentGameWindowAspectRatio = GLView_Full;

    m_GridSettings.visible = true;
    m_GridSettings.snapEnabled = false;
    m_GridSettings.stepSize.Set( 1, 1, 1 );

    m_Mode_SwitchFocusOnPlayStop = true;
#if MYFW_WINDOWS
    m_Mode_CurrentLaunchPlatform = LaunchPlatform_Win32;
#else
    m_Mode_CurrentLaunchPlatform = LaunchPlatform_OSX;
#endif

    m_Debug_DrawPhysicsDebugShapes = true;

    m_pImGuiStylePrefs = MyNew( ImGuiStylePrefs );
}

EditorPrefs::~EditorPrefs()
{
    MyAssert( m_pSaveFile == nullptr );

    cJSON_Delete( m_jEditorPrefs );

#if MYFW_USING_IMGUI
    delete m_pImGuiStylePrefs;
#endif
}

void EditorPrefs::Init()
{
    const char* iniFilename = "EditorPrefs.ini";

    const char* string = PlatformSpecific_LoadFile( iniFilename );

    m_jEditorPrefs = cJSON_Parse( string );

    delete[] string;
}

void EditorPrefs::LoadWindowSizePrefs()
{
    if( m_jEditorPrefs == nullptr )
        return;

#if MYFW_USING_IMGUI
    // Load in some prefs.
    cJSONExt_GetInt( m_jEditorPrefs, "WindowX", &m_WindowX );
    cJSONExt_GetInt( m_jEditorPrefs, "WindowY", &m_WindowY );
    cJSONExt_GetInt( m_jEditorPrefs, "WindowWidth", &m_WindowWidth );
    cJSONExt_GetInt( m_jEditorPrefs, "WindowHeight", &m_WindowHeight );
    cJSONExt_GetBool( m_jEditorPrefs, "IsMaximized", &m_IsWindowMaximized );

    // Resize window.
    SetWindowPos( g_hWnd, nullptr, m_WindowX, m_WindowY, m_WindowWidth, m_WindowHeight, 0 );
    if( m_IsWindowMaximized )
    {
        ShowWindow( g_hWnd, SW_MAXIMIZE );
    }
#endif //MYFW_USING_IMGUI
}

void EditorPrefs::LoadPrefs()
{
    if( m_jEditorPrefs == nullptr )
        return;

    cJSON* jObject;

    if( g_pEngineCore )
    {
        jObject = cJSON_GetObjectItem( m_jEditorPrefs, "EditorCam" );
        if( jObject )
            g_pEngineCore->GetEditorState()->GetEditorCamera()->m_pComponentTransform->ImportFromJSONObject( jObject, SCENEID_EngineObjects );
    }

    // 2D Animation Editor.
    jObject = cJSON_GetObjectItem( m_jEditorPrefs, "2DAnimInfoBeingEdited" );
    if( jObject )
    {
        g_pEngineCore->GetEditorMainFrame_ImGui()->SetFullPathToLast2DAnimInfoBeingEdited( jObject->valuestring );
    }

    // File menu options.
    cJSON* jRecentScenesArray = cJSON_GetObjectItem( m_jEditorPrefs, "File_RecentScenes" );
    if( jRecentScenesArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jRecentScenesArray ); i++ )
        {
            cJSON* jScene = cJSON_GetArrayItem( jRecentScenesArray, i );
            m_File_RecentScenes.push_back( jScene->valuestring );
        }
    }

    // Document menu options.
    cJSON* jRecentDocumentsArray = cJSON_GetObjectItem( m_jEditorPrefs, "Document_RecentDocuments" );
    if( jRecentDocumentsArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jRecentDocumentsArray ); i++ )
        {
            cJSON* jDocument = cJSON_GetArrayItem( jRecentDocumentsArray, i );
            m_Document_RecentDocuments.push_back( jDocument->valuestring );
        }
    }

    // View menu options.
    cJSONExt_GetBool( m_jEditorPrefs, "View_ShowEditorIcons", &m_View_ShowEditorIcons );
    cJSONExt_GetBool( m_jEditorPrefs, "View_EditorCamDeferred", &m_View_EditorCamDeferred );
    cJSONExt_GetBool( m_jEditorPrefs, "View_SelectedObjects_ShowWireframe", &m_View_SelectedObjects_ShowWireframe );
    cJSONExt_GetBool( m_jEditorPrefs, "View_SelectedObjects_ShowEffect", &m_View_SelectedObjects_ShowEffect );

    // Aspect menu options.
    cJSONExt_GetInt( m_jEditorPrefs, "Aspect_GameAspectRatio", (int*)&m_Aspect_CurrentGameWindowAspectRatio );

    // Grid menu options.
    cJSONExt_GetBool( m_jEditorPrefs, "Grid_Visible", &m_GridSettings.visible );
    if( g_pEngineCore )
    {
        g_pEngineCore->SetGridVisible( m_GridSettings.visible );
    }
    cJSONExt_GetBool( m_jEditorPrefs, "Grid_SnapEnabled", &m_GridSettings.snapEnabled );
    cJSONExt_GetFloatArray( m_jEditorPrefs, "Grid_StepSize", &m_GridSettings.stepSize.x, 3 );

    // Mode menu options.
    cJSONExt_GetBool( m_jEditorPrefs, "Mode_SwitchFocusOnPlayStop", &m_Mode_SwitchFocusOnPlayStop );
    cJSONExt_GetUnsignedInt( m_jEditorPrefs, "LaunchPlatform", (unsigned int*)&m_Mode_CurrentLaunchPlatform );

    // Debug menu options.
    cJSONExt_GetBool( m_jEditorPrefs, "Debug_DrawPhysicsDebugShapes", &m_Debug_DrawPhysicsDebugShapes );

    // Lua script menu options.
    cJSON* jRecentFilesArray = cJSON_GetObjectItem( m_jEditorPrefs, "Lua_RecentFiles" );
    if( jRecentFilesArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jRecentFilesArray ); i++ )
        {
            cJSON* jFile = cJSON_GetArrayItem( jRecentFilesArray, i );
            m_Lua_RecentScripts.push_back( jFile->valuestring );
        }
    }

#if MYFW_USING_IMGUI
    m_pImGuiStylePrefs->LoadPrefs( m_jEditorPrefs );
    g_pEngineCore->GetEditorMainFrame_ImGui()->GetLayoutManager()->LoadPrefs( m_jEditorPrefs );
#endif
}

void EditorPrefs::LoadLastSceneLoaded()
{
    bool sceneWasLoaded = false;

    // Load the scene at the end.
    if( m_jEditorPrefs != nullptr )
    {
        cJSON* jObject = cJSON_GetObjectItem( m_jEditorPrefs, "LastSceneLoaded" );
        if( jObject && jObject->valuestring[0] != '\0' )
        {
            char* sceneName = jObject->valuestring;

            if( g_pEngineCore == nullptr )
                return;

            // Load the scene from file.
            // This might cause some "undo" actions, so wipe them out once the load is complete.
            unsigned int numItemsInUndoStack = g_pEngineCore->GetCommandStack()->GetUndoStackSize();

            char fullpath[MAX_PATH];
            GetFullPath( sceneName, fullpath, MAX_PATH );
            SceneID sceneid = g_pEngineCore->LoadSceneFromFile( fullpath );

            sceneWasLoaded = true;

            g_pEngineCore->GetCommandStack()->ClearUndoStack( numItemsInUndoStack );

            //this->SetTitle( scenename ); //g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath );
        }
    }

    if( sceneWasLoaded == false )
    {
        g_pEngineCore->GetEditorState()->ClearKeyAndActionStates();
        g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
        g_pComponentSystemManager->CreateNewScene( "Unsaved.scene", SCENEID_MainScene );
        g_pEngineCore->CreateDefaultSceneObjects();
    }
}

cJSON* EditorPrefs::SaveStart()
{
    MyAssert( m_pSaveFile == nullptr );

#if MYFW_WINDOWS
    fopen_s( &m_pSaveFile, "EditorPrefs.ini", "wb" );
#else
    m_pSaveFile = fopen( "EditorPrefs.ini", "wb" );
#endif
    if( m_pSaveFile )
    {
        cJSON* jPrefs = cJSON_CreateObject();

#if MYFW_USING_IMGUI
        WINDOWINFO info;
        if( GetWindowInfo( g_hWnd, &info ) )
        {
            m_WindowX = info.rcWindow.left;
            m_WindowY = info.rcWindow.top;

            m_WindowWidth = info.rcWindow.right - info.rcWindow.left;
            m_WindowHeight = info.rcWindow.bottom - info.rcWindow.top;

            if( info.dwStyle & WS_MAXIMIZE )
                m_IsWindowMaximized = true;
            else
                m_IsWindowMaximized = false;
        }
#endif //MYFW_USING_IMGUI

        cJSON_AddNumberToObject( jPrefs, "WindowX", m_WindowX );
        cJSON_AddNumberToObject( jPrefs, "WindowY", m_WindowY );
        cJSON_AddNumberToObject( jPrefs, "WindowWidth", m_WindowWidth );
        cJSON_AddNumberToObject( jPrefs, "WindowHeight", m_WindowHeight );
        cJSON_AddNumberToObject( jPrefs, "IsMaximized", m_IsWindowMaximized );

        const char* relativePath = GetRelativePath( g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->m_FullPath );
        if( relativePath )
            cJSON_AddStringToObject( jPrefs, "LastSceneLoaded", relativePath );
        else
            cJSON_AddStringToObject( jPrefs, "LastSceneLoaded", g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->m_FullPath );

        cJSON_AddItemToObject( jPrefs, "EditorCam", g_pEngineCore->GetEditorState()->GetEditorCamera()->m_pComponentTransform->ExportAsJSONObject( false, true ) );

        // 2D Animation Editor.
        My2DAnimInfo* pAnimInfo = g_pEngineCore->GetEditorMainFrame_ImGui()->Get2DAnimInfoBeingEdited();
        if( pAnimInfo && pAnimInfo->GetSourceFile() && pAnimInfo->GetSourceFile()->GetFullPath() )
        {
            cJSON_AddStringToObject( jPrefs, "2DAnimInfoBeingEdited", pAnimInfo->GetSourceFile()->GetFullPath() );
        }

        //cJSON* jGameObjectFlagsArray = cJSON_CreateStringArray( g_pEngineCore->GetGameObjectFlagStringArray(), 32 );
        //cJSON_AddItemToObject( pPrefs, "GameObjectFlags", jGameObjectFlagsArray );

        // File menu options.
        cJSON* jRecentScenesArray = cJSON_CreateArray();
        for( unsigned int i=0; i<m_File_RecentScenes.size(); i++ )
        {
            cJSON* jScene = cJSON_CreateString( m_File_RecentScenes[i].c_str() );
            cJSON_AddItemToArray( jRecentScenesArray, jScene );
        }
        cJSON_AddItemToObject( jPrefs, "File_RecentScenes", jRecentScenesArray );

        // Document menu options.
        cJSON* jRecentDocumentsArray = cJSON_CreateArray();
        for( unsigned int i=0; i<m_Document_RecentDocuments.size(); i++ )
        {
            cJSON* jDocument = cJSON_CreateString( m_Document_RecentDocuments[i].c_str() );
            cJSON_AddItemToArray( jRecentDocumentsArray, jDocument );
        }
        cJSON_AddItemToObject( jPrefs, "Document_RecentDocuments", jRecentDocumentsArray );

        // View menu options
        //cJSON_AddNumberToObject( pPrefs, "EditorLayout", GetDefaultEditorPerspectiveIndex() );
        //cJSON_AddNumberToObject( pPrefs, "GameplayLayout", GetDefaultGameplayPerspectiveIndex() );
        //extern GLViewTypes g_CurrentGLViewType;
        cJSON_AddNumberToObject( jPrefs, "View_ShowEditorIcons", m_View_ShowEditorIcons );
        cJSON_AddNumberToObject( jPrefs, "View_EditorCamDeferred", m_View_EditorCamDeferred );
        cJSON_AddNumberToObject( jPrefs, "View_SelectedObjects_ShowWireframe", m_View_SelectedObjects_ShowWireframe );
        cJSON_AddNumberToObject( jPrefs, "View_SelectedObjects_ShowEffect", m_View_SelectedObjects_ShowEffect );

        // Aspect menu options.
        cJSON_AddNumberToObject( jPrefs, "Aspect_GameAspectRatio", m_Aspect_CurrentGameWindowAspectRatio );

        // Grid menu options.
        cJSON_AddNumberToObject( jPrefs, "Grid_Visible", m_GridSettings.visible );
        cJSON_AddNumberToObject( jPrefs, "Grid_SnapEnabled", m_GridSettings.snapEnabled );
        cJSONExt_AddFloatArrayToObject( jPrefs, "Grid_StepSize", &m_GridSettings.stepSize.x, 3 );

        // Mode menu options.
        cJSON_AddNumberToObject( jPrefs, "Mode_SwitchFocusOnPlayStop", m_Mode_SwitchFocusOnPlayStop );
        cJSON_AddNumberToObject( jPrefs, "LaunchPlatform", m_Mode_CurrentLaunchPlatform );

        // Debug menu options.
        cJSON_AddNumberToObject( jPrefs, "Debug_DrawPhysicsDebugShapes", m_Debug_DrawPhysicsDebugShapes );

        // Lua script menu options.
        cJSON* jRecentFilesArray = cJSON_CreateArray();
        for( unsigned int i=0; i<m_Lua_RecentScripts.size(); i++ )
        {
            cJSON* jFile = cJSON_CreateString( m_Lua_RecentScripts[i].c_str() );
            cJSON_AddItemToArray( jRecentFilesArray, jFile );
        }
        cJSON_AddItemToObject( jPrefs, "Lua_RecentFiles", jRecentFilesArray );

#if MYFW_USING_IMGUI
        m_pImGuiStylePrefs->SavePrefs( jPrefs );
        g_pEngineCore->GetEditorMainFrame_ImGui()->GetLayoutManager()->SavePrefs( jPrefs );
#endif

        return jPrefs;
    }

    return nullptr;
}

void EditorPrefs::SaveFinish(cJSON* jPrefs)
{
    MyAssert( m_pSaveFile != nullptr );
    MyAssert( jPrefs != nullptr );

    char* string = cJSON_Print( jPrefs );
    cJSON_Delete( jPrefs );

    fprintf( m_pSaveFile, "%s", string );
    fclose( m_pSaveFile );
    m_pSaveFile = nullptr;

    cJSONExt_free( string );
}

void EditorPrefs::Toggle_Grid_Visible()
{
    m_GridSettings.visible = !m_GridSettings.visible;
    g_pEngineCore->SetGridVisible( m_GridSettings.visible );
}

void EditorPrefs::Toggle_Grid_SnapEnabled()
{
    m_GridSettings.snapEnabled = !m_GridSettings.snapEnabled;
}

void EditorPrefs::AddRecentLuaScript(const char* relativePath)
{
    // Remove a single matching item.
    auto it = std::find( m_Lua_RecentScripts.begin(), m_Lua_RecentScripts.end(), relativePath );
    if( it != m_Lua_RecentScripts.end() )
    {
        m_Lua_RecentScripts.erase( it );
    }

    // Insert the path at the start of the list.
    it = m_Lua_RecentScripts.begin();
    m_Lua_RecentScripts.insert( it, relativePath );

    // Remove any paths past the end.
    while( m_Lua_RecentScripts.size() > MAX_RECENT_LUA_SCRIPTS )
        m_Lua_RecentScripts.pop_back();
}

void EditorPrefs::AddRecentScene(const char* relativePath)
{
    // Remove a single matching item.
    auto it = std::find( m_File_RecentScenes.begin(), m_File_RecentScenes.end(), relativePath );
    if( it != m_File_RecentScenes.end() )
    {
        m_File_RecentScenes.erase( it );
    }

    // Insert the path at the start of the list.
    it = m_File_RecentScenes.begin();
    m_File_RecentScenes.insert( it, relativePath );

    // Remove any paths past the end.
    while( m_File_RecentScenes.size() > MAX_RECENT_SCENES )
        m_File_RecentScenes.pop_back();
}

void EditorPrefs::AddRecentDocument(const char* relativePath)
{
    // Remove a single matching item.
    auto it = std::find( m_Document_RecentDocuments.begin(), m_Document_RecentDocuments.end(), relativePath );
    if( it != m_Document_RecentDocuments.end() )
    {
        m_Document_RecentDocuments.erase( it );
    }

    // Insert the path at the start of the list.
    it = m_Document_RecentDocuments.begin();
    m_Document_RecentDocuments.insert( it, relativePath );

    // Remove any paths past the end.
    while( m_Document_RecentDocuments.size() > MAX_RECENT_DOCUMENTS )
        m_Document_RecentDocuments.pop_back();
}

void EditorPrefs::FillGridSettingsWindow()
{
#if MYFW_USING_IMGUI
    ImGui::DragFloat3( "Step Size", &m_GridSettings.stepSize.x );
#endif
}
