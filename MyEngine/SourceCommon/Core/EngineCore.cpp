//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EngineCore* g_pEngineCore = 0;

EngineCore::EngineCore()
{
    g_pEngineCore = this;

    m_pComponentSystemManager = 0;

#if MYFW_USING_WX
    m_EditorMode = true;
#else
    m_EditorMode = false;
#endif
    m_AllowGameToRunInEditorMode = false;

    m_Paused = false;

    m_TimeSinceLastPhysicsStep = 0;

    m_pShaderFile_TintColor = 0;
    m_pShaderFile_SelectedObjects = 0;
    m_pShaderFile_ClipSpaceTexture = 0;
    m_pShader_TintColor = 0;
    m_pShader_SelectedObjects = 0;
    m_pShader_ClipSpaceTexture = 0;
    m_pMaterial_Box2DDebugDraw = 0;
    m_pMaterial_3DGrid = 0;
    m_pMaterial_MousePicker = 0;
    m_pMaterial_ClipSpaceTexture = 0;

    m_GameWidth = 0;
    m_GameHeight = 0;

    m_UnloadAllScenesNextTick = false;
    m_SceneReloadRequested = false;
    for( int i=0; i<MAX_SCENE_FILES_QUEUED_UP; i++ )
    {
        m_pSceneFilesLoading[i].m_pFile = 0;
        m_pSceneFilesLoading[i].m_SceneID = -1;
    }

    m_pBulletWorld = 0;

#if MYFW_USING_LUA
    m_pLuaGameState = 0;
#endif //MYFW_USING_LUA

    m_PauseTimeToAdvance = 0;

    m_LastMousePos.Set( -1, -1 );

    m_Debug_DrawWireframe = false;

#if MYFW_USING_WX
    m_pEditorState = 0;
    m_Debug_DrawMousePickerFBO = false;
    m_Debug_DrawSelectedAnimatedMesh = false;
    m_Debug_DrawSelectedMaterial = false;
    m_Debug_DrawPhysicsDebugShapes = true;
    m_Debug_ShowProfilingInfo = true;
    m_Debug_DrawGLStats = false;
    m_pSphereMeshFile = 0;
    m_pSprite_DebugQuad = 0;
    m_pMesh_MaterialBall = 0;
    m_FreeAllMaterialsAndTexturesWhenUnloadingScene = false;
    m_pDebugFont = 0;
    m_pDebugTextMesh = 0;

    g_pPanelObjectList->m_pCallbackFunctionObject = this;
    g_pPanelObjectList->m_pOnTreeSelectionChangedFunction = StaticOnObjectListTreeSelectionChanged;
    g_pPanelObjectList->m_pOnTreeMultipleSelectionFunction = StaticOnObjectListTreeMultipleSelection;
    g_pPanelObjectList->m_pOnTreeDeleteSelectionFunction = StaticOnObjectListTreeDeleteSelection;

    m_pEditorInterfaces[EditorInterfaceType_SceneManagement] = MyNew EditorInterface_SceneManagement();
    m_pEditorInterfaces[EditorInterfaceType_2DPointEditor] = MyNew EditorInterface_2DPointEditor();
    m_pEditorInterfaces[EditorInterfaceType_VoxelMeshEditor] = MyNew EditorInterface_VoxelMeshEditor();
    m_CurrentEditorInterfaceType = EditorInterfaceType_NumInterfaces;
    m_pCurrentEditorInterface = 0;
#endif //MYFW_USING_WX

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
        m_GameObjectFlagStrings[i] = 0;
    }
}

EngineCore::~EngineCore()
{
    SAFE_DELETE( g_pImGuiManager );

    SAFE_DELETE( g_pRTQGlobals );

#if MYFW_USING_LUA
    SAFE_DELETE( m_pLuaGameState );
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
    for( int i=0; i<EditorInterfaceType_NumInterfaces; i++ )
        SAFE_DELETE( m_pEditorInterfaces[i] );
#endif //MYFW_USING_WX

    g_pFileManager->FreeFile( m_pShaderFile_TintColor );
    g_pFileManager->FreeFile( m_pShaderFile_SelectedObjects );
    g_pFileManager->FreeFile( m_pShaderFile_ClipSpaceTexture );
    SAFE_RELEASE( m_pShader_TintColor );
    SAFE_RELEASE( m_pShader_SelectedObjects );
    SAFE_RELEASE( m_pShader_ClipSpaceTexture );
    SAFE_RELEASE( m_pMaterial_Box2DDebugDraw );
    SAFE_RELEASE( m_pMaterial_3DGrid );
    SAFE_RELEASE( m_pMaterial_MousePicker );
    SAFE_RELEASE( m_pMaterial_ClipSpaceTexture );

#if MYFW_USING_WX
    SAFE_DELETE( m_pEditorState );
    SAFE_RELEASE( m_pSphereMeshFile );
    SAFE_RELEASE( m_pSprite_DebugQuad );
    SAFE_RELEASE( m_pMesh_MaterialBall );
    SAFE_RELEASE( m_pDebugFont );
    SAFE_RELEASE( m_pDebugTextMesh );
#endif //MYFW_USING_WX

    SAFE_DELETE( m_pComponentSystemManager );
    SAFE_DELETE( m_pBulletWorld );

    for( int i=0; i<32; i++ )
    {
        delete[] m_GameObjectFlagStrings[i];
    }

    m_SingleFrameMemoryStack.Cleanup();
}

// Helper functions for some global namespace lua binding.
void EngineCoreSetMousePosition(float x, float y)
{
    g_pEngineCore->SetMousePosition( x, y );
}

void EngineCoreSetMouseLock(bool lock)
{
    g_pEngineCore->SetMouseLock( lock );
}

#if MYFW_USING_LUA
void EngineCore::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<EngineCore>( "EngineCore" )
            .addFunction( "RequestScene", &EngineCore::RequestScene )
            .addFunction( "SwitchScene", &EngineCore::SwitchScene )
            .addFunction( "ReloadScene", &EngineCore::ReloadScene )
            //.addFunction( "SetMousePosition", &EngineCore::SetMousePosition )
        .endClass();
    
    luabridge::getGlobalNamespace( luastate ).addFunction( "SetMousePosition", EngineCoreSetMousePosition );    
    luabridge::getGlobalNamespace( luastate ).addFunction( "SetMouseLock", EngineCoreSetMouseLock );    
}
#endif //MYFW_USING_LUA

void EngineCore::InitializeManagers()
{
    if( g_pFileManager == 0 )
        g_pFileManager = MyNew EngineFileManager;

    GameCore::InitializeManagers();

    if( g_pRTQGlobals == 0 )
        g_pRTQGlobals = MyNew RenderTextQuickGlobals;

    if( g_pImGuiManager == 0 )
        g_pImGuiManager = MyNew ImGuiManager;
}

void EngineCore::InitializeGameObjectFlagStrings(cJSON* jStringsArray)
{
    if( jStringsArray == 0 )
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

    // allocate one meg of ram for now, this stack gets wiped each frame in OnDrawFrameDone()
    //  TODO: expose size, hardcoded to 5meg for now... used by editor mode scene load and voxel world creation
    m_SingleFrameMemoryStack.Initialize( 5000000 );

#if MYFW_USING_WX
    m_pEditorState = MyNew EditorState;

    m_pEditorState->m_pDebugViewFBO = g_pTextureManager->CreateFBO( 0, 0, GL_NEAREST, GL_NEAREST, false, 0, false, true );
    m_pEditorState->m_pMousePickerFBO = g_pTextureManager->CreateFBO( 0, 0, GL_NEAREST, GL_NEAREST, false, 0, false, true );

    if( m_pDebugFont == 0 )
    {
        m_pDebugFont = g_pFontManager->CreateFont( "Data/DataEngine/Fonts/Nevis60.fnt" );
    }

    if( m_pDebugTextMesh == 0 )
    {
        m_pDebugTextMesh = MyNew MyMeshText( 100, m_pDebugFont );
    }

    SetEditorInterface( EditorInterfaceType_SceneManagement );
#endif //MYFW_USING_WX

    // setup our shaders
    m_pShaderFile_TintColor = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_TintColor.glsl" );
    m_pShaderFile_SelectedObjects = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_SelectedObjects.glsl" );
    m_pShaderFile_ClipSpaceTexture = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_ClipSpaceTexture.glsl" );
    m_pShader_TintColor = MyNew ShaderGroup( m_pShaderFile_TintColor );
    m_pShader_SelectedObjects = MyNew ShaderGroup( m_pShaderFile_SelectedObjects );
    m_pShader_ClipSpaceTexture = MyNew ShaderGroup( m_pShaderFile_ClipSpaceTexture );
    m_pMaterial_Box2DDebugDraw = MyNew MaterialDefinition( m_pShader_TintColor, ColorByte(128,128,128,255) );
    m_pMaterial_3DGrid = MyNew MaterialDefinition( m_pShader_TintColor, ColorByte(128,128,128,255) );
    m_pMaterial_MousePicker = MyNew MaterialDefinition( m_pShader_ClipSpaceTexture );
    m_pMaterial_ClipSpaceTexture = MyNew MaterialDefinition( m_pShader_ClipSpaceTexture );

    // Initialize our component system.
    m_pComponentSystemManager = MyNew ComponentSystemManager( CreateComponentTypeManager() );

    // initialize lua state and register any variables needed.
#if MYFW_USING_LUA
    m_pLuaGameState = CreateLuaGameState();
    m_pLuaGameState->Rebuild(); // reset the lua state.
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
//    m_pComponentSystemManager->CreateNewScene( "Unsaved.scene", 1 );
    CreateDefaultEditorSceneObjects();

    // Initialize our editor interfaces (load materials, etc)
    for( int i=0; i<EditorInterfaceType_NumInterfaces; i++ )
    {
        m_pEditorInterfaces[i]->Initialize();
    }
#endif //MYFW_USING_WX

    // create the box2d world, pass in a material for the debug renderer.
    //ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
    //m_pBox2DWorld = MyNew Box2DWorld( m_pMaterial_Box2DDebugDraw, &pCamera->m_Camera3D.m_matViewProj, new EngineBox2DContactListener );

//    CreateDefaultSceneObjects();

    //OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );

    if( g_pImGuiManager )
    {
        g_pImGuiManager->Init();
#if MYFW_USING_WX
        // For editor build, start the next frame immediately, so imgui calls can be made in tick callbacks.
        // Tick happens before game(0) window is drawn, g_pImGuiManager's draw only happens on editor(1) window.
        g_pImGuiManager->StartFrame();
#endif //MYFW_USING_WX
    }

    // Create one bullet world shared between all scenes.
#if !MYFW_USING_WX
    // disable debug draw in non-editor builds
    m_pBulletWorld = MyNew BulletWorld( 0, 0 );
#else
    ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
    m_pBulletWorld = MyNew BulletWorld( m_pMaterial_Box2DDebugDraw, &pCamera->m_Camera3D.m_matViewProj );
#endif

#if !MYFW_USING_WX
    // TODO: fix! this won't work if flags were customized and saved into editorprefs.ini
    InitializeGameObjectFlagStrings( 0 );
#endif
}

bool EngineCore::IsReadyToRender()
{
    return true;
}

double EngineCore::Tick(double TimePassed)
{
    checkGlError( "EngineCore::Tick" );

#if MYFW_PROFILING_ENABLED
    static double Timing_LastFrameTime = 0;

    double Timing_Start = MyTime_GetSystemTime();
#endif

    if( g_pImGuiManager )
    {
        g_pImGuiManager->StartTick( TimePassed );
    }

    if( m_UnloadAllScenesNextTick )
    {
        UnloadScene( -1, true );
#if MYFW_USING_WX
        // If switching scene while in the editor, delete the quick save.
        Editor_DeleteQuickScene( "temp_editor_onplay.scene" );
#endif
        m_UnloadAllScenesNextTick = false;
        return 0;
    }

    if( m_SceneReloadRequested )
    {
        ReloadSceneInternal( 1 );
        m_SceneReloadRequested = false;
        return 0;
    }

#if MYFW_USING_WX
    m_pCurrentEditorInterface->Tick( TimePassed );

    EngineMainFrame_DumpCachedMessagesToLogPane();
#endif

    {
        static int numframes = 0;
        static double totaltime = 0;

        totaltime += TimePassed;
        numframes++;
        if( totaltime > 1 )
        {
            m_DebugFPS = (float)(numframes / totaltime);
            numframes = 0;
            totaltime = 0;
        }
    }

    if( TimePassed > 0.2 )
        TimePassed = 0.2;

    //if( TimePassed == 0 && m_EditorMode == false )
    //    LOGInfo( LOGTag, "Tick: %f\n", TimePassed );
    //LOGInfo( LOGTag, "Tick: %f\n", TimePassed );

    double TimeUnpaused = TimePassed;

    GameCore::Tick( TimePassed );

    // if the next scene requested is ready load the scene.
    MyFileObject* pFile = m_pSceneFilesLoading[0].m_pFile;
    if( pFile && pFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        unsigned int sceneid = g_pComponentSystemManager->GetNextSceneID();

        g_pComponentSystemManager->m_pSceneInfoMap[sceneid].Reset();
        g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_InUse = true;
        g_pComponentSystemManager->m_pSceneInfoMap[sceneid].ChangePath( pFile->GetFullPath() );

        // Loading an additional scene, or a lua script requested a scene.
        //     so if we're in editor mode, don't call "play" when loading is finished.
        bool playwhenfinishedloading = false;

#if MYFW_USING_WX
        if( m_EditorMode == true )
        {
            playwhenfinishedloading = false;
        }
        else
#endif
        {
            playwhenfinishedloading = true;
        }

        LoadSceneFromJSON( pFile->GetFilenameWithoutExtension(), pFile->GetBuffer(), sceneid, playwhenfinishedloading );

        SAFE_RELEASE( m_pSceneFilesLoading[0].m_pFile );

        // Shift all objects up a slot in the queue.
        for( int i=0; i<MAX_SCENE_FILES_QUEUED_UP-1; i++ )
        {
            m_pSceneFilesLoading[i] = m_pSceneFilesLoading[i+1];
        }
        m_pSceneFilesLoading[MAX_SCENE_FILES_QUEUED_UP-1].m_pFile = 0;
        m_pSceneFilesLoading[MAX_SCENE_FILES_QUEUED_UP-1].m_SceneID = 0;

#if !MYFW_USING_WX
        RegisterGameplayButtons();
#endif
    }

#if MYFW_USING_WX
    m_pEditorState->m_pTransformGizmo->Tick( TimePassed, m_pEditorState );
    m_pEditorState->UpdateCamera( TimePassed );
#endif

    // change timepassed if needed
    {
        float timescale = m_pComponentSystemManager->m_TimeScale;

        TimePassed *= timescale;

        if( m_EditorMode && m_AllowGameToRunInEditorMode == false )
            TimePassed = 0;

        if( m_Paused )
            TimePassed = m_PauseTimeToAdvance;

        m_PauseTimeToAdvance = 0;
    }

    if( m_EditorMode == false || m_AllowGameToRunInEditorMode )
    {
#if MYFW_PROFILING_ENABLED
        double Physics_Timing_Start = MyTime_GetSystemTime();
#endif // MYFW_PROFILING_ENABLED

        m_pBulletWorld->PhysicsUpdate( (float)TimePassed );

        m_TimeSinceLastPhysicsStep += TimePassed;
        while( m_TimeSinceLastPhysicsStep > 1/60.0f )
        {
            m_TimeSinceLastPhysicsStep -= 1/60.0f;
            //m_pBulletWorld->PhysicsStep();

            for( int i=0; i<g_pComponentSystemManager->MAX_SCENES_LOADED; i++ )
            {
                if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse && g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld )
                {
                    g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->PhysicsStep();
                    //g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->m_pWorld->ClearForces();
                }
            }
        }

#if MYFW_PROFILING_ENABLED
        double Physics_Timing_End = MyTime_GetSystemTime();

#if MYFW_USING_WX
        if( g_GLCanvasIDActive == 0 )
            m_FrameTimingInfo[m_FrameTimingNextEntry].Update_Physics = (float)((Physics_Timing_End - Physics_Timing_Start)*1000);
#else
#endif // MYFW_USING_WX
#endif // MYFW_PROFILING_ENABLED
    }

    // tick all components.
    m_pComponentSystemManager->Tick( TimePassed );

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

    // update the global unpaused time.
    if( m_EditorMode && m_AllowGameToRunInEditorMode == false )
        return TimeUnpaused;
    else
        return TimePassed;
}

void OnFileUpdated_CallbackFunction(MyFileObject* pFile)
{
#if MYFW_USING_WX
    g_pComponentSystemManager->OnFileUpdated( pFile );

    LOGInfo( LOGTag, "OnFileUpdated_CallbackFunction pFile = %s\n", pFile->GetFullPath() );

    if( strcmp( pFile->GetExtensionWithDot(), ".mymaterial" ) == 0 )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->FindMaterialByFilename( pFile->GetFullPath() );
        g_pMaterialManager->ReloadMaterial( pMaterial );
    }

    if( strcmp( pFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
    {
        MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
        // clear out the old mesh and load in the new one.
        pMesh->Clear();
    }
#endif

    // TODO: entitycomponentmanager-> tell all script components file is updated.
}

void EngineCore::OnFocusGained()
{
    GameCore::OnFocusGained();

#if MYFW_USING_WX
    m_pEditorState->ClearKeyAndActionStates();

    // check if any of the "source" files, like .fbx's were updated, they aren't loaded by FileManager so wouldn't be detected there.
    g_pComponentSystemManager->CheckForUpdatedDataSourceFiles( false );
#endif

    // reload any files that changed while we were out of focus.
    if( g_pFileManager == 0 )
        return;

    int filesupdated = g_pFileManager->ReloadAnyUpdatedFiles( OnFileUpdated_CallbackFunction );

    if( filesupdated )
    {
        //g_pShaderManager->InvalidateAllShaders( true );
        //g_pTextureManager->InvalidateAllTextures( true );
        //g_pBufferManager->InvalidateAllBuffers( true );
    }
}

void EngineCore::OnFocusLost()
{
    GameCore::OnFocusLost();

#if MYFW_USING_WX
    m_pEditorState->OnFocusLost();
#endif

    g_pImGuiManager->OnFocusLost();
}

void EngineCore::OnDrawFrameStart(unsigned int canvasid)
{
    GameCore::OnDrawFrameStart( canvasid );

#if !MYFW_USING_WX
    if( g_pImGuiManager )
    {
        g_pImGuiManager->StartFrame();
    }
#endif //!MYFW_USING_WX
}

void EngineCore::OnDrawFrame(unsigned int canvasid)
{
#if MYFW_PROFILING_ENABLED
    double Timing_Start = MyTime_GetSystemTime();
#endif

#if !MYFW_OPENGLES2
    if( m_Debug_DrawWireframe )
        glPolygonMode( GL_FRONT, GL_LINE );
#endif

    GameCore::OnDrawFrame( canvasid );

    MyRect windowrect( 0, 0, 0, 0 );

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 )
    {
        m_pCurrentEditorInterface->OnDrawFrame( canvasid );
        windowrect = m_pEditorState->m_EditorWindowRect;
    }
    else
#endif
    {
        // draw all components.
        m_pComponentSystemManager->OnDrawFrame();
        windowrect.Set( (int)m_WindowStartX, (int)m_WindowStartY, (int)m_WindowWidth, (int)m_WindowHeight );
    }

#if !MYFW_OPENGLES2
    if( m_Debug_DrawWireframe )
        glPolygonMode( GL_FRONT, GL_FILL );
#endif

#if MYFW_USING_WX
    if( m_Debug_DrawGLStats && m_pDebugTextMesh )// && g_GLCanvasIDActive == 1 )
    {
        if( m_pDebugTextMesh->GetMaterial( 0 ) == 0 )
        {
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( "Data/DataEngine/Materials/Nevis60.mymaterial" );
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

        MyMatrix mat;
        mat.CreateOrtho( (float)windowrect.x, (float)windowrect.x+windowrect.w, (float)windowrect.y, (float)windowrect.y+windowrect.h, -1, 1 );
        glDisable( GL_DEPTH_TEST );
        m_pDebugTextMesh->Draw( 0, &mat, 0,0,0,0,0,0,0,0 );
        glEnable( GL_DEPTH_TEST );
    }
#endif

    //glFinish();

#if MYFW_PROFILING_ENABLED
    double Timing_End = MyTime_GetSystemTime();

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 0 )
        m_FrameTimingInfo[m_FrameTimingNextEntry].Render_Game = (float)((Timing_End - Timing_Start)*1000);
    else if( g_GLCanvasIDActive == 1 )
        m_FrameTimingInfo[m_FrameTimingNextEntry].Render_Editor = (float)((Timing_End - Timing_Start)*1000);
#else
    m_FrameTimingInfo[m_FrameTimingNextEntry].Render_Game = (float)((Timing_End - Timing_Start)*1000);
#endif

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 && m_Debug_ShowProfilingInfo )
#endif
    {
        ImGui::SetNextWindowSize( ImVec2(150,50), ImGuiSetCond_FirstUseEver );
        ImGui::Begin( "Timing" );
        //ImGui::Text( "Hello world!" );
        //ImGui::SliderFloat( "", &m_AnimationTime, 0, 1, "Time: %.3f" );
    
        int numsamplestoshow = 60*5; // 5 seconds worth @ 60fps
        MyAssert( numsamplestoshow < MAX_FRAMES_TO_STORE );

        int numentries = m_FrameTimingNextEntry + 1;
        int start = m_FrameTimingNextEntry - numsamplestoshow;
        if( start < 0 )
            start = 0;
        if( numsamplestoshow > numentries )
            numsamplestoshow = numentries;

        m_FrameTimingNextEntry++;

        // sort of circular buffer the frame timings.
        if( m_FrameTimingNextEntry >= MAX_FRAMES_TO_STORE )
        {
            // copy the last "numsamples" entries back to the start and reset where we insert records.
            for( int i=0; i<numsamplestoshow; i++ )
                m_FrameTimingInfo[0+i] = m_FrameTimingInfo[MAX_FRAMES_TO_STORE-numsamplestoshow+i];

            m_FrameTimingNextEntry = numsamplestoshow;
        }

        //m_FrameTimingInfo.back().Tick = sin( (float)MyTime_GetSystemTime() );

        ImGui::PlotLines( "Frame Time",    &m_FrameTimingInfo[start].FrameTime,         numsamplestoshow, 0, "", 0.0f, 1000/20.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );

        ImGui::PlotLines( "Tick",          &m_FrameTimingInfo[start].Tick,              numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );

        ImGui::PlotLines( "Physics",       &m_FrameTimingInfo[start].Update_Physics,    numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );
#if MYFW_USING_WX
        ImGui::PlotLines( "Render Editor", &m_FrameTimingInfo[start].Render_Editor,     numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );
#endif
        ImGui::PlotLines( "Render Game",   &m_FrameTimingInfo[start].Render_Game,       numsamplestoshow, 0, "", 0.0f, 1000/60.0f, ImVec2(0,20), sizeof(FrameTimingInfo) );

        ImGui::End();
    }
#endif //MYFW_PROFILING_ENABLED

    if( g_pImGuiManager )
    {
#if MYFW_USING_WX
        // In editor builds, only draw imgui interface over editor window.
        if( canvasid == 1 )
#endif
        {
            g_pImGuiManager->EndFrame( (float)windowrect.w, (float)windowrect.h, true );

#if MYFW_USING_WX
            // For editor build, start the next frame immediately, so imgui calls can be made in tick callbacks.
            // Tick happens before game(0) window is drawn, g_pImGuiManager's draw only happens on editor(1) window.
            g_pImGuiManager->StartFrame();
#endif
        }
    }
}

void EngineCore::OnDrawFrameDone()
{
    GameCore::OnDrawFrameDone();

    m_SingleFrameStackSizeLastFrame = m_SingleFrameStackSizeThisFrame;
    m_SingleFrameStackSizeThisFrame = m_SingleFrameMemoryStack.GetBytesUsed();
    m_SingleFrameMemoryStack.Clear();

#if MYFW_USING_WX
    //if( g_GLCanvasIDActive == 1 )
#endif
    //{
    //    if( g_pImGuiManager )
    //        g_pImGuiManager->ClearInput();
    //}
}

void EngineCore::OnFileRenamed(const char* fullpathbefore, const char* fullpathafter)
{
    g_pComponentSystemManager->OnFileRenamed( fullpathbefore, fullpathafter );
}

bool EngineCore::OnEvent(MyEvent* pEvent)
{
    return g_pComponentSystemManager->OnEvent( pEvent );
}

void EngineCore::SetMousePosition(float x, float y)
{
    // TODO: get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    if( pCamera == 0 )
        return;

    // convert mouse to x/y in window space. TODO: put this in camera component.
    x = (x / m_GameWidth) * pCamera->m_Camera2D.m_ScreenWidth + pCamera->m_Camera2D.m_ScreenOffsetX + pCamera->m_WindowStartX;
    y = (y / m_GameHeight) * pCamera->m_Camera2D.m_ScreenHeight + pCamera->m_Camera2D.m_ScreenOffsetY + pCamera->m_WindowStartY;
    //x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - pCamera->m_WindowStartX) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
    //y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + pCamera->m_WindowStartY) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;

    // window space wants mouse at top left.
    y = pCamera->m_WindowHeight - y;

    PlatformSpecific_SetMousePosition( x, y );
}

bool EngineCore::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    if( GameCore::OnTouch( action, id, x, y, pressure, size ) )
        return true;

#if MYFW_USING_WX
    //if( m_EditorMode )
    {
        if( g_GLCanvasIDActive == 1 )
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
                    // prefer 0,0 at bottom left.
                    y = pCamera->m_WindowHeight - y;
                }

                if( HandleEditorInput( g_GLCanvasIDActive, -1, -1, action, id, x, y, pressure ) )
                    return true;
            }

            return false;
        }
    }

    if( g_GLCanvasIDActive != 0 )
    {
        return false;
    }
#endif

#if !MYFW_USING_WX
    if( g_pGameCore->IsMouseLocked() == false )
    {
        float toplefty = y;
        if( g_pImGuiManager->HandleInput( -1, -1, action, id, x, toplefty, pressure ) )
            return true;
    }
#endif

    // if mouse lock was requested, don't let mouse held messages go further.
    if( g_pGameCore->WasMouseLockRequested() && g_pGameCore->IsMouseLocked() == false && action == GCBA_Held )
    {
        return false;
    }

    // TODO: get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    if( pCamera == 0 )
        return false;

    // if the mouse is locked and it's a mouse held message, leave the x/y as is
    //     otherwise, convert to camera space.
    if( action == GCBA_RelativeMovement )
    {
        // x/y should be showing diffs in position, so leave them as is.
    }
    else
    {
        // prefer 0,0 at bottom left.
        y = pCamera->m_WindowHeight - y;

        // convert mouse to x/y in Camera2D space. TODO: put this in camera component.
        x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - pCamera->m_WindowStartX) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
        y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + pCamera->m_WindowStartY) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;
    }

    m_LastMousePos.Set( x, y );

    // mouse moving without button down.
    if( id == -1 )
        return false;

    return m_pComponentSystemManager->OnTouch( action, id, x, y, pressure, size );
}

bool EngineCore::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    GameCore::OnButtons( action, id );

    return m_pComponentSystemManager->OnButtons( action, id );
}

bool EngineCore::OnKeys(GameCoreButtonActions action, int keycode, int unicodechar)
{
#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 )
    {
        // not calling GameCore::OnKeys( action, keycode, unicodechar ) which translates keypresses to joystick input

        if( action == GCBA_Down && keycode == 344 ) // F5 )
        {
            int filesupdated = g_pFileManager->ReloadAnyUpdatedFiles( OnFileUpdated_CallbackFunction );

            if( filesupdated )
            {
                g_pShaderManager->InvalidateAllShaders( true );
                //g_pTextureManager->InvalidateAllTextures( true );
                //g_pBufferManager->InvalidateAllBuffers( true );
                return true;
            }
        }

        if( HandleEditorInput( g_GLCanvasIDActive, action, keycode, -1, -1, -1, -1, -1 ) )
            return true;
    }
#endif

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 0 )
#endif
    {
#if !MYFW_USING_WX
        if( g_pImGuiManager->HandleInput( action, keycode, -1, -1, -1, -1, -1 ) )
            return true;
#endif

        // GameCore::OnKeys translates keypresses to joystick input
        if( GameCore::OnKeys( action, keycode, unicodechar ) )
            return true;

        if( m_pComponentSystemManager->OnKeys( action, keycode, unicodechar ) )
            return true;
    }

    return false;
}

bool EngineCore::OnChar(unsigned int c)
{
#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 )
#endif
    {
        g_pImGuiManager->OnChar( c );
    }

    return false;
}

void EngineCore::OnModeTogglePlayStop()
{
#if MYFW_USING_WX
    if( m_EditorMode )
    {
        OnModePlay();
        
        // Set focus to gameplay window.
        if( g_pEngineMainFrame->Mode_SwitchFocusOnPlayStop() )
        {
            if( g_pEngineMainFrame->GetGLCanvasEditor()->GetParent() == g_pEngineMainFrame->GetFullScreenFrame() )
            {
                g_pEngineMainFrame->SetGLCanvasFullScreenMode( g_pEngineMainFrame->GetGLCanvas(), true );
            }
            else
            {
                g_pEngineMainFrame->m_pGLCanvas->SetFocus();
            }
        }
    }
    else
    {
        OnModeStop();

        // Set focus to editor window.
        if( g_pEngineMainFrame->Mode_SwitchFocusOnPlayStop() )
        {
            if( g_pEngineMainFrame->m_pGLCanvas->GetParent() == g_pEngineMainFrame->GetFullScreenFrame() )
            {
                g_pEngineMainFrame->SetGLCanvasFullScreenMode( g_pEngineMainFrame->GetGLCanvasEditor(), true );
            }
            else
            {
                g_pEngineMainFrame->GetGLCanvasEditor()->SetFocus();
            }
        }
    }
#endif //MYFW_USING_WX
}

void EngineCore::OnModePlay()
{
#if MYFW_USING_WX
    if( m_EditorMode )
    {
        g_pMaterialManager->SaveAllMaterials();
        //m_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs();
        m_pSoundManager->SaveAllCues();
        Editor_QuickSaveScene( "temp_editor_onplay.scene" );
        m_EditorMode = false;
        m_Paused = false;
        g_pEngineMainFrame->SetWindowPerspectiveToDefault();
        m_pComponentSystemManager->OnPlay( -1 );

        RegisterGameplayButtons();
    }
#endif
}

void EngineCore::OnModeStop()
{
#if MYFW_USING_WX
    if( m_EditorMode == false )
    {
        // Call OnStop() for all components.
        m_pComponentSystemManager->OnStop( -1 );

        // Unload all runtime created objects.
        UnloadScene( 0, false );

        m_Paused = false;

        // Reload the scene objects from the state they were in before "play" was pressed.
        Editor_QuickLoadScene( "temp_editor_onplay.scene" );

        // Set to true after quick load, so any actions (e.g. changing material when loaded) won't be added to undo stack.
        m_EditorMode = true;

        g_pEngineMainFrame->SetWindowPerspectiveToDefault();
        m_pEditorState->ClearKeyAndActionStates();
        //m_pEditorState->ClearSelectedObjectsAndComponents();

        m_pComponentSystemManager->SyncAllRigidBodiesToObjectTransforms();

        UnregisterGameplayButtons();
        return;
    }
#endif
}

void EngineCore::OnModePause()
{
    if( m_EditorMode )
        OnModePlay();

    m_Paused = !m_Paused;
}

void EngineCore::OnModeAdvanceTime(double time)
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

#if MYFW_USING_WX
MySprite* EngineCore::GetSprite_DebugQuad()
{
    if( m_pSprite_DebugQuad == 0 )
    {
        m_pSprite_DebugQuad = MyNew MySprite( false );
    }

    return m_pSprite_DebugQuad;
}

MyMesh* EngineCore::GetMesh_MaterialBall()
{
    if( m_pMesh_MaterialBall == 0 )
    {
        m_pMesh_MaterialBall = MyNew MyMesh();
        MyAssert( m_pSphereMeshFile == 0 );
        m_pSphereMeshFile = RequestFile( "Data/DataEngine/Meshes/sphere.obj.mymesh" );

        return 0;
    }

    if( m_pMesh_MaterialBall && m_pMesh_MaterialBall->IsReady() == false )
    {
        if( g_pEngineCore->m_pSphereMeshFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            m_pMesh_MaterialBall->SetSourceFile( g_pEngineCore->m_pSphereMeshFile );
        }

        return 0;
    }

    return m_pMesh_MaterialBall;
}

bool EngineCore::HandleEditorInput(int canvasid, int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    float toplefty = m_pEditorState->m_EditorWindowRect.h - y;
    if( g_pImGuiManager->HandleInput( keyaction, keycode, mouseaction, id, x, toplefty, pressure ) )
        return true;

    if( m_pCurrentEditorInterface->HandleInput( keyaction, keycode, mouseaction, id, x, y, pressure ) )
        return true;

    if( m_pEditorState->m_pTransformGizmo->HandleInput( this, -1, -1, mouseaction, id, x, y, pressure ) )
        return true;

    // clear modifier key and mouse button states.
    m_pCurrentEditorInterface->ClearModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    return false;
}
#endif //MYFW_USING_WX

void EngineCore::CreateDefaultEditorSceneObjects()
{
#if MYFW_USING_WX
    GameObject* pGameObject;
    ComponentCamera* pComponentCamera;
    ComponentMesh* pComponentMesh;

    // create a 3D X/Z plane grid
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false, ENGINE_SCENE_ID ); // not managed.
        pGameObject->SetName( "3D Grid Plane" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, ENGINE_SCENE_ID );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( g_pEngineMainFrame->GetGridSettings()->visible );
            pComponentMesh->SetMaterial( m_pMaterial_3DGrid, 0 ); //( m_pShader_TransformGizmo );
            pComponentMesh->SetLayersThisExistsOn( Layer_Editor | Layer_EditorUnselectable );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorLineGridXZ( Vector3(0,0,0), 1, 5 );
            // TODOMaterials: put this back for plane.
            //pComponentMesh->m_pMesh->m_Tint.Set( 150, 150, 150, 255 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        MyAssert( m_pEditorState->m_p3DGridPlane == 0 );
        m_pEditorState->m_p3DGridPlane = pGameObject;
    }

    // create a 3d transform gizmo for each axis.
    m_pEditorState->m_pTransformGizmo->CreateAxisObjects( ENGINE_SCENE_ID, 0.03f, m_pEditorState );

    // create a 3D editor camera, renders editor view.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false, ENGINE_SCENE_ID ); // not managed.
        pGameObject->SetName( "Editor Camera" );
#if MYFW_RIGHTHANDED
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, 10 ) );
#else
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, -10 ) );
#endif

        // add an editor scene camera
        {
            pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, ENGINE_SCENE_ID );
            pComponentCamera->SetDesiredAspectRatio( 640, 960 );
            pComponentCamera->m_Orthographic = false;
            pComponentCamera->m_LayersToRender = Layer_Editor | Layer_MainScene;
            pComponentCamera->m_ClearColorBuffer = true;
            pComponentCamera->m_ClearDepthBuffer = true;

            // add the camera component to the list, but disabled, so it won't render.
            pComponentCamera->SetEnabled( false );
            //m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        // add a foreground camera for the transform gizmo only ATM.
        {
            pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, ENGINE_SCENE_ID );
            pComponentCamera->SetDesiredAspectRatio( 640, 960 );
            pComponentCamera->m_Orthographic = false;
            pComponentCamera->m_LayersToRender = Layer_EditorFG;
            pComponentCamera->m_ClearColorBuffer = false;
            pComponentCamera->m_ClearDepthBuffer = true;

            // add the camera component to the list, but disabled, so it won't render.
            pComponentCamera->SetEnabled( false );
            //m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        MyAssert( m_pEditorState->m_pEditorCamera == 0 );
        m_pEditorState->m_pEditorCamera = pGameObject;
    }
#endif
}

void EngineCore::CreateDefaultSceneObjects()
{
    GameObject* pGameObject;
    ComponentCamera* pComponentCamera;

    // create a 3D camera, renders first... created first so GetFirstCamera() will get the game cam.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( true, 1 );
        pGameObject->SetName( "Main Camera" );
        pGameObject->SetFlags( 1<<0 );
#if MYFW_RIGHTHANDED
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, 10 ) );
#else
        pGameObject->GetTransform()->SetWorldPosition( Vector3( 0, 0, -10 ) );
#endif

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = false;
        pComponentCamera->m_LayersToRender = Layer_MainScene;
    }

    // create a 2D camera, renders after 3d, for hud.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( true, 1 );
        pGameObject->SetName( "Hud Camera" );
        pGameObject->SetFlags( 1<<1 );

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = true;
        pComponentCamera->m_LayersToRender = Layer_HUD;
        pComponentCamera->m_ClearColorBuffer = false;
        pComponentCamera->m_ClearDepthBuffer = false;
    }
}

void EngineCore::ReloadScene(unsigned int sceneid)
{
    m_SceneReloadRequested = true;
}

void EngineCore::ReloadSceneInternal(unsigned int sceneid)
{
    checkGlError( "start of ReloadSceneInternal" );

    m_SceneReloadRequested = false;

    OnModeStop();
    char oldscenepath[MAX_PATH];
    sprintf_s( oldscenepath, MAX_PATH, "%s", g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_FullPath );
    UnloadScene( -1, true );
    RequestedSceneInfo* pRequestedSceneInfo = RequestSceneInternal( oldscenepath );
    MyAssert( pRequestedSceneInfo );
    if( pRequestedSceneInfo )
    {
        pRequestedSceneInfo->m_SceneID = sceneid;
    }

    checkGlError( "end of ReloadSceneInternal" );
}

void EngineCore::RequestScene(const char* fullpath)
{
    RequestSceneInternal( fullpath );
}

RequestedSceneInfo* EngineCore::RequestSceneInternal(const char* fullpath)
{
    // if the scene is already loaded, don't request it again
    if( g_pComponentSystemManager->IsSceneLoaded( fullpath ) )
        return 0;

    // check if scene is already queued up
    {
        for( int i=0; i<MAX_SCENE_FILES_QUEUED_UP; i++ )
        {
            MyFileObject* pFile = m_pSceneFilesLoading[i].m_pFile;
            if( pFile && strcmp( pFile->GetFullPath(), fullpath ) == 0 )
                return 0;
        }
    }

    int i;
    for( i=0; i<MAX_SCENE_FILES_QUEUED_UP; i++ )
    {
        if( m_pSceneFilesLoading[i].m_pFile == 0 )
            break;
    }
    
    MyAssert( i != MAX_SCENE_FILES_QUEUED_UP ); // Too many scenes queued up.
    if( i == MAX_SCENE_FILES_QUEUED_UP )
        return 0;

    m_pSceneFilesLoading[i].m_pFile = g_pEngineFileManager->RequestFile_UntrackedByScene( fullpath );
    m_pSceneFilesLoading[i].m_SceneID = -1;

    return &m_pSceneFilesLoading[i];
}

void EngineCore::SwitchScene(const char* fullpath)
{
#if MYFW_USING_WX
    int answer = wxID_YES;

    if( m_pCommandStack->GetUndoStackSize() != g_pEngineMainFrame->GetUndoStackDepthAtLastSave() )
    {
        wxMessageDialog dlg( g_pEngineMainFrame, "Some changes aren't saved.\nLoad anyway?", "Confirm", wxYES_NO | wxNO_DEFAULT );
        dlg.SetYesNoLabels( "Load/Lose changes", "Return to editor" );
        answer = dlg.ShowModal();
    }

    if( answer == wxID_YES )
#endif
    {
        m_UnloadAllScenesNextTick = true;
        RequestSceneInternal( fullpath );
    }
}

void EngineCore::SaveScene(const char* fullpath, unsigned int sceneid)
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

void EngineCore::ExportBox2DScene(const char* fullpath, unsigned int sceneid)
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

void EngineCore::UnloadScene(unsigned int sceneid, bool cleareditorobjects)
{
    m_pLuaGameState->Rebuild(); // reset the lua state.

    g_pComponentSystemManager->UnloadScene( sceneid, false );

    if( sceneid == UINT_MAX && m_FreeAllMaterialsAndTexturesWhenUnloadingScene )
    {
        // temp code while RTQGlobals is a thing.
        SAFE_RELEASE( g_pRTQGlobals->m_pMaterial );
    }

    // reset the editorstate structure.
#if MYFW_USING_WX
    if( sceneid != 0 )
    {
        g_pEngineMainFrame->m_pCommandStack->ClearStacks();
        g_pEngineMainFrame->StoreCurrentUndoStackSize();
    }
    m_pEditorState->ClearEditorState( false );
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
unsigned int EngineCore::LoadSceneFromFile(const char* fullpath)
{
    unsigned int sceneid = g_pComponentSystemManager->GetNextSceneID();

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
            jsonstr[length] = 0;

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
            bool playwhenfinishedloading = false;

            if( sceneid != 1 )
            {
                playwhenfinishedloading = false;
            }
#if MYFW_USING_WX
            else if( m_EditorMode )
            {
                playwhenfinishedloading = m_AllowGameToRunInEditorMode;
            }
            else
#endif
            {
                playwhenfinishedloading = true;
            }

            LoadSceneFromJSON( filenamestart, jsonstr, sceneid, playwhenfinishedloading );

            g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_InUse = true;
            strcpy_s( g_pComponentSystemManager->m_pSceneInfoMap[sceneid].m_FullPath, MAX_PATH, fullpath );

            // Probably shouldn't bother with a rewind,
            //   might cause issues in future if LoadSceneFromJSON() uses stack and wants to keep info around
            m_SingleFrameMemoryStack.RewindStack( stackpointer );
        }

        fclose( filehandle );
    }

    return sceneid;
}

void EngineCore::Editor_QuickSaveScene(const char* fullpath)
{
    char* savestring = g_pComponentSystemManager->SaveSceneToJSON( UINT_MAX );

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
            jsonstr[length] = 0;

            // We're quickloading a temp scene, so don't call "play" in editor builds.
            bool playwhenfinishedloading = false;

#if !MYFW_USING_WX
            playwhenfinishedloading = true;
#endif

            LoadSceneFromJSON( fullpath, jsonstr, UINT_MAX, playwhenfinishedloading );

            // Probably shouldn't bother with a rewind,
            //   might cause issues in future if LoadSceneFromJSON() uses stack and wants to keep info around
            m_SingleFrameMemoryStack.RewindStack( stackpointer );
        }

        fclose( filehandle );
    }
}

void EngineCore::Editor_DeleteQuickScene(const char* fullpath)
{
    remove( fullpath );
}
#endif //MYFW_USING_WX

void EngineCore::LoadSceneFromJSON(const char* scenename, const char* jsonstr, unsigned int sceneid, bool playwhenfinishedloading)
{
    LOGInfo( LOGTag, "Loading scene file(%d): %s\n", sceneid, scenename );
    // reset the editorstate structure.
#if MYFW_USING_WX
    m_pEditorState->ClearEditorState( false );
#endif //MYFW_USING_WX

    g_pComponentSystemManager->LoadSceneFromJSON( scenename, jsonstr, sceneid );

    // Tell all the cameras loaded in the scene the dimensions of the window. // TODO: move this into camera's onload.
    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );

    // FinishLoading calls OnLoad and OnPlay for all components in scene.
    g_pComponentSystemManager->FinishLoading( false, sceneid, playwhenfinishedloading );

#if MYFW_USING_WX
    g_pEngineMainFrame->ResizeViewport();
#endif
}

#if MYFW_USING_WX
void EngineCore::Editor_OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    MyAssert( g_GLCanvasIDActive != 0 );

    if( g_GLCanvasIDActive != 0 )
    {
        if( m_pEditorState && m_pEditorState->m_pEditorCamera )
        {
            for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
            {
                ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
                MyAssert( pCamera != 0 );
                if( pCamera )
                    pCamera->OnSurfaceChanged( startx, starty, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
            }

            m_pEditorState->OnSurfaceChanged( startx, starty, width, height );
        }
    }
}
#endif //MYFW_USING_WX

void EngineCore::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 )
    {
        Editor_OnSurfaceChanged( startx, starty, width, height );
        return;
    }
#endif //MYFW_USING_WX

    GameCore::OnSurfaceChanged( startx, starty, width, height );

    glEnable( GL_CULL_FACE );
#if !MYFW_RIGHTHANDED
    glFrontFace( GL_CW );
#endif
    glEnable( GL_DEPTH_TEST );

    if( height == 0 || width == 0 )
        return;

    //float devicewidth = m_WindowWidth;
    //float deviceheight = m_WindowHeight;
    //float deviceratio = devicewidth / deviceheight;

    //if( width > height )
    //{
    //    m_GameWidth = 960.0f;
    //    m_GameHeight = 640.0f;
    //}
    //else if( height > width )
    //{
    //    m_GameWidth = 640.0f;
    //    m_GameHeight = 960.0f;
    //}
    //else
    //{
    //    m_GameWidth = 640.0f;
    //    m_GameHeight = 640.0f;
    //}

    //m_GameWidth = 640.0f;
    //m_GameHeight = 960.0f;

    // reset the viewport sizes of the game or editor cameras.
    if( m_pComponentSystemManager )
    {
#if MYFW_USING_WX
        MyAssert( g_GLCanvasIDActive == 0 );
#endif
        {
            m_pComponentSystemManager->OnSurfaceChanged( startx, starty, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
        }
    }
}

#if MYFW_USING_WX
void EngineCore::RenderSingleObject(GameObject* pObject)
{
    // render the scene to an FBO.
    m_pEditorState->m_pDebugViewFBO->Bind( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, m_pEditorState->m_pDebugViewFBO->m_Width, m_pEditorState->m_pDebugViewFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw all editor camera components.
    ComponentCamera* pCamera = 0;
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
            float aspect = (float)m_pEditorState->m_pDebugViewFBO->m_Width / m_pEditorState->m_pDebugViewFBO->m_Height;
            matProj.CreatePerspectiveHFoV( 45, aspect, 0.1f, 1000.0f );

            MyMatrix matViewProj = matProj * matView;

            m_pComponentSystemManager->DrawSingleObject( &matViewProj, pObject, 0 );

            glClear( GL_DEPTH_BUFFER_BIT );
        }
    }

    m_pEditorState->m_pDebugViewFBO->Unbind( true );
}

void EngineCore::GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end)
{
    ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();

    // Convert mouse coord into clip space.
    Vector2 mouseclip;
    mouseclip.x = (mousepos.x / pCamera->m_WindowWidth) * 2.0f - 1.0f;
    mouseclip.y = (mousepos.y / pCamera->m_WindowHeight) * 2.0f - 1.0f;

    // Convert the mouse ray into view space from clip space.
    MyMatrix invProj = pCamera->m_Camera3D.m_matProj;
    invProj.Inverse();
    Vector4 rayview = invProj * Vector4( mouseclip, -1, 1 );

    // Convert the mouse ray into world space from view space.
    MyMatrix invView = pCamera->m_Camera3D.m_matView;
    invView.Inverse();
#if MYFW_RIGHTHANDED
    Vector3 rayworld = (invView * Vector4( rayview.x, rayview.y, -1, 0 )).XYZ();
#else
    Vector3 rayworld = (invView * Vector4( rayview.x, rayview.y, 1, 0 )).XYZ();
#endif

    // define the ray.
    Vector3 raystart = pCamera->m_pComponentTransform->GetLocalPosition();
    Vector3 rayend = raystart + rayworld * 10000;

    start->Set( raystart.x, raystart.y, raystart.z );
    end->Set( rayend.x, rayend.y, rayend.z );
}

void EngineCore::SetGridVisible(bool visible)
{
    if( m_pEditorState->m_p3DGridPlane )
    {
        ComponentMesh* pComponentMesh = (ComponentMesh*)m_pEditorState->m_p3DGridPlane->GetFirstComponentOfType( "MeshComponent" );
        pComponentMesh->SetVisible( g_pEngineMainFrame->GetGridSettings()->visible );
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
#endif //MYFW_USING_WX

#if MYFW_USING_WX
void EngineCore::OnObjectListTreeSelectionChanged() //StaticOnObjectListTreeSelectionChanged
{
    if( m_pEditorState )
    {
        //LOGInfo( LOGTag, "Clearing Selected Objects\n" );
        m_pEditorState->m_pSelectedObjects.clear();
        m_pEditorState->m_pSelectedComponents.clear();
    }
}

void EngineCore::OnObjectListTreeMultipleSelection(bool prepForDraggingCopy) //StaticOnObjectListTreeMultipleSelection
{
    bool allowFolderSelection = false;
    bool selectContentsOfSelectedFolders = true;
    bool allowPrefabSelection = true;
    bool updateWatchWindowWithNewSelection = true;
    bool allowChildrenToBeSelectedIfParentIsSelected = true;
    
    if( prepForDraggingCopy == true )
    {
        allowFolderSelection = true;
        selectContentsOfSelectedFolders = false;
        allowPrefabSelection = false;
        updateWatchWindowWithNewSelection = false;
        allowChildrenToBeSelectedIfParentIsSelected = false;
    }

    if( m_pEditorState == 0 )
        return;

    g_pPanelWatch->ClearAllVariables();
    OnObjectListTreeSelectionChanged();

    wxArrayTreeItemIds selecteditems;
    unsigned int numselected = (unsigned int)g_pPanelObjectList->m_pTree_Objects->GetSelections( selecteditems );

    if( numselected == 0 )
        return;

    // If only 1 item is selected and it's a GameObject(not a folder), show just this one GameObject.
    if( numselected == 1 )
    {
        wxTreeItemId id = selecteditems[0].GetID();
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)g_pPanelObjectList->m_pTree_Objects->GetItemData( id );

        MyAssert( pData && pData->m_pObject );

        GameObject* pGameObject = 0;
        bool isprefab = false;
        
        if( pData->m_pLeftClickFunction == GameObject::StaticOnLeftClick )
        {
            pGameObject = (GameObject*)pData->m_pObject;
        }
        else if( pData->m_pLeftClickFunction == PrefabObject::StaticOnLeftClick )
        {
            // Allow prefabs in watch panel.
            isprefab = true;
            pGameObject = (GameObject*)pData->m_pUserData;
        }

        // Show this GameObject in the watch panel, if it's not a folder
        if( updateWatchWindowWithNewSelection )
        {
            if( pGameObject && pGameObject->IsFolder() == false )
            {
                pGameObject->ShowInWatchPanel( isprefab );
                return;
            }
        }
    }

    // Add all selected GameObjects to editor selection list.
    for( unsigned int i=0; i<numselected; i++ )
    {
        wxTreeItemId id = selecteditems[i].GetID();
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)g_pPanelObjectList->m_pTree_Objects->GetItemData( id );

        MyAssert( pData && pData->m_pObject );

        GameObject* pGameObject = 0;
        
        if( pData->m_pLeftClickFunction == GameObject::StaticOnLeftClick )
        {
            pGameObject = (GameObject*)pData->m_pObject;
        }
        else if( pData->m_pLeftClickFunction == PrefabObject::StaticOnLeftClick )
        {
            // Don't allow multi-selection of prefabs in watch panel.
            // TODO: revisit this...
            //pGameObject = ((PrefabObject*)pData->m_pObject)->GetGameObject();
            continue;
        }

        if( pGameObject )
        {
            // Skip children if their parents are selected.
            if( allowChildrenToBeSelectedIfParentIsSelected == false )
            {
                bool parentfound = false;

                for( unsigned int i=0; i<numselected; i++ )
                {
                    wxTreeItemId id = selecteditems[i].GetID();
                    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)g_pPanelObjectList->m_pTree_Objects->GetItemData( id );
                    MyAssert( pData && pData->m_pObject );

                    // Only check gameobjects, not prefabs.
                    if( pData->m_pLeftClickFunction == GameObject::StaticOnLeftClick )
                    {
                        if( pGameObject->IsParentedTo( (GameObject*)pData->m_pObject, false ) )
                        {
                            parentfound = true;
                        }
                    }
                }

                if( parentfound )
                {
                    // Skip this child.
                    continue;
                }
            }

            // Select the GameObject (also select folders if allowed).
            if( allowFolderSelection || pGameObject->IsFolder() == false )
            {
                if( m_pEditorState->IsGameObjectSelected( pGameObject ) == false )
                    m_pEditorState->m_pSelectedObjects.push_back( pGameObject );
            }

            // If this is a folder, select all objects inside.
            if( selectContentsOfSelectedFolders && pGameObject->IsFolder() )
            {
                for( CPPListNode* pNode = pGameObject->GetChildList()->GetHead(); pNode; pNode = pNode->GetNext() )
                {
                    GameObject* pGameObject = (GameObject*)pNode;

                    if( pGameObject->IsManaged() )
                    {
                        pGameObject->AddToList( &m_pEditorState->m_pSelectedObjects );
                    }
                }
            }
        }
    }

    // Update watch panel with multiple selections.
    if( updateWatchWindowWithNewSelection )
    {
        // Add a spacer saying how many objects were selected.
        char temp[30];
        sprintf_s( temp, 30, "%d objects selected", m_pEditorState->m_pSelectedObjects.size() );
        g_pPanelWatch->AddSpace( temp, 0, 0, 0 );

        // Show common components of all selected Gameobjects:
        if( m_pEditorState->m_pSelectedObjects.size() > 0 )
        {
            GameObject* pFirstGameObject = m_pEditorState->m_pSelectedObjects[0];

            // Search all components including GameObject properties and transform.
            for( unsigned int i=0; i<pFirstGameObject->GetComponentCountIncludingCore(); i++ )
            {
                ComponentBase* pComponentToLookFor = pFirstGameObject->GetComponentByIndexIncludingCore( i );

                MyAssert( pComponentToLookFor );

                pComponentToLookFor->m_MultiSelectedComponents.clear();

                // Loop through selected gameobjects and check if they all have to least one of this component type on them.
                bool allgameobjectshavecomponent = true;
                for( unsigned int i=1; i<m_pEditorState->m_pSelectedObjects.size(); i++ )
                {
                    GameObject* pGameObject = m_pEditorState->m_pSelectedObjects[i];

                    bool hascomponent = false;
                    for( unsigned int i=0; i<pGameObject->GetComponentCountIncludingCore(); i++ )
                    {
                        ComponentBase* pOtherComponent = pGameObject->GetComponentByIndexIncludingCore( i );

                        if( pOtherComponent && pOtherComponent->IsA( pComponentToLookFor->GetClassname() ) == true )
                        {
                            pComponentToLookFor->m_MultiSelectedComponents.push_back( pOtherComponent );
                            hascomponent = true;
                            break;
                        }
                    }

                    if( hascomponent == false )
                    {
                        allgameobjectshavecomponent = false;
                        break;
                    }
                }

                if( allgameobjectshavecomponent == true )
                {
                    pComponentToLookFor->OnLeftClick( (unsigned int)m_pEditorState->m_pSelectedObjects.size(), false );
                }
            }
        }
    }
}

void EngineCore::OnObjectListTreeDeleteSelection() //StaticOnObjectListTreeDeleteSelection
{
    if( m_pEditorState == 0 )
        return;

    g_pPanelWatch->ClearAllVariables();
    OnObjectListTreeSelectionChanged();

    wxArrayTreeItemIds selecteditems;
    unsigned int numselected = (unsigned int)g_pPanelObjectList->m_pTree_Objects->GetSelections( selecteditems );

    if( numselected == 0 )
        return;

    // Add all selected GameObjects to editor selection list.
    for( unsigned int i=0; i<numselected; i++ )
    {
        wxTreeItemId id = selecteditems[i].GetID();
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)g_pPanelObjectList->m_pTree_Objects->GetItemData( id );

        MyAssert( pData && pData->m_pObject );

        GameObject* pGameObject = 0;
        
        if( pData->m_pLeftClickFunction == GameObject::StaticOnLeftClick )
            pGameObject = (GameObject*)pData->m_pObject;
        else if( pData->m_pLeftClickFunction == PrefabObject::StaticOnLeftClick )
            pGameObject = ((PrefabObject*)pData->m_pObject)->GetGameObject();

        if( pGameObject )
        {
            pGameObject->AddToList( &m_pEditorState->m_pSelectedObjects );
        }
    }

    // Delete the current selected gameobjects.
    m_pEditorState->DeleteSelectedObjects();
}
#endif //MYFW_USING_WX
