//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EngineCore.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "Core/EngineComponentTypeManager.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Renderer_Base.h"
#include "../../../SharedGameCode/Core/MyMeshText.h"
#include "../../../SharedGameCode/Core/RenderTextQuick.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorMainFrame.h"
#include "../SourceEditor/EditorPrefs.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/TransformGizmo.h"
#include "../SourceEditor/Editor_ImGui/EditorLayoutManager_ImGui.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#include "../SourceEditor/Interfaces/EditorInterface.h"
#include "../SourceEditor/Interfaces/EditorInterface_SceneManagement.h"
#include "../SourceEditor/Interfaces/EditorInterface_2DPointEditor.h"
#include "../SourceEditor/Interfaces/EditorInterface_VoxelMeshEditor.h"
#endif

EngineCore* g_pEngineCore = nullptr;

EngineCore::EngineCore(Renderer_Base* pRenderer, bool createAndOwnGlobalManagers)
: GameCore( pRenderer, createAndOwnGlobalManagers )
{
    g_pEngineCore = this;

    m_pComponentSystemManager = nullptr;

    m_pImGuiManager = nullptr;
    m_pBulletWorld = nullptr;

#if MYFW_USING_LUA
    m_pLuaGameState = nullptr;
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
    m_EditorMode = true;
#else
    m_EditorMode = false;
#endif
    m_AllowGameToRunInEditorMode = false;

    m_Paused = false;

    m_TimeSinceLastPhysicsStep = 0;

    m_pShaderFile_TintColor = nullptr;
    m_pShaderFile_TintColorWithAlpha = nullptr;
    m_pShaderFile_SelectedObjects = nullptr;
    m_pShaderFile_ClipSpaceTexture = nullptr;
    m_pShaderFile_ClipSpaceColor = nullptr;
    m_pShaderFile_FresnelTint = nullptr;
    m_pShader_TintColor = nullptr;
    m_pShader_TintColorWithAlpha = nullptr;
    m_pShader_SelectedObjects = nullptr;
    m_pShader_ClipSpaceTexture = nullptr;
    m_pShader_ClipSpaceColor = nullptr;
    m_pShader_FresnelTint = nullptr;
    m_pMaterial_Box2DDebugDraw = nullptr;
    m_pMaterial_3DGrid = nullptr;
    m_pMaterial_MousePicker = nullptr;
    m_pMaterial_ClipSpaceTexture = nullptr;
    m_pMaterial_ClipSpaceColor = nullptr;
    m_pMaterial_FresnelTint = nullptr;

    m_GameWidth = 0;
    m_GameHeight = 0;

    m_UnloadAllScenesNextTick = false;
    m_SceneReloadRequested = false;
    for( int i=0; i<MAX_SCENES_QUEUED_TO_LOAD; i++ )
    {
        m_pSceneFilesLoading[i].m_pFile = nullptr;
        m_pSceneFilesLoading[i].m_SceneID = SCENEID_NotSet;
    }

    m_PauseTimeToAdvance = 0;

    m_LastMousePos.Set( -1, -1 );

    m_Debug_DrawWireframe = false;

#if MYFW_EDITOR
    m_pEditorPrefs = nullptr;
    m_pEditorState = nullptr;
    m_pEditorMainFrame = nullptr;

    m_Debug_DrawMousePickerFBO = false;
    m_Debug_DrawSelectedAnimatedMesh = false;
    m_Debug_DrawSelectedMaterial = false;
    m_Debug_ShowProfilingInfo = true;
    m_Debug_DrawGLStats = false;
    m_pSphereMeshFile = nullptr;
    m_pSprite_DebugQuad = nullptr;
    m_pMesh_MaterialBall = nullptr;
    m_FreeAllMaterialsAndTexturesWhenUnloadingScene = false;
    m_pDebugFont = nullptr;
    m_pDebugTextMesh = nullptr;

    m_pEditorInterfaces[EditorInterfaceType_SceneManagement] = MyNew EditorInterface_SceneManagement( this );
    m_pEditorInterfaces[EditorInterfaceType_2DPointEditor] = MyNew EditorInterface_2DPointEditor( this );
    m_pEditorInterfaces[EditorInterfaceType_VoxelMeshEditor] = MyNew EditorInterface_VoxelMeshEditor( this );
    m_CurrentEditorInterfaceType = EditorInterfaceType_NumInterfaces;
    m_pCurrentEditorInterface = nullptr;
#endif //MYFW_EDITOR

#if MYFW_PROFILING_ENABLED
    m_FrameTimingNextEntry = 0;
#endif

    m_DebugFPS = 0;
    m_LuaMemoryUsedLastFrame = 0;
    m_LuaMemoryUsedThisFrame = 0;
    m_TotalMemoryAllocatedLastFrame = 0;
    m_SingleFrameStackSizeLastFrame = 0;
    m_SingleFrameStackSizeThisFrame = 0;

    for( int i=0; i<32; i++ )
    {
        m_GameObjectFlagStrings[i] = nullptr;
    }
}

EngineCore::~EngineCore()
{
    Cleanup();
}

void EngineCore::Cleanup()
{
    SaveEditorPrefs();

    SAFE_DELETE( m_pImGuiManager );

    SAFE_DELETE( g_pRTQGlobals );

#if MYFW_USING_LUA
    SAFE_DELETE( m_pLuaGameState );
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
    for( int i=0; i<EditorInterfaceType_NumInterfaces; i++ )
        SAFE_DELETE( m_pEditorInterfaces[i] );
#endif //MYFW_EDITOR

    if( m_pShaderFile_TintColor )           g_pFileManager->FreeFile( m_pShaderFile_TintColor );          m_pShaderFile_TintColor = nullptr;
    if( m_pShaderFile_TintColorWithAlpha )  g_pFileManager->FreeFile( m_pShaderFile_TintColorWithAlpha ); m_pShaderFile_TintColorWithAlpha = nullptr;
    if( m_pShaderFile_SelectedObjects )     g_pFileManager->FreeFile( m_pShaderFile_SelectedObjects );    m_pShaderFile_SelectedObjects = nullptr;
    if( m_pShaderFile_ClipSpaceTexture )    g_pFileManager->FreeFile( m_pShaderFile_ClipSpaceTexture );   m_pShaderFile_ClipSpaceTexture = nullptr;
    if( m_pShaderFile_ClipSpaceColor )      g_pFileManager->FreeFile( m_pShaderFile_ClipSpaceColor );     m_pShaderFile_ClipSpaceColor = nullptr;
    if( m_pShaderFile_FresnelTint )         g_pFileManager->FreeFile( m_pShaderFile_FresnelTint );        m_pShaderFile_FresnelTint = nullptr;
    SAFE_RELEASE( m_pShader_TintColor );
    SAFE_RELEASE( m_pShader_TintColorWithAlpha );
    SAFE_RELEASE( m_pShader_SelectedObjects );
    SAFE_RELEASE( m_pShader_ClipSpaceTexture );
    SAFE_RELEASE( m_pShader_ClipSpaceColor );
    SAFE_RELEASE( m_pShader_FresnelTint );
    SAFE_RELEASE( m_pMaterial_Box2DDebugDraw );
    SAFE_RELEASE( m_pMaterial_3DGrid );
    SAFE_RELEASE( m_pMaterial_MousePicker );
    SAFE_RELEASE( m_pMaterial_ClipSpaceTexture );
    SAFE_RELEASE( m_pMaterial_ClipSpaceColor );
    SAFE_RELEASE( m_pMaterial_FresnelTint );

#if MYFW_EDITOR
    SAFE_DELETE( m_pEditorPrefs );
    SAFE_DELETE( m_pEditorState );
    SAFE_DELETE( m_pEditorMainFrame );

    SAFE_RELEASE( m_pSphereMeshFile );
    SAFE_RELEASE( m_pSprite_DebugQuad );
    SAFE_RELEASE( m_pMesh_MaterialBall );
    SAFE_RELEASE( m_pDebugFont );
    SAFE_RELEASE( m_pDebugTextMesh );
#endif //MYFW_EDITOR

    SAFE_DELETE( m_pComponentSystemManager );
    SAFE_DELETE( m_pBulletWorld );

    for( int i=0; i<32; i++ )
    {
        delete[] m_GameObjectFlagStrings[i];
        m_GameObjectFlagStrings[i] = nullptr;
    }

    m_SingleFrameMemoryStack.Cleanup();
}

void EngineCore::SaveEditorPrefs()
{
#if MYFW_EDITOR
    if( m_pEditorPrefs == nullptr )
        return;

    cJSON* jPrefs = m_pEditorPrefs->SaveStart();

    //// Save Layout strings.
    //for( int i=0; i<4; i++ )
    //{
    //    if( g_SavedPerspectives[i] != g_DefaultPerspectives[i] )
    //    {
    //        char name[10];
    //        sprintf_s( name, 10, "Layout%d", i );
    //        cJSON_AddStringToObject( pPrefs, name, g_SavedPerspectives[i] );
    //    }
    //}

    // General options.
    //const char* relativepath = GetRelativePath( g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath );
    //if( relativepath )
    //    cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", relativepath );
    //else
    //    cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath );

    //cJSON_AddItemToObject( pPrefs, "EditorCam", g_pEngineCore->GetEditorState()->GetEditorCamera()->m_pComponentTransform->ExportAsJSONObject( false, true ) );

    //cJSON* jGameObjectFlagsArray = cJSON_CreateStringArray( g_pEngineCore->GetGameObjectFlagStringArray(), 32 );
    //cJSON_AddItemToObject( pPrefs, "GameObjectFlags", jGameObjectFlagsArray );

    //// View menu options.
    //cJSON_AddNumberToObject( pPrefs, "EditorLayout", GetDefaultEditorPerspectiveIndex() );
    //cJSON_AddNumberToObject( pPrefs, "GameplayLayout", GetDefaultGameplayPerspectiveIndex() );
    //extern GLViewTypes g_CurrentGLViewType;
    //cJSON_AddNumberToObject( pPrefs, "GameAspectRatio", g_CurrentGLViewType );

    //// Mode menu options.
    //cJSON_AddNumberToObject( pPrefs, "LaunchPlatform", GetLaunchPlatformIndex() );

    m_pEditorPrefs->SaveFinish( jPrefs );
#endif //MYFW_EDITOR
}

// Helper functions for some global namespace lua binding.
// Exposed to Lua, change elsewhere if function signature changes.
void EngineCoreSetMousePosition(float x, float y)
{
    g_pEngineCore->SetMousePosition( x, y );
}

// Exposed to Lua, change elsewhere if function signature changes.
void EngineCoreSetMouseLock(bool lock)
{
    g_pEngineCore->SetMouseLock( lock );
}

#if MYFW_USING_LUA
void EngineCore::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<EngineCore>( "EngineCore" )
            .addFunction( "RequestScene", &EngineCore::RequestScene ) // void EngineCore::RequestScene(const char* fullpath)
            .addFunction( "SwitchScene", &EngineCore::SwitchScene ) // void EngineCore::SwitchScene(const char* fullpath)
            .addFunction( "ReloadScene", &EngineCore::ReloadScene ) // void EngineCore::ReloadScene(SceneID sceneid)
            //.addFunction( "SetMousePosition", &EngineCore::SetMousePosition )
        .endClass();
    
    luabridge::getGlobalNamespace( luastate ).addFunction( "SetMousePosition", EngineCoreSetMousePosition ); // void EngineCoreSetMousePosition(float x, float y)
    luabridge::getGlobalNamespace( luastate ).addFunction( "SetMouseLock", EngineCoreSetMouseLock ); // void EngineCoreSetMouseLock(bool lock)
}
#endif //MYFW_USING_LUA

void EngineCore::InitializeManagers()
{
    if( g_pFileManager == nullptr )
        g_pFileManager = MyNew EngineFileManager( this );

    GameCore::InitializeManagers();

    if( g_pRTQGlobals == nullptr )
        g_pRTQGlobals = MyNew RenderTextQuickGlobals( this );

    if( m_pImGuiManager == nullptr )
        m_pImGuiManager = MyNew ImGuiManager;
}

void EngineCore::InitializeGameObjectFlagStrings(cJSON* jStringsArray)
{
    if( jStringsArray == nullptr )
    {
        const char* strings[32] =
        {
            "Camera-Main", "Camera-HUD", "Player", "Enemy", "Target", "5", "6", "7", "8", "9",
            "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
            "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
            "30", "31",
        };
        
        for( int i=0; i<32; i++ )
        {
            int len = (int)strlen( strings[i] ) + 1;
            m_GameObjectFlagStrings[i] = new char[len];
            strcpy_s( m_GameObjectFlagStrings[i], len, strings[i] );
        }
    }
    else
    {
        int stringarraylength = cJSON_GetArraySize( jStringsArray );
        if( stringarraylength > 32 )
        {
            stringarraylength = 32;
            LOGError( LOGTag, "Too many strings in 'GameObjectFlags' in 'EditorPrefs.ini'\n" );
            MyAssert( false );
        }

        for( int i=0; i<stringarraylength; i++ )
        {
            cJSON* jGameObjectFlagsString = cJSON_GetArrayItem( jStringsArray, i );
            
            int len = (int)strlen( jGameObjectFlagsString->valuestring ) + 1;
            m_GameObjectFlagStrings[i] = new char[len];
            strcpy_s( m_GameObjectFlagStrings[i], len, jGameObjectFlagsString->valuestring );
        }
    }
}

void EngineCore::OneTimeInit()
{
    GameCore::OneTimeInit();

    // Allocate one meg of ram for now, this stack gets wiped each frame in OnDrawFrameDone().
    //  TODO: Expose size, hardcoded to 5meg for now... used by editor mode scene load and voxel world creation.
    m_SingleFrameMemoryStack.Initialize( 5000000 );

#if MYFW_EDITOR
    m_pEditorState = MyNew EditorState( this );
    m_pEditorState->m_pDebugViewFBO = GetManagers()->GetTextureManager()->CreateFBO( 0, 0, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_None, 0, false, true );
    m_pEditorState->m_pMousePickerFBO = GetManagers()->GetTextureManager()->CreateFBO( 0, 0, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_None, 0, false, true );

    if( m_pDebugFont == nullptr )
    {
        m_pDebugFont = g_pFontManager->CreateFont( "Data/DataEngine/Fonts/Nevis60.fnt" );
#if MYFW_EDITOR
        m_pDebugFont->GetFile()->MemoryPanel_Hide();
#endif
    }

    SetEditorInterface( EditorInterfaceType_SceneManagement );
#endif //MYFW_EDITOR

    TextureDefinition* pErrorTexture = GetManagers()->GetTextureManager()->GetErrorTexture();
    MaterialManager* pMaterialManager = GetManagers()->GetMaterialManager();
    ShaderGroupManager* pShaderGroupManager = GetManagers()->GetShaderGroupManager();

    // Setup our shaders and materials.
    m_pShaderFile_TintColor          = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_TintColor.glsl" );
    m_pShaderFile_TintColorWithAlpha = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_TintColorWithAlpha.glsl" );
    m_pShaderFile_SelectedObjects    = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_SelectedObjects.glsl" );
    m_pShaderFile_ClipSpaceTexture   = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_ClipSpaceTexture.glsl" );
    m_pShaderFile_ClipSpaceColor     = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_ClipSpaceColor.glsl" );
    m_pShaderFile_FresnelTint        = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_FresnelTint.glsl" );
#if MYFW_EDITOR
    m_pShaderFile_TintColor->MemoryPanel_Hide();
    m_pShaderFile_TintColorWithAlpha->MemoryPanel_Hide();
    m_pShaderFile_SelectedObjects->MemoryPanel_Hide();
    m_pShaderFile_ClipSpaceTexture->MemoryPanel_Hide();
    m_pShaderFile_ClipSpaceColor->MemoryPanel_Hide();
    m_pShaderFile_FresnelTint->MemoryPanel_Hide();
#endif
    m_pShader_TintColor          = MyNew ShaderGroup( pShaderGroupManager, m_pShaderFile_TintColor, pErrorTexture );
    m_pShader_TintColorWithAlpha = MyNew ShaderGroup( pShaderGroupManager, m_pShaderFile_TintColorWithAlpha, pErrorTexture );
    m_pShader_SelectedObjects    = MyNew ShaderGroup( pShaderGroupManager, m_pShaderFile_SelectedObjects, pErrorTexture );
    m_pShader_ClipSpaceTexture   = MyNew ShaderGroup( pShaderGroupManager, m_pShaderFile_ClipSpaceTexture, pErrorTexture );
    m_pShader_ClipSpaceColor     = MyNew ShaderGroup( pShaderGroupManager, m_pShaderFile_ClipSpaceColor, pErrorTexture );
    m_pShader_FresnelTint        = MyNew ShaderGroup( pShaderGroupManager, m_pShaderFile_FresnelTint, pErrorTexture );
    m_pMaterial_Box2DDebugDraw   = MyNew MaterialDefinition( pMaterialManager, m_pShader_TintColor, ColorByte(128,128,128,255) );
    m_pMaterial_3DGrid           = MyNew MaterialDefinition( pMaterialManager, m_pShader_TintColor, ColorByte(128,128,128,255) );
    m_pMaterial_MousePicker      = MyNew MaterialDefinition( pMaterialManager, m_pShader_ClipSpaceTexture );
    m_pMaterial_ClipSpaceTexture = MyNew MaterialDefinition( pMaterialManager, m_pShader_ClipSpaceTexture );
    m_pMaterial_ClipSpaceColor   = MyNew MaterialDefinition( pMaterialManager, m_pShader_ClipSpaceColor );
    m_pMaterial_FresnelTint      = MyNew MaterialDefinition( pMaterialManager, m_pShader_FresnelTint );

    // Initialize our component system.
    m_pComponentSystemManager = MyNew ComponentSystemManager( CreateComponentTypeManager(), this );

    if( m_pDebugTextMesh == nullptr )
    {
        m_pDebugTextMesh = MyNew MyMeshText( 100, m_pDebugFont, m_pComponentSystemManager->GetGameCore()->GetManagers()->GetMeshManager() );
    }

    // Initialize lua state and register any variables needed.
#if MYFW_USING_LUA
    m_pLuaGameState = CreateLuaGameState();
    m_pLuaGameState->Rebuild(); // Reset the lua state.
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
//    m_pComponentSystemManager->CreateNewScene( "Unsaved.scene", 1 );
    CreateDefaultEditorSceneObjects();

    // Initialize our editor interfaces (load materials, etc).
    for( int i=0; i<EditorInterfaceType_NumInterfaces; i++ )
    {
        m_pEditorInterfaces[i]->Initialize();
    }
#endif //MYFW_EDITOR

    // Create the box2d world, pass in a material for the debug renderer.
    //ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
    //m_pBox2DWorld = MyNew Box2DWorld( m_pMaterial_Box2DDebugDraw, &pCamera->m_Camera3D.m_matViewProj, new EngineBox2DContactListener );

//    CreateDefaultSceneObjects();

    if( m_pImGuiManager )
    {
        m_pImGuiManager->Init( (float)GetWindowWidth(), (float)GetWindowHeight() );
#if MYFW_EDITOR
        m_pEditorMainFrame = MyNew EditorMainFrame_ImGui( this );

        // For editor build, start the next frame immediately, so imgui calls can be made in tick callbacks.
        // Tick happens before game(0) window is drawn, m_pImGuiManager's draw only happens on editor(1) window.
        //m_pImGuiManager->StartFrame();
#endif //MYFW_EDITOR
    }

    // Create one bullet world shared between all scenes.
#if !MYFW_EDITOR
    // Disable debug draw in non-editor builds.
    m_pBulletWorld = MyNew BulletWorld( 0, 0, 0 );
#else
    ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
    m_pBulletWorld = MyNew BulletWorld( m_pMaterial_Box2DDebugDraw, &pCamera->m_Camera3D.m_matProj, &pCamera->m_Camera3D.m_matView );
#endif

#if MYFW_EDITOR
    m_pEditorPrefs = g_pEditorPrefs;
    if( m_pEditorPrefs == nullptr )
    {
        // This should be the editor pref load point for ImGui Editor builds.
        m_pEditorPrefs = MyNew EditorPrefs;
        m_pEditorPrefs->Init();
        m_pEditorPrefs->LoadWindowSizePrefs();
        m_pEditorPrefs->LoadPrefs();
    }

    // Load in some editor preferences.
    if( g_pEditorPrefs && g_pEditorPrefs->GetEditorPrefsJSONString() )
    {
        cJSON* jEditorPrefs = g_pEditorPrefs->GetEditorPrefsJSONString();

        cJSON* jGameObjectFlagsArray = cJSON_GetObjectItem( jEditorPrefs, "GameObjectFlags" );
        g_pEngineCore->InitializeGameObjectFlagStrings( jGameObjectFlagsArray );
    }
    else
    {
        g_pEngineCore->InitializeGameObjectFlagStrings( nullptr );
    }

    m_pEditorPrefs->LoadLastSceneLoaded();
#else
    // TODO: Fix! This won't work if flags were customized and saved into editorprefs.ini.
    InitializeGameObjectFlagStrings( nullptr );
#endif
}

bool EngineCore::IsReadyToRender()
{
    return true;
}

void EngineCore::RequestClose()
{
#if MYFW_EDITOR
    ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->RequestCloseWindow();
#else
    GameCore::RequestClose();
#endif
}

float EngineCore::Tick(float deltaTime)
{
#if MYFW_EDITOR
    // If a change in editor perspective was requested, change it before the start of the frame.
    ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->GetLayoutManager()->ApplyLayoutChange();

    m_pImGuiManager->StartFrame();
#endif

    //ImGui::Begin( "Editor Debug" );
    //ImGui::Text( "IsAnyWindowHovered: %d", ImGui::IsAnyWindowHovered() );
    //ImGui::Text( "IsAnyWindowFocused: %d", ImGui::IsAnyWindowFocused() );
    //ImGui::Text( "IsMouseLocked: %d", g_pGameCore->IsMouseLocked() );
    //ImGui::Text( "MousePos: %0.0f, %0.0f", ImGui::GetMousePos().x, ImGui::GetMousePos().y );
    //ImGui::Text( "FrameCount: %d", ImGui::GetFrameCount() );
    //ImGui::Text( "Time: %f", ImGui::GetTime() );
    //ImGui::End();
    //m_pImGuiManager->OnFocusLost();

#if MYFW_PROFILING_ENABLED
    static double Timing_LastFrameTime = 0;

    double Timing_Start = MyTime_GetSystemTime();
#endif

    m_pLuaGameState->Tick();

    if( m_pImGuiManager )
    {
        m_pImGuiManager->StartTick( deltaTime );
    }

#if MYFW_EDITOR
    if( m_pEditorMainFrame )
    {
        ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->Update( deltaTime );
        ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->AddEverything();
    }
#endif

    if( m_UnloadAllScenesNextTick )
    {
        UnloadScene( SCENEID_AllScenes, true );
#if MYFW_EDITOR
        // If switching scene while in the editor, delete the quick save.
        Editor_DeleteQuickScene( "temp_editor_onplay.scene" );
#endif
        m_UnloadAllScenesNextTick = false;
        return 0;
    }

    if( m_SceneReloadRequested )
    {
        ReloadSceneInternal( SCENEID_MainScene );
        m_SceneReloadRequested = false;
        return 0;
    }

#if MYFW_EDITOR
    m_pCurrentEditorInterface->Tick( deltaTime );
#endif

    {
        static int numframes = 0;
        static double totaltime = 0;

        totaltime += deltaTime;
        numframes++;
        if( totaltime > 1 )
        {
            m_DebugFPS = (float)(numframes / totaltime);
            numframes = 0;
            totaltime = 0;
        }
    }

    if( deltaTime > 0.2f )
        deltaTime = 0.2f;

    //if( deltaTime == 0 && m_EditorMode == false )
    //    LOGInfo( LOGTag, "Tick: %f\n", deltaTime );
    //LOGInfo( LOGTag, "Tick: %f\n", deltaTime );

    float TimeUnpaused = deltaTime;

    GameCore::Tick( deltaTime );

    // If the next scene requested is ready load the scene.
    MyFileObject* pFile = m_pSceneFilesLoading[0].m_pFile;
    if( pFile && pFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        SceneID sceneid = g_pComponentSystemManager->GetNextSceneID();

        g_pComponentSystemManager->m_pSceneInfoMap[sceneid].Reset();
        g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_InUse = true;
        g_pComponentSystemManager->m_pSceneInfoMap[sceneid].ChangePath( pFile->GetFullPath() );

        // Loading an additional scene, or a lua script requested a scene.
        //     So, if we're in editor mode, don't call "play" when loading is finished.
        bool playWhenFinishedLoading = false;

#if MYFW_EDITOR
        if( m_EditorMode == true )
        {
            playWhenFinishedLoading = false;
            //playWhenFinishedLoading = true;
        }
        else
#endif
        {
            playWhenFinishedLoading = true;
        }

        LoadSceneFromJSON( pFile->GetFilenameWithoutExtension(), pFile->GetBuffer(), sceneid, playWhenFinishedLoading );

        SAFE_RELEASE( m_pSceneFilesLoading[0].m_pFile );

        // Shift all objects up a slot in the queue.
        for( int i=0; i<MAX_SCENES_QUEUED_TO_LOAD-1; i++ )
        {
            m_pSceneFilesLoading[i] = m_pSceneFilesLoading[i+1];
        }
        m_pSceneFilesLoading[MAX_SCENES_QUEUED_TO_LOAD-1].m_pFile = nullptr;
        m_pSceneFilesLoading[MAX_SCENES_QUEUED_TO_LOAD-1].m_SceneID = SCENEID_NotSet;

#if !MYFW_EDITOR
        RegisterGameplayButtons();
#endif
    }

#if MYFW_EDITOR
    m_pEditorState->m_pTransformGizmo->Tick( deltaTime, m_pEditorState );
    m_pEditorState->UpdateCamera( deltaTime );
#endif

    // Change deltaTime if needed.
    {
        float timescale = m_pComponentSystemManager->GetTimeScale();

        deltaTime *= timescale;

        if( m_EditorMode && m_AllowGameToRunInEditorMode == false )
            deltaTime = 0;

        if( m_Paused )
            deltaTime = m_PauseTimeToAdvance;

        m_PauseTimeToAdvance = 0;
    }

    if( m_EditorMode == false || m_AllowGameToRunInEditorMode )
    {
#if MYFW_PROFILING_ENABLED && MYFW_EDITOR
        double Physics_Timing_Start = MyTime_GetSystemTime();
#endif //MYFW_PROFILING_ENABLED && MYFW_EDITOR

        m_pBulletWorld->PhysicsUpdate( deltaTime );

        m_TimeSinceLastPhysicsStep += deltaTime;
        while( m_TimeSinceLastPhysicsStep > 1/60.0f )
        {
            m_TimeSinceLastPhysicsStep -= 1/60.0f;
            //m_pBulletWorld->PhysicsStep();

            for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
            {
                if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse && g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld )
                {
                    g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->PhysicsStep();
                    //g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->m_pWorld->ClearForces();
                }
            }
        }

#if MYFW_PROFILING_ENABLED && MYFW_EDITOR
        double Physics_Timing_End = MyTime_GetSystemTime();

        if( g_GLCanvasIDActive == 0 )
            m_FrameTimingInfo[m_FrameTimingNextEntry].Update_Physics = (float)((Physics_Timing_End - Physics_Timing_Start)*1000);
#endif //MYFW_PROFILING_ENABLED && MYFW_EDITOR
    }

    // Tick all components.
    m_pComponentSystemManager->Tick( deltaTime );

#if MYFW_USING_LUA
    if( g_pLuaGameState && g_pLuaGameState->m_pLuaState )
    {
        int luamemcountk = lua_gc( g_pLuaGameState->m_pLuaState, LUA_GCCOUNT, 0 );
        int luamemcountb = lua_gc( g_pLuaGameState->m_pLuaState, LUA_GCCOUNTB, 0 );
        m_LuaMemoryUsedLastFrame = m_LuaMemoryUsedThisFrame;
        m_LuaMemoryUsedThisFrame = luamemcountk*1024 + luamemcountb;

        lua_gc( g_pLuaGameState->m_pLuaState, LUA_GCCOLLECT, 0 );
    }
#endif

#if MYFW_PROFILING_ENABLED
    double Timing_End = MyTime_GetSystemTime();
    
    m_FrameTimingInfo[m_FrameTimingNextEntry].Tick = (float)((Timing_End - Timing_Start)*1000);
    m_FrameTimingInfo[m_FrameTimingNextEntry].FrameTime = (float)((Timing_Start - Timing_LastFrameTime)*1000);

    Timing_LastFrameTime = Timing_Start;
#endif

    // Update the global unpaused time.
    if( m_EditorMode && m_AllowGameToRunInEditorMode == false )
        return TimeUnpaused;
    else
        return deltaTime;
}

void OnFileUpdated_CallbackFunction(GameCore* pGameCore, MyFileObject* pFile)
{
#if MYFW_EDITOR
#if MYFW_USING_WX
    g_pComponentSystemManager->OnFileUpdated( pFile );
#endif

    LOGInfo( LOGTag, "OnFileUpdated_CallbackFunction pFile = %s\n", pFile->GetFullPath() );

    if( strcmp( pFile->GetExtensionWithDot(), ".mymaterial" ) == 0 )
    {
        MaterialManager* pMaterialManager = pGameCore->GetManagers()->GetMaterialManager();
        MaterialDefinition* pMaterial = pMaterialManager->FindMaterialByFilename( pFile->GetFullPath() );
        pMaterialManager->ReloadMaterial( pMaterial );
    }

    if( strcmp( pFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
    {
        MeshManager* pMeshManager = pGameCore->GetManagers()->GetMeshManager();
        MyMesh* pMesh = pMeshManager->FindMeshBySourceFile( pFile );
        // Clear out the old mesh and load in the new one.
        pMesh->Clear();
    }
#endif

    // TODO: EntityComponentManager-> Tell all script components file is updated.
}

void EngineCore::OnFocusGained()
{
    if( this->HasOneTimeInitBeenCalled() == false )
        return;

    GameCore::OnFocusGained();

#if MYFW_EDITOR
    m_pEditorState->ClearKeyAndActionStates();

#if MYFW_USING_WX
    // Check if any of the "source" files, like .fbx's were updated, they aren't loaded by FileManager so wouldn't be detected there.
    g_pComponentSystemManager->CheckForUpdatedDataSourceFiles( false );
#endif
#endif

    // Reload any files that changed while we were out of focus.
    if( g_pFileManager == nullptr )
        return;

    int filesupdated = g_pFileManager->ReloadAnyUpdatedFiles( this, OnFileUpdated_CallbackFunction );

    if( filesupdated )
    {
        //GetManagers()->GetShaderManager()->InvalidateAllShaders( true );
        //GetManagers()->GetTextureManager()->InvalidateAllTextures( true );
        //GetManagers()->GetBufferManager()->InvalidateAllBuffers( true );
    }
}

void EngineCore::OnFocusLost()
{
    GameCore::OnFocusLost();

#if MYFW_EDITOR
    m_pEditorState->OnFocusLost();
#endif

    m_pImGuiManager->OnFocusLost();
}

void EngineCore::OnDrawFrameStart(unsigned int canvasid)
{
    GameCore::OnDrawFrameStart( canvasid );

#if !MYFW_EDITOR
    if( m_pImGuiManager )
    {
        m_pImGuiManager->StartFrame();
    }
#endif //!MYFW_EDITOR
}

void EngineCore::OnDrawFrame(unsigned int canvasid)
{
#if MYFW_PROFILING_ENABLED
    double Timing_Start = MyTime_GetSystemTime();
#endif

#if !MYFW_OPENGLES2
    if( m_Debug_DrawWireframe )
    {
        g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Line );
    }
#endif

#if MYFW_EDITOR
    if( m_pEditorMainFrame )
    {
        // Backup the window width/height.
        uint32 windowWidth = GetWindowWidth();
        uint32 windowHeight = GetWindowHeight();

        // Draw the game and editor contents into textures.
        ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->DrawGameAndEditorWindows( this );

        // Reset to full window size.
        MyViewport viewport( 0, 0, windowWidth, windowHeight );
        g_pRenderer->EnableViewport( &viewport, true );

        // Render out the ImGui command list to the full window.
        g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.1f, 0.2f, 1.0f ) );
        g_pRenderer->ClearBuffers( true, true, false );

        if( m_pImGuiManager )
        {
            m_pImGuiManager->EndFrame( (float)windowWidth, (float)windowHeight, true );
        }

        return;
    }
#endif //MYFW_EDITOR

    GameCore::OnDrawFrame( canvasid );

    MyRect windowrect( 0, 0, 0, 0 );

#if MYFW_EDITOR
    if( g_GLCanvasIDActive == 1 )
    {
        m_pCurrentEditorInterface->OnDrawFrame( canvasid );
        windowrect = m_pEditorState->m_EditorWindowRect;
    }
    else
#endif
    {
        // Draw all components.
        m_pComponentSystemManager->OnDrawFrame();
        windowrect.Set( (int)m_MainViewport.GetX(), (int)m_MainViewport.GetY(), (int)m_MainViewport.GetWidth(), (int)m_MainViewport.GetHeight() );
    }

#if !MYFW_OPENGLES2
    if( m_Debug_DrawWireframe )
        g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Fill );
#endif

#if MYFW_EDITOR
    if( m_Debug_DrawGLStats && m_pDebugTextMesh )// && g_GLCanvasIDActive == 1 )
    {
        if( m_pDebugTextMesh->GetMaterial( 0 ) == nullptr )
        {
            MaterialDefinition* pMaterial = GetManagers()->GetMaterialManager()->LoadMaterial( "Data/DataEngine/Materials/Nevis60.mymaterial" );
            MyAssert( pMaterial );
            if( pMaterial )
            {
                m_pDebugTextMesh->SetMaterial( pMaterial, 0 );
                pMaterial->Release();
            }
        }

        //m_pDebugTextMesh->CreateStringWhite( false, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h, Justify_TopRight, Vector2(0,0),
        //                                     "GLStats - buffers(%0.2fM) - draws(%d) - fps(%d)", g_pBufferManager->CalculateTotalMemoryUsedByBuffers()/1000000.0f, g_GLStats.GetNumDrawCallsLastFrameForCurrentCanvasID(), (int)m_DebugFPS );
        m_pDebugTextMesh->CreateStringWhite( false, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h, Justify_TopRight, Vector2(0,0),
            "GL - draws(%d) - fps(%d)", g_GLStats.GetNumDrawCallsLastFrameForCurrentCanvasID(), (int)m_DebugFPS );

        // Draw Lua memory usage.
        {
            int megs = m_LuaMemoryUsedThisFrame/1000000;
            int kilos = (m_LuaMemoryUsedThisFrame - megs*1000000)/1000;
            int bytes = m_LuaMemoryUsedThisFrame%1000;

            int change = m_LuaMemoryUsedThisFrame - m_LuaMemoryUsedLastFrame;

            if( megs == 0 )
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-10, Justify_TopRight, Vector2(0,0),
                    "Lua - memory(%d,%03d) - (%d)", kilos, bytes, change );
            }
            else
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-10, Justify_TopRight, Vector2(0,0),
                    "Lua - memory(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
            }
        }

#if MYFW_WINDOWS
        // Draw Main ram memory usage.
        {
            size_t bytesused = MyMemory_GetNumberOfBytesAllocated();
            int megs = (int)(bytesused/1000000);
            int kilos = (int)((bytesused - megs*1000000)/1000);
            int bytes = bytesused%1000;

            int change = (int)(bytesused - m_TotalMemoryAllocatedLastFrame);

            if( megs == 0 )
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-20, Justify_TopRight, Vector2(0,0),
                    "Memory(%03d,%03d) - (%d)", kilos, bytes, change );
            }
            else
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-20, Justify_TopRight, Vector2(0,0),
                    "Memory(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
            }

            m_TotalMemoryAllocatedLastFrame = bytesused;
        }
#endif

        // Draw single frame stack ram memory usage.
        {
            unsigned int bytesused = m_SingleFrameStackSizeThisFrame;
            int megs = bytesused/1000000;
            int kilos = (bytesused - megs*1000000)/1000;
            int bytes = bytesused%1000;

            int change = bytesused - m_SingleFrameStackSizeLastFrame;

            if( megs == 0 )
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-30, Justify_TopRight, Vector2(0,0),
                    "Frame Stack(%03d,%03d) - (%d)", kilos, bytes, change );
            }
            else
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-30, Justify_TopRight, Vector2(0,0),
                    "Frame Stack(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
            }
        }

        MyMatrix matProj;
        matProj.CreateOrtho( (float)windowrect.x, (float)windowrect.x+windowrect.w, (float)windowrect.y, (float)windowrect.y+windowrect.h, -1, 1 );
        g_pRenderer->SetDepthTestEnabled( false );
        m_pDebugTextMesh->Draw( &matProj, nullptr, nullptr, nullptr,nullptr,nullptr,0,nullptr,nullptr,nullptr,nullptr );
        g_pRenderer->SetDepthTestEnabled( true );
    }
#endif //MYFW_EDITOR

    //glFinish();

#if MYFW_PROFILING_ENABLED
    double Timing_End = MyTime_GetSystemTime();

#if MYFW_EDITOR
    if( g_GLCanvasIDActive == 0 )
        m_FrameTimingInfo[m_FrameTimingNextEntry].Render_Game = (float)((Timing_End - Timing_Start)*1000);
    else if( g_GLCanvasIDActive == 1 )
        m_FrameTimingInfo[m_FrameTimingNextEntry].Render_Editor = (float)((Timing_End - Timing_Start)*1000);
#else
    m_FrameTimingInfo[m_FrameTimingNextEntry].Render_Game = (float)((Timing_End - Timing_Start)*1000);
#endif

#if MYFW_EDITOR
    if( g_GLCanvasIDActive == 1 && m_Debug_ShowProfilingInfo )
#elif MYFW_OPENGLES2
    if( false )
#endif
    {
        ImGui::SetNextWindowSize( ImVec2(150,50), ImGuiSetCond_FirstUseEver );
        ImGui::Begin( "Timing" );
        //ImGui::Text( "Hello world!" );
        //ImGui::SliderFloat( "", &m_AnimationTime, 0, 1, "Time: %.3f" );
    
        int numsamplestoshow = 60*5; // 5 seconds worth @ 60fps.
        MyAssert( numsamplestoshow < MAX_FRAMES_TO_STORE );

        int numentries = m_FrameTimingNextEntry + 1;
        int start = m_FrameTimingNextEntry - numsamplestoshow;
        if( start < 0 )
            start = 0;
        if( numsamplestoshow > numentries )
            numsamplestoshow = numentries;

        m_FrameTimingNextEntry++;

        // Sort of circular buffer the frame timings.
        if( m_FrameTimingNextEntry >= MAX_FRAMES_TO_STORE )
        {
            // Copy the last "numsamples" entries back to the start and reset where we insert records.
            for( int i=0; i<numsamplestoshow; i++ )
                m_FrameTimingInfo[0+i] = m_FrameTimingInfo[MAX_FRAMES_TO_STORE-numsamplestoshow+i];

            m_FrameTimingNextEntry = numsamplestoshow;
        }

        //m_FrameTimingInfo.back().Tick = sin( (float)MyTime_GetSystemTime() );

        ImGui::PlotLines( "Frame Time",    &m_FrameTimingInfo[start].FrameTime,         numsamplestoshow, 0, "", 0.0f, 1000/20.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );

        ImGui::PlotLines( "Tick",          &m_FrameTimingInfo[start].Tick,              numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );

        ImGui::PlotLines( "Physics",       &m_FrameTimingInfo[start].Update_Physics,    numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );
#if MYFW_EDITOR
        ImGui::PlotLines( "Render Editor", &m_FrameTimingInfo[start].Render_Editor,     numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );
#endif
        ImGui::PlotLines( "Render Game",   &m_FrameTimingInfo[start].Render_Game,       numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );

        ImGui::End();
    }
#endif //MYFW_PROFILING_ENABLED

    if( m_pImGuiManager )
    {
        m_pImGuiManager->EndFrame( (float)windowrect.w, (float)windowrect.h, true );

#if MYFW_EDITOR
        // For editor build, start the next frame immediately, so imgui calls can be made in tick callbacks.
        // Tick happens before game(0) window is drawn, m_pImGuiManager's draw only happens on editor(1) window.
        m_pImGuiManager->StartFrame();
#endif
    }
}

void EngineCore::OnDrawFrameDone()
{
    GameCore::OnDrawFrameDone();

    m_SingleFrameStackSizeLastFrame = m_SingleFrameStackSizeThisFrame;
    m_SingleFrameStackSizeThisFrame = m_SingleFrameMemoryStack.GetBytesUsed();
    m_SingleFrameMemoryStack.Clear();

#if MYFW_EDITOR
    //if( g_GLCanvasIDActive == 1 )
#endif
    //{
    //    if( m_pImGuiManager )
    //        m_pImGuiManager->ClearInput();
    //}
}

void EngineCore::OnFileRenamed(const char* fullpathbefore, const char* fullpathafter)
{
    g_pComponentSystemManager->OnFileRenamed( fullpathbefore, fullpathafter );
}

void EngineCore::OnDropFile(const char* filename)
{
    char fullpath[MAX_PATH];

    sprintf_s( fullpath, MAX_PATH, "%s", (const char*)filename );

    // If the datafile is in our working directory, then load it... otherwise TODO: copy it in?
    const char* relativepath = GetRelativePath( fullpath );
    if( relativepath == nullptr )
    {
        // File is not in our working directory.
        // TODO: Copy the file into our data folder?
        LOGError( LOGTag, "File must be in working directory\n" );
        //MyAssert( false );
        return;
    }

    g_pEngineCore->GetComponentSystemManager()->LoadDataFile( relativepath, SCENEID_MainScene, filename, true );
}

void EngineCore::SetMousePosition(float x, float y)
{
    // TODO: Get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    if( pCamera == nullptr )
        return;

    MyViewport* pViewport = &pCamera->m_Viewport;

    // Convert mouse to x/y in window space. TODO: put this in camera component.
    x = (x / m_GameWidth) * pCamera->m_Camera2D.m_ScreenWidth + pCamera->m_Camera2D.m_ScreenOffsetX + pViewport->GetX();
    y = (y / m_GameHeight) * pCamera->m_Camera2D.m_ScreenHeight + pCamera->m_Camera2D.m_ScreenOffsetY + pViewport->GetY();
    //x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - pCamera->m_WindowStartX) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
    //y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + pCamera->m_WindowStartY) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;

    // Window space wants mouse at top left.
    y = pViewport->GetHeight() - y;

    PlatformSpecific_SetMousePosition( x, y );
}

void EngineCore::SetMouseLock(bool lock)
{
    GameCore::SetMouseLock( lock );

#if MYFW_EDITOR
    // Don't lock the mouse if the game window isn't in focus.
    if( ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->IsGameWindowFocused() )
        LockSystemMouse();
#endif
}

bool EngineCore::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    if( GameCore::OnTouch( action, id, x, y, pressure, size ) )
        return true;

#if MYFW_EDITOR
    //if( m_EditorMode )
    {
        if( m_pEditorState->m_pEditorCamera )
        {
            ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();

            if( action == GCBA_RelativeMovement )
            {
                // Mouse held messages while the mouse is locked are relative.
                // Flip the y direction, up should be positive, down negative.
                y *= -1;
            }
            else
            {
                // Prefer 0,0 at bottom left.
                y = y;
            }

            if( HandleEditorInput( g_GLCanvasIDActive, -1, -1, action, id, x, y, pressure ) )
                return true;
        }

        //return false;
    }

    if( g_GLCanvasIDActive != 0 )
    {
        return false;
    }
#endif

#if !MYFW_EDITOR
    if( g_pGameCore->IsMouseLocked() == false )
    {
        float toplefty = y;
        if( m_pImGuiManager->HandleInput( -1, -1, action, id, x, toplefty, pressure ) )
            return true;
    }
    else
    {
        // In standalone game, if the mouse is locked, imgui needs to reset inputs.
        // TODO: This should be done once when the mouse is locked, not each frame.
        m_pImGuiManager->OnFocusLost();
    }
#endif

#if !MYFW_EDITOR && MYFW_WINDOWS
    // Good ol' hack to include this global function from MYFWWinMain.h
    bool LockSystemMouse();
    
    // For non ImGui editor builds, if the game wants the mouse locked, finish the locking process here after imgui manager uses the mouse.
    bool wasLockedBecauseOfThisClick = false;
    if( action == GCBA_Down && g_pGameCore->WasMouseLockRequested() )
    {
        wasLockedBecauseOfThisClick = LockSystemMouse();
        
        if( wasLockedBecauseOfThisClick )
            return true;
    }
#endif

    return OnTouchGameWindow( action, id, x, y, pressure, size );
}

bool EngineCore::OnTouchGameWindow(int action, int id, float x, float y, float pressure, float size)
{
    // If mouse lock was requested, don't let mouse held messages go further.
    if( g_pGameCore->WasMouseLockRequested() && g_pGameCore->IsMouseLocked() == false && action == GCBA_Held )
    {
        return false;
    }

#if MYFW_EDITOR
    if( g_pGameCore->WasMouseLockRequested() && action == GCBA_Down )
    {
        // If this call to lock the mouse actually did lock it, don't send the click to the game.
        // TODO: Also ignore the movements and the mouse up.
        if( LockSystemMouse() )
            return true;
    }
#endif

    // TODO: Get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    if( pCamera == nullptr )
        return false;

    // If the mouse is locked and it's a mouse held message, leave the x/y as is.
    //     Otherwise, convert to camera space.
    if( action == GCBA_RelativeMovement )
    {
        // x/y should be showing diffs in position, so leave them as is.
    }
    else
    {
        MyViewport* pViewport = &pCamera->m_Viewport;

        // Prefer 0,0 at bottom left.
        y = pViewport->GetHeight() - y;

        // Convert mouse to x/y in Camera2D space. TODO: put this in camera component.
        x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - pViewport->GetX()) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
        y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + pViewport->GetY()) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;
    }

    m_LastMousePos.Set( x, y );

    // Mouse moving without button down.
    //if( id == -1 )
    //    return false;

    return m_pComponentSystemManager->OnTouch( action, id, x, y, pressure, size );
}

bool EngineCore::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    GameCore::OnButtons( action, id );

    return m_pComponentSystemManager->OnButtons( action, id );
}

bool EngineCore::OnKeys(GameCoreButtonActions action, int keycode, int unicodechar)
{
#if MYFW_EDITOR
    // Not calling GameCore::OnKeys( action, keycode, unicodechar ) which translates keypresses to joystick input.

    if( action == GCBA_Down && keycode == 344 ) // F5 )
    {
        int filesupdated = g_pFileManager->ReloadAnyUpdatedFiles( this, OnFileUpdated_CallbackFunction );

        if( filesupdated )
        {
            GetManagers()->GetShaderManager()->InvalidateAllShaders( true );
            //GetManagers()->m_pTextureManager->InvalidateAllTextures( true );
            //GetManagers()->m_pBufferManager->InvalidateAllBuffers( true );
            return true;
        }
    }

    if( action == GCBA_Down )
    {
        if( keycode >= 0 && keycode < 255 )
        {
            m_KeysHeld[keycode] = true;
        }
    }

    if( HandleEditorInput( g_GLCanvasIDActive, action, keycode, -1, -1, -1, -1, -1 ) )
    {
        return true;
    }
#endif

#if MYFW_EDITOR
    if( g_GLCanvasIDActive == 0 )
#endif
    {
#if !MYFW_EDITOR
        if( m_pImGuiManager->HandleInput( action, keycode, -1, -1, -1, -1, -1 ) )
            return true;
#endif

        // GameCore::OnKeys translates keypresses to joystick input.
        if( GameCore::OnKeys( action, keycode, unicodechar ) )
            return true;

        if( m_pComponentSystemManager->OnKeys( action, keycode, unicodechar ) )
            return true;
    }

    return false;
}

bool EngineCore::OnChar(unsigned int c)
{
    m_pImGuiManager->OnChar( c );

    return false;
}

void EngineCore::OnModeTogglePlayStop()
{
#if MYFW_EDITOR
    if( m_EditorMode )
    {
        OnModePlay();
        
        // Set focus to gameplay window.
        if( g_pEngineCore->GetEditorPrefs()->Get_Mode_SwitchFocusOnPlayStop() )
        {
            ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->OnModeTogglePlayStop( false );
        }
    }
    else
    {
        OnModeStop();

        // Set focus to editor window.
        if( g_pEngineCore->GetEditorPrefs()->Get_Mode_SwitchFocusOnPlayStop() )
        {
            ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->OnModeTogglePlayStop( true );
        }
    }
#endif //MYFW_EDITOR
}

void EngineCore::OnModePlay()
{
#if MYFW_EDITOR
    if( m_EditorMode )
    {
        MaterialManager* pMaterialManager = GetManagers()->GetMaterialManager();
        pMaterialManager->SaveAllMaterials();
        //m_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs();
        m_Managers.GetSoundManager()->SaveAllCues();

        Editor_QuickSaveScene( "temp_editor_onplay.scene" );
        m_EditorMode = false;
        m_Paused = false;
        m_pComponentSystemManager->OnPlay( SCENEID_AllScenes );

        RegisterGameplayButtons();
    }
#endif
}

void EngineCore::OnModeStop()
{
#if MYFW_EDITOR
    if( m_EditorMode == false )
    {
        // If any object or components from the unmanaged/runtime scene are selected, unselect them.
        m_pEditorState->ClearSelectedObjectsAndComponentsFromScene( SCENEID_Unmanaged );

        // Call OnStop() for all components.
        m_pComponentSystemManager->OnStop( SCENEID_AllScenes );

        // Unload all runtime created objects.
        UnloadScene( SCENEID_Unmanaged, false );

        m_Paused = false;

        // Reload the scene objects from the state they were in before "play" was pressed.
        Editor_QuickLoadScene( "temp_editor_onplay.scene" );

        // Set to true after quick load, so any actions (e.g. changing material when loaded) won't be added to undo stack.
        m_EditorMode = true;

        m_pEditorState->ClearKeyAndActionStates();
        //m_pEditorState->ClearSelectedObjectsAndComponents();

        m_pComponentSystemManager->SyncAllRigidBodiesToObjectTransforms();

        UnregisterGameplayButtons();

        UnlockSystemMouse();

        return;
    }
#endif //MYFW_EDITOR
}

void EngineCore::OnModePause()
{
    if( m_EditorMode )
        OnModePlay();

    m_Paused = !m_Paused;
}

void EngineCore::OnModeAdvanceTime(float time)
{
    if( m_EditorMode )
        OnModePlay();

    m_Paused = true;
    m_PauseTimeToAdvance = time;
}

void EngineCore::RegisterGameplayButtons()
{
    this->m_KeyMappingToButtons['W'] = GCBI_Up;
    this->m_KeyMappingToButtons['A'] = GCBI_Left;
    this->m_KeyMappingToButtons['S'] = GCBI_Down;
    this->m_KeyMappingToButtons['D'] = GCBI_Right;

    this->m_KeyMappingToButtons['w'] = GCBI_Up;
    this->m_KeyMappingToButtons['a'] = GCBI_Left;
    this->m_KeyMappingToButtons['s'] = GCBI_Down;
    this->m_KeyMappingToButtons['d'] = GCBI_Right;

    this->m_KeyMappingToButtons[MYKEYCODE_UP] = GCBI_Up;
    this->m_KeyMappingToButtons[MYKEYCODE_LEFT] = GCBI_Left;
    this->m_KeyMappingToButtons[MYKEYCODE_DOWN] = GCBI_Down;
    this->m_KeyMappingToButtons[MYKEYCODE_RIGHT] = GCBI_Right;

    this->m_KeyMappingToButtons[MYKEYCODE_ESC] = GCBI_Back;

    if( MYKEYCODE_ENTER < 512 )
        this->m_KeyMappingToButtons[MYKEYCODE_ENTER] = GCBI_ButtonA;

    this->m_KeyMappingToButtons['Z'] = GCBI_ButtonA;
    this->m_KeyMappingToButtons['X'] = GCBI_ButtonB;
    this->m_KeyMappingToButtons['C'] = GCBI_ButtonC;
    this->m_KeyMappingToButtons['V'] = GCBI_ButtonD;

    this->m_KeyMappingToButtons['z'] = GCBI_ButtonA;
    this->m_KeyMappingToButtons['x'] = GCBI_ButtonB;
    this->m_KeyMappingToButtons['c'] = GCBI_ButtonC;
    this->m_KeyMappingToButtons['v'] = GCBI_ButtonD;
}

void EngineCore::UnregisterGameplayButtons()
{
    for( int i=0; i<GCBI_NumButtons; i++ )
        m_ButtonsHeld[i] = false;

    for( int i=0; i<255; i++ )
    {
        m_KeysHeld[i] = false;
        m_KeyMappingToButtons[i] = GCBI_NumButtons;
    }
}

#if MYFW_EDITOR
MySprite* EngineCore::GetSprite_DebugQuad()
{
    if( m_pSprite_DebugQuad == nullptr )
    {
        m_pSprite_DebugQuad = MyNew MySprite();
    }

    return m_pSprite_DebugQuad;
}

MyMesh* EngineCore::GetMesh_MaterialBall()
{
    if( m_pMesh_MaterialBall == nullptr )
    {
        m_pMesh_MaterialBall = MyNew MyMesh();
        m_pMesh_MaterialBall->SetMeshManagerAndAddToMeshList( g_pComponentSystemManager->GetGameCore()->GetManagers()->GetMeshManager() );
        m_pMesh_MaterialBall->SetLoadDefaultMaterials( false );
        MyAssert( m_pSphereMeshFile == nullptr );
        m_pSphereMeshFile = RequestFile( "Data/DataEngine/Meshes/sphere.obj.mymesh" );
        m_pSphereMeshFile->m_ShowInMemoryPanel = false;

        return nullptr;
    }

    if( m_pMesh_MaterialBall && m_pMesh_MaterialBall->IsReady() == false )
    {
        if( g_pEngineCore->m_pSphereMeshFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            m_pMesh_MaterialBall->SetSourceFile( g_pEngineCore->m_pSphereMeshFile );
        }

        return nullptr;
    }

    return m_pMesh_MaterialBall;
}

bool EngineCore::HandleEditorInput(int canvasid, int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    // Fill the imgui io structure.
    m_pImGuiManager->HandleInput( keyaction, keycode, mouseaction, id, x, y, pressure );

    // Pass all inputs to our imgui frame, which will deliver it to the correct window (game, editor or widget).
    bool inputUsed = ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->HandleInput( keyaction, keycode, mouseaction, id, x, y, pressure );

    // inputUsed will be false if the game window was in focus and the input event wasn't a hotkey.
    return inputUsed;
}
#endif //MYFW_EDITOR

void EngineCore::CreateDefaultEditorSceneObjects()
{
#if MYFW_EDITOR
    GameObject* pGameObject;
    ComponentCamera* pComponentCamera;
    ComponentMesh* pComponentMesh;

    // Create a 3D X/Z plane grid.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false, SCENEID_EngineObjects ); // Not managed.
        pGameObject->SetName( "3D Grid Plane" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, SCENEID_EngineObjects, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            if( g_pEngineCore->GetEditorPrefs() )
            {
                pComponentMesh->SetVisible( g_pEngineCore->GetEditorPrefs()->GetGridSettings()->visible );
            }

            pComponentMesh->SetMaterial( m_pMaterial_3DGrid, 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_Editor | Layer_EditorUnselectable );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->SetMeshManagerAndAddToMeshList( g_pComponentSystemManager->GetGameCore()->GetManagers()->GetMeshManager() );
            pComponentMesh->m_pMesh->CreateEditorLineGridXZ( Vector3(0,0,0), 1, 5 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();

            pComponentMesh->m_pSceneGraphObjects[0]->SetAsEditorObject();
        }

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        MyAssert( m_pEditorState->m_p3DGridPlane == nullptr );
        m_pEditorState->m_p3DGridPlane = pGameObject;
    }

    // Create a 3d transform gizmo for each axis.
    m_pEditorState->m_pTransformGizmo->CreateAxisObjects( SCENEID_EngineObjects, 0.03f, m_pEditorState );

    // Create a 3D editor camera, renders editor view.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false, SCENEID_EngineObjects ); // Not managed.
        pGameObject->SetName( "Editor Camera" );
#if MYFW_RIGHTHANDED
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, 10 ) );
#else
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, -10 ) );
#endif

        // Add an editor scene camera.
        {
            pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, SCENEID_EngineObjects, g_pComponentSystemManager );
            pComponentCamera->SetDesiredAspectRatio( 640, 960 );
            pComponentCamera->m_Orthographic = false;
            pComponentCamera->m_LayersToRender = Layer_Editor | Layer_MainScene;
            pComponentCamera->m_ClearColorBuffer = true;
            pComponentCamera->m_ClearDepthBuffer = true;

            // Add the camera component to the list, but disabled, so it won't render.
            pComponentCamera->SetEnabled( false );
            //m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        // Add a foreground camera for the transform gizmo only ATM.
        {
            pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, SCENEID_EngineObjects, g_pComponentSystemManager );
            pComponentCamera->SetDesiredAspectRatio( 640, 960 );
            pComponentCamera->m_Orthographic = false;
            pComponentCamera->m_LayersToRender = Layer_EditorFG;
            pComponentCamera->m_ClearColorBuffer = false;
            pComponentCamera->m_ClearDepthBuffer = true;

            // Add the camera component to the list, but disabled, so it won't render.
            pComponentCamera->SetEnabled( false );
            //m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        MyAssert( m_pEditorState->m_pEditorCamera == nullptr );
        m_pEditorState->m_pEditorCamera = pGameObject;
    }
#endif
}

void EngineCore::CreateDefaultSceneObjects()
{
    GameObject* pGameObject;
    ComponentCamera* pComponentCamera;

    // Create a 3D camera, renders first... created first so GetFirstCamera() will get the game cam.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( true, SCENEID_MainScene );
        pGameObject->SetName( "Main Camera" );
        pGameObject->SetFlags( 1<<0 );
#if MYFW_RIGHTHANDED
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, 10 ) );
#else
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, -10 ) );
#endif

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, SCENEID_MainScene, g_pComponentSystemManager );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = false;
        pComponentCamera->m_LayersToRender = Layer_MainScene;
    }

    // Create a 2D camera, renders after 3d, for hud.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( true, SCENEID_MainScene );
        pGameObject->SetName( "Hud Camera" );
        pGameObject->SetFlags( 1<<1 );

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, SCENEID_MainScene, g_pComponentSystemManager );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = true;
        pComponentCamera->m_LayersToRender = Layer_HUD;
        pComponentCamera->m_ClearColorBuffer = false;
        pComponentCamera->m_ClearDepthBuffer = false;
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void EngineCore::ReloadScene(SceneID sceneid)
{
    m_SceneReloadRequested = true;
}

void EngineCore::ReloadSceneInternal(SceneID sceneid)
{
    m_SceneReloadRequested = false;

    OnModeStop();
    char oldscenepath[MAX_PATH];
    sprintf_s( oldscenepath, MAX_PATH, "%s", g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_FullPath );
    UnloadScene( SCENEID_AllScenes, true );
    RequestedSceneInfo* pRequestedSceneInfo = RequestSceneInternal( oldscenepath );
    MyAssert( pRequestedSceneInfo );
    if( pRequestedSceneInfo )
    {
        pRequestedSceneInfo->m_SceneID = sceneid;
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void EngineCore::RequestScene(const char* fullpath)
{
    RequestSceneInternal( fullpath );
}

RequestedSceneInfo* EngineCore::RequestSceneInternal(const char* fullpath)
{
    // If the scene is already loaded, don't request it again.
    if( g_pComponentSystemManager->IsSceneLoaded( fullpath ) )
        return nullptr;

    // Check if scene is already queued up.
    {
        for( int i=0; i<MAX_SCENES_QUEUED_TO_LOAD; i++ )
        {
            MyFileObject* pFile = m_pSceneFilesLoading[i].m_pFile;
            if( pFile && strcmp( pFile->GetFullPath(), fullpath ) == 0 )
                return nullptr;
        }
    }

    int i;
    for( i=0; i<MAX_SCENES_QUEUED_TO_LOAD; i++ )
    {
        if( m_pSceneFilesLoading[i].m_pFile == nullptr )
            break;
    }
    
    MyAssert( i != MAX_SCENES_QUEUED_TO_LOAD ); // Too many scenes queued up.
    if( i == MAX_SCENES_QUEUED_TO_LOAD )
        return nullptr;

    m_pSceneFilesLoading[i].m_pFile = g_pEngineFileManager->RequestFile_UntrackedByScene( fullpath );
    m_pSceneFilesLoading[i].m_SceneID = SCENEID_NotSet;

    return &m_pSceneFilesLoading[i];
}

// Exposed to Lua, change elsewhere if function signature changes.
void EngineCore::SwitchScene(const char* fullpath)
{
    m_UnloadAllScenesNextTick = true;
    RequestSceneInternal( fullpath );
}

void EngineCore::SaveScene(const char* fullpath, SceneID sceneid)
{
    char* savestring = g_pComponentSystemManager->SaveSceneToJSON( sceneid );

    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t error = fopen_s( &filehandle, fullpath, "w" );
#else
    filehandle = fopen( fullpath, "w" );
#endif
    if( filehandle )
    {
        fprintf( filehandle, "%s", savestring );
        fclose( filehandle );
    }

    cJSONExt_free( savestring );
}

void EngineCore::SaveAllScenes()
{
    if( g_pEngineCore->IsInEditorMode() == false )
    {
        LOGInfo( LOGTag, "Scenes aren't saved when gameplay is active... use \"Save As\"\n" );
    }
    else
    {
        for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneID sceneid = (SceneID)i;
            SceneInfo* pSceneInfo = &g_pComponentSystemManager->m_pSceneInfoMap[i];

            if( sceneid != SCENEID_Unmanaged && sceneid != SCENEID_EngineObjects )
            {
                if( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath[0] == 0 )
                {
                    LOGInfo( LOGTag, "Untitled scene not saved.\n" );
                }
                else
                {
                    LOGInfo( LOGTag, "Saving scene... %s\n", pSceneInfo->m_FullPath );

#if MYFW_USING_IMGUI
                    g_pEngineCore->GetEditorMainFrame_ImGui()->StoreCurrentUndoStackSize();
#endif
                    g_pEngineCore->SaveScene( pSceneInfo->m_FullPath, sceneid );
                }
            }
        }
    }
}

void EngineCore::ExportBox2DScene(const char* fullpath, SceneID sceneid)
{
    char* savestring = g_pComponentSystemManager->ExportBox2DSceneToJSON( sceneid );

    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t error = fopen_s( &filehandle, fullpath, "w" );
#else
    filehandle = fopen( fullpath, "w" );
#endif
    if( filehandle )
    {
        fprintf( filehandle, "%s", savestring );
        fclose( filehandle );
    }

    cJSONExt_free( savestring );
}

void EngineCore::UnloadScene(SceneID sceneid, bool clearEditorObjects)
{
    // TODO: When unloading a single scene and gameplay is active, resetting the Lua state will break the game.
    //       This will need to be dealt with some other way,
    //           maybe individual GameObjects and Script components will need to unregister themselves from the Lua state when unloaded.
    {
        MyAssert( sceneid == SCENEID_AllScenes || m_EditorMode == false );
        m_pLuaGameState->Rebuild(); // Reset the lua state.
    }

    g_pComponentSystemManager->UnloadScene( sceneid, false );

    if( sceneid == SCENEID_AllScenes && m_FreeAllMaterialsAndTexturesWhenUnloadingScene )
    {
        // Temp code while RTQGlobals is a thing.
        SAFE_RELEASE( g_pRTQGlobals->m_pMaterial );
    }

    // Reset the editorstate structure.
#if MYFW_EDITOR
    if( sceneid != SCENEID_Unmanaged )
    {
        ((EditorMainFrame_ImGui*)m_pEditorMainFrame)->StoreCurrentUndoStackSize();
    }
    
    // TODO: Only unselect objects from the scene being unloaded.
    m_pEditorState->ClearEditorState( clearEditorObjects );
#endif //MYFW_EDITOR
}

#if MYFW_EDITOR
SceneID EngineCore::LoadSceneFromFile(const char* fullpath)
{
    SceneID sceneid = g_pComponentSystemManager->GetNextSceneID();

    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t err = fopen_s( &filehandle, fullpath, "rb" );
#else
    filehandle = fopen( fullpath, "rb" );
#endif

    char* jsonstr;

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long length = ftell( filehandle );
        if( length > 0 )
        {
            fseek( filehandle, 0, SEEK_SET );

            MyStackAllocator::MyStackPointer stackpointer;
            jsonstr = (char*)m_SingleFrameMemoryStack.AllocateBlock( length+1, &stackpointer );
            fread( jsonstr, length, 1, filehandle );
            jsonstr[length] = '\0';

            const char* filenamestart;
            int i;
            for( i=(int)strlen(fullpath)-1; i>=0; i-- )
            {
                if( fullpath[i] == '\\' || fullpath[i] == '/' )
                    break;
            }
            filenamestart = &fullpath[i+1];

            // If we're loading additional scenes, don't call "play",
            //     otherwise we need to decide if "play" should be called.
            bool playWhenFinishedLoading = false;

            if( sceneid != SCENEID_MainScene )
            {
                // Only start playing if the main scene is loaded, but secondary-scenes.
                playWhenFinishedLoading = false;
            }
#if MYFW_EDITOR
            else if( m_EditorMode )
            {
                playWhenFinishedLoading = m_AllowGameToRunInEditorMode;
            }
            else
#endif
            {
                playWhenFinishedLoading = true;
            }

            LoadSceneFromJSON( filenamestart, jsonstr, sceneid, playWhenFinishedLoading );

            g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_InUse = true;
            g_pComponentSystemManager->m_pSceneInfoMap[sceneid].ChangePath( fullpath );

            // Probably shouldn't bother with a rewind,
            //     might cause issues in future if LoadSceneFromJSON() uses stack and wants to keep info around.
            m_SingleFrameMemoryStack.RewindStack( stackpointer );
        }

        fclose( filehandle );
    }

    return sceneid;
}

void EngineCore::Editor_QuickSaveScene(const char* fullpath)
{
    char* savestring = g_pComponentSystemManager->SaveSceneToJSON( SCENEID_TempPlayStop );

    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t error = fopen_s( &filehandle, fullpath, "w" );
#else
    filehandle = fopen( fullpath, "w" );
#endif

    if( filehandle )
    {
        fprintf( filehandle, "%s", savestring );
        fclose( filehandle );
    }

    cJSONExt_free( savestring );
}

void EngineCore::Editor_QuickLoadScene(const char* fullpath)
{
    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t err = fopen_s( &filehandle, fullpath, "rb" );
#else
    filehandle = fopen( fullpath, "rb" );
#endif

    char* jsonstr;

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long length = ftell( filehandle );
        if( length > 0 )
        {
            fseek( filehandle, 0, SEEK_SET );

            MyStackAllocator::MyStackPointer stackpointer;
            jsonstr = (char*)m_SingleFrameMemoryStack.AllocateBlock( length+1, &stackpointer );
            fread( jsonstr, length, 1, filehandle );
            jsonstr[length] = '\0';

            // We're quickloading a temp scene, so don't call "play" in editor builds.
            bool playWhenFinishedLoading = false;

#if !MYFW_EDITOR
            playWhenFinishedLoading = true;
#endif

            LoadSceneFromJSON( fullpath, jsonstr, SCENEID_TempPlayStop, playWhenFinishedLoading );

            // Probably shouldn't bother with a rewind,
            //     might cause issues in future if LoadSceneFromJSON() uses stack and wants to keep info around.
            m_SingleFrameMemoryStack.RewindStack( stackpointer );
        }

        fclose( filehandle );
    }
}

void EngineCore::Editor_DeleteQuickScene(const char* fullpath)
{
    remove( fullpath );
}
#endif //MYFW_EDITOR

void EngineCore::LoadSceneFromJSON(const char* scenename, const char* jsonstr, SceneID sceneid, bool playWhenFinishedLoading)
{
    if( sceneid != SCENEID_TempPlayStop )
    {
        LOGInfo( LOGTag, "Loading scene file(%d): %s\n", sceneid, scenename );
    }

    // Reset the editorstate structure.
#if MYFW_EDITOR
    m_pEditorState->ClearEditorState( false );
#endif //MYFW_EDITOR

    g_pComponentSystemManager->LoadSceneFromJSON( scenename, jsonstr, sceneid );

    // Tell all the cameras loaded in the scene the dimensions of the window. // TODO: move this into camera's onload.
    OnSurfaceChanged( m_MainViewport.GetX(), m_MainViewport.GetY(), m_MainViewport.GetWidth(), m_MainViewport.GetHeight() );

    // FinishLoading calls OnLoad and OnPlay for all components in scene.
    g_pComponentSystemManager->FinishLoading( false, sceneid, playWhenFinishedLoading );
}

#if MYFW_EDITOR
void EngineCore::Editor_OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height)
{
    MyAssert( g_GLCanvasIDActive != 0 );

    if( g_GLCanvasIDActive != 0 )
    {
        if( m_pEditorState && m_pEditorState->m_pEditorCamera )
        {
            for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
            {
                ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
                MyAssert( pCamera != nullptr );
                if( pCamera )
                    pCamera->OnSurfaceChanged( x, y, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
            }

            m_pEditorState->OnSurfaceChanged( x, y, width, height );
        }
    }
}
#endif //MYFW_EDITOR

void EngineCore::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height)
{
#if MYFW_EDITOR
    if( g_GLCanvasIDActive == 1 )
    {
        Editor_OnSurfaceChanged( x, y, width, height );
        return;
    }
#endif //MYFW_EDITOR

    GameCore::OnSurfaceChanged( x, y, width, height );

    g_pRenderer->SetCullingEnabled( true );
#if !MYFW_RIGHTHANDED
    g_pRenderer->SetFrontFaceWinding( MyRE::FrontFaceWinding_Clockwise );
#endif
    g_pRenderer->SetDepthTestEnabled( true );

    if( height == 0 || width == 0 )
        return;

    // Reset the viewport sizes of the game or editor cameras.
    if( m_pComponentSystemManager )
    {
        m_pComponentSystemManager->OnSurfaceChanged( x, y, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
    }
}

#if MYFW_EDITOR
void EngineCore::RenderSingleObject(GameObject* pObject, FBODefinition* pFBOToUse)
{
    FBODefinition* pFBO = pFBOToUse;
    if( pFBO == nullptr )
        pFBO = m_pEditorState->m_pDebugViewFBO;

    // Render the scene to an FBO.
    pFBO->Bind( true );

    MyViewport viewport( 0, 0, pFBO->GetWidth(), pFBO->GetHeight() );
    g_pRenderer->EnableViewport( &viewport, true );

    g_pRenderer->SetClearColor( ColorFloat( 0, 0, 0, 0 ) );
    g_pRenderer->ClearBuffers( true, true, false );

    // Draw all editor camera components.
    ComponentCamera* pCamera = nullptr;
    for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        if( pCamera )
        {
            Vector3 objpos = pObject->GetTransform()->GetWorldPosition();
            Vector3 center(0,0,0);
            ComponentRenderable* pComponent = (ComponentRenderable*)pObject->GetFirstComponentOfType( "RenderableComponent" );
            if( pComponent )
            {
                MyAABounds* pBounds = pComponent->GetBounds();
                if( pBounds )
                    center = pBounds->GetCenter();
            }

            MyMatrix matView;
            matView.CreateLookAtView( objpos + center + Vector3(0, 2, -5), Vector3(0,1,0), objpos + center );

            MyMatrix matProj;
            float aspect = (float)pFBO->GetWidth() / pFBO->GetHeight();
            matProj.CreatePerspectiveHFoV( 45, aspect, 0.1f, 1000.0f );

            //MyMatrix matViewProj = matProj * matView;

            m_pComponentSystemManager->DrawSingleObject( &matProj, &matView, pObject, nullptr );

            g_pRenderer->ClearBuffers( false, true, false );
        }
    }

    pFBO->Unbind( true );
}

void EngineCore::GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end)
{
    ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
    MyViewport* pViewport = &pCamera->m_Viewport;

    // Convert mouse coord into clip space.
    Vector2 mouseClip;
    mouseClip.x = (mousepos.x / pViewport->GetWidth()) * 2.0f - 1.0f;
    mouseClip.y = (mousepos.y / pViewport->GetHeight()) * 2.0f - 1.0f;

    // Compute the inverse view projection matrix.
    MyMatrix invVP = ( pCamera->m_Camera3D.m_matProj * pCamera->m_Camera3D.m_matView ).GetInverse();

    // Store the camera position as the near world point (or alternatively, calculate world pos for near clip plane).
    //Vector4 nearClipPoint4 = Vector4( mouseClip, -1, 1 );
    //Vector4 nearWorldPoint4 = invVP * nearClipPoint4;
    //Vector3 nearWorldPoint = nearWorldPoint4.XYZ() / nearWorldPoint4.w;
    Vector3 nearWorldPoint = pCamera->m_pComponentTransform->GetWorldPosition();

    // Calculate the world position of the far clip plane where the mouse is pointing.
    Vector4 farClipPoint4 = Vector4( mouseClip, 1, 1 );
    Vector4 farWorldPoint4 = invVP * farClipPoint4;
    Vector3 farWorldPoint = farWorldPoint4.XYZ() / farWorldPoint4.w;

    //// Create a direction vector toward the far clip from the camera (or near clip).
    //Vector3 worldDir = farWorldPoint - nearWorldPoint;

    //// Calculate the intersection point for a given depth.  Ray => nearWorldPoint + worldDir * t
    //// First calculate 't' for a given depth, then calculate point at that 't' along the ray.
    //float t = (depthWanted - nearWorldPoint.z) / worldDir.z;
    //Vector3 intersectPoint = nearWorldPoint + (worldDir * t);

    *start = nearWorldPoint;
    *end = farWorldPoint;
}

void EngineCore::SetGridVisible(bool visible)
{
    if( m_pEditorState->m_p3DGridPlane )
    {
        ComponentMesh* pComponentMesh = (ComponentMesh*)m_pEditorState->m_p3DGridPlane->GetFirstComponentOfType( "MeshComponent" );
        pComponentMesh->SetVisible( visible );
    }
}

void EngineCore::SetEditorInterface(EditorInterfaceTypes type)
{
    MyAssert( type >= 0 && type < EditorInterfaceType_NumInterfaces );

    if( m_pCurrentEditorInterface )
        m_pCurrentEditorInterface->OnDeactivated();

    m_CurrentEditorInterfaceType = type;
    m_pCurrentEditorInterface = m_pEditorInterfaces[type];

    m_pCurrentEditorInterface->OnActivated();
}

EditorInterface* EngineCore::GetEditorInterface(EditorInterfaceTypes type)
{
    return m_pEditorInterfaces[type];
}

EditorInterface* EngineCore::GetCurrentEditorInterface()
{
    return m_pCurrentEditorInterface;
}
#endif //MYFW_EDITOR
