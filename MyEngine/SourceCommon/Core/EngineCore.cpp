//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
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
    m_pShaderFile_ClipSpaceTexture = 0;
    m_pShader_TintColor = 0;
    m_pShader_ClipSpaceTexture = 0;
    m_pMaterial_3DGrid = 0;
    m_pMaterial_TransformGizmoX = 0;
    m_pMaterial_TransformGizmoY = 0;
    m_pMaterial_TransformGizmoZ = 0;
    m_pMaterial_MousePicker = 0;
    m_pMaterial_ClipSpaceTexture = 0;

    m_GameWidth = 0;
    m_GameHeight = 0;

    m_pSceneFileToLoad = 0;
    m_SceneLoaded = false;

    m_pBulletWorld = MyNew BulletWorld();

#if MYFW_USING_LUA
    m_pLuaGameState = 0;
#endif //MYFW_USING_LUA

    m_PauseTimeToAdvance = 0;

    m_LastMousePos.Set( -1, -1 );

#if MYFW_USING_WX
    m_pEditorState = 0;
    m_Debug_DrawMousePickerFBO = false;
    m_Debug_DrawSelectedAnimatedMesh = false;
    m_Debug_DrawSelectedMaterial = false;
    m_Debug_DrawGLStats = false;
    m_pSphereMeshFile = 0;
    m_pDebugQuadSprite = 0;
    m_pMaterialBallMesh = 0;
    m_FreeAllMaterialsAndTexturesWhenUnloadingScene = false;
    m_pDebugFont = 0;
    m_pDebugTextMesh = 0;

    g_pPanelObjectList->m_pCallbackFunctionObject = this;
    g_pPanelObjectList->m_pOnTreeSelectionChangedFunction = StaticOnObjectListTreeSelectionChanged;
#endif //MYFW_USING_WX

    m_DebugFPS = 0;
    m_LuaMemoryUsedLastFrame = 0;
    m_LuaMemoryUsedThisFrame = 0;
}

EngineCore::~EngineCore()
{
    SAFE_DELETE( g_pRTQGlobals );

#if MYFW_USING_LUA
    SAFE_DELETE( m_pLuaGameState );
#endif //MYFW_USING_LUA

    g_pFileManager->FreeFile( m_pShaderFile_TintColor );
    g_pFileManager->FreeFile( m_pShaderFile_ClipSpaceTexture );
    SAFE_RELEASE( m_pShader_TintColor );
    SAFE_RELEASE( m_pShader_ClipSpaceTexture );
    SAFE_RELEASE( m_pMaterial_3DGrid );
    SAFE_RELEASE( m_pMaterial_TransformGizmoX );
    SAFE_RELEASE( m_pMaterial_TransformGizmoY );
    SAFE_RELEASE( m_pMaterial_TransformGizmoZ );
    SAFE_RELEASE( m_pMaterial_MousePicker );
    SAFE_RELEASE( m_pMaterial_ClipSpaceTexture );

#if MYFW_USING_WX
    SAFE_DELETE( m_pEditorState );
    SAFE_RELEASE( m_pSphereMeshFile );
    SAFE_RELEASE( m_pDebugQuadSprite );
    SAFE_RELEASE( m_pMaterialBallMesh );
    SAFE_RELEASE( m_pDebugFont );
    SAFE_RELEASE( m_pDebugTextMesh );
#endif //MYFW_USING_WX

    SAFE_DELETE( m_pComponentSystemManager );
    SAFE_DELETE( m_pBulletWorld );
}

#if MYFW_USING_LUA
void EngineCore::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<EngineCore>( "EngineCore" )
            .addFunction( "LoadSceneFromFile", &EngineCore::LoadSceneFromFile )
        .endClass();
}
#endif //MYFW_USING_LUA

void EngineCore::InitializeManagers()
{
    if( g_pFileManager == 0 )
        g_pFileManager = MyNew EngineFileManager;

    GameCore::InitializeManagers();

    if( g_pRTQGlobals == 0 )
        g_pRTQGlobals = MyNew RenderTextQuickGlobals;
}

void EngineCore::OneTimeInit()
{
    GameCore::OneTimeInit();

#if MYFW_USING_WX
    m_pEditorState = MyNew EditorState;

    m_pEditorState->m_pDebugViewFBO = g_pTextureManager->CreateFBO( 0, 0, GL_NEAREST, GL_NEAREST, false, 0, false, true );
    m_pEditorState->m_pMousePickerFBO = g_pTextureManager->CreateFBO( 0, 0, GL_NEAREST, GL_NEAREST, false, 0, false, true );

    if( m_pDebugFont == 0 )
    {
        m_pDebugFont = g_pFontManager->CreateFont( "DataEngine/Fonts/Nevis60.fnt" );
    }

    if( m_pDebugTextMesh == 0 )
    {
        m_pDebugTextMesh = MyNew MyMeshText( 100, m_pDebugFont );
    }
#endif //MYFW_USING_WX

    // setup our shaders
    m_pShaderFile_TintColor = RequestFile( "DataEngine/Shaders/Shader_TintColor.glsl" );
    m_pShaderFile_ClipSpaceTexture = RequestFile( "DataEngine/Shaders/Shader_ClipSpaceTexture.glsl" );
    m_pShader_TintColor = MyNew ShaderGroup( m_pShaderFile_TintColor );
    m_pShader_ClipSpaceTexture = MyNew ShaderGroup( m_pShaderFile_ClipSpaceTexture );
    m_pMaterial_3DGrid = MyNew MaterialDefinition( m_pShader_TintColor, ColorByte(128,128,128,255) );
    m_pMaterial_TransformGizmoX = MyNew MaterialDefinition( m_pShader_TintColor, ColorByte(255,0,0,255) );
    m_pMaterial_TransformGizmoY = MyNew MaterialDefinition( m_pShader_TintColor, ColorByte(0,255,0,255) );
    m_pMaterial_TransformGizmoZ = MyNew MaterialDefinition( m_pShader_TintColor, ColorByte(0,0,255,255) );
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
#endif //MYFW_USING_WX

//    CreateDefaultSceneObjects();

    //OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );
}

bool EngineCore::IsReadyToRender()
{
    return true;
}

double EngineCore::Tick(double TimePassed)
{
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

#if !MYFW_USING_WX
    if( m_SceneLoaded == false && m_pSceneFileToLoad && m_pSceneFileToLoad->m_FileLoadStatus == FileLoadStatus_Success )
    {
        LoadScene( 0, m_pSceneFileToLoad->m_pBuffer, 1 );
        //m_pComponentSystemManager->OnPlay();
        RegisterGameplayButtons();
        SAFE_RELEASE( m_pSceneFileToLoad );
    }
#endif

#if MYFW_USING_WX
    m_pEditorState->m_pTransformGizmo->Tick( TimePassed, m_pEditorState );
    m_pEditorState->UpdateCamera( TimePassed );
#endif

    // tick all components.
    {
        if( m_EditorMode && m_AllowGameToRunInEditorMode == false )
            TimePassed = 0;

        if( m_Paused )
            TimePassed = m_PauseTimeToAdvance;

        m_PauseTimeToAdvance = 0;

        m_pComponentSystemManager->Tick( TimePassed );
    }

    if( m_EditorMode == false || m_AllowGameToRunInEditorMode )
    {
        m_TimeSinceLastPhysicsStep += TimePassed;

        while( m_TimeSinceLastPhysicsStep > 1/60.0f )
        {
            m_TimeSinceLastPhysicsStep -= 1/60.0f;
            m_pBulletWorld->PhysicsStep();
            //LOGInfo( LOGTag, "m_pBulletWorld->PhysicsStep()\n" );
        }
    }

#if MYFW_USING_LUA
    if( g_pLuaGameState && g_pLuaGameState->m_pLuaState )
    {
        int luamemcountk = lua_gc( g_pLuaGameState->m_pLuaState, LUA_GCCOUNT, 0 );
        int luamemcountb = lua_gc( g_pLuaGameState->m_pLuaState, LUA_GCCOUNTB, 0 );
        m_LuaMemoryUsedLastFrame = m_LuaMemoryUsedThisFrame;
        m_LuaMemoryUsedThisFrame = luamemcountk*1024 + luamemcountb;
    }
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

    LOGInfo( LOGTag, "OnFileUpdated_CallbackFunction pFile = %s\n", pFile->m_FullPath );

    if( strcmp( pFile->m_ExtensionWithDot, ".mymaterial" ) == 0 )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->FindMaterialByFilename( pFile->m_FullPath );
        g_pMaterialManager->ReloadMaterial( pMaterial );
    }

    if( strcmp( pFile->m_ExtensionWithDot, ".mymesh" ) == 0 )
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
    int filesupdated = g_pFileManager->ReloadAnyUpdatedFiles( OnFileUpdated_CallbackFunction );

    if( filesupdated )
    {
        g_pShaderManager->InvalidateAllShaders( true );
        //g_pTextureManager->InvalidateAllTextures( true );
        //g_pBufferManager->InvalidateAllBuffers( true );
    }
}

void EngineCore::OnFocusLost()
{
    GameCore::OnFocusLost();
}

void EngineCore::OnDrawFrame(unsigned int canvasid)
{
    GameCore::OnDrawFrame( canvasid );

#if MYFW_USING_WX
    ComponentCamera* pEditorCamera = 0;
    if( g_GLCanvasIDActive == 1 )
    {
        MyAssert( m_pEditorState->m_pEditorCamera );
        if( m_pEditorState->m_pEditorCamera )
        {
            // draw editor camera and editorFG camera
            for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
            {
                pEditorCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );

                if( pEditorCamera )
                    pEditorCamera->OnDrawFrame();
            }
        }
    }
    else
#endif
    {
        // draw all components.
        m_pComponentSystemManager->OnDrawFrame();
    }

    // Draw our mouse picker frame over the screen
#if MYFW_USING_WX
    if( m_Debug_DrawMousePickerFBO && g_GLCanvasIDActive == 1 )
    {
        if( m_pDebugQuadSprite == 0 )
            m_pDebugQuadSprite = MyNew MySprite( false );

        m_pDebugQuadSprite->CreateInPlace( "debug", 0.75f, 0.75f, 0.5f, 0.5f, 0, 1, 1, 0, Justify_Center, false );
        m_pMaterial_MousePicker->SetTextureColor( m_pEditorState->m_pMousePickerFBO->m_pColorTexture );
        m_pDebugQuadSprite->SetMaterial( m_pMaterial_MousePicker );
        m_pDebugQuadSprite->Draw( 0 );
    }

    if( m_Debug_DrawSelectedAnimatedMesh && g_GLCanvasIDActive == 1 )
    {
        if( m_pEditorState->m_pSelectedObjects.size() > 0 )
        {
            // TODO: have the file selecter pick the right game object/mesh
            GameObject* pObject = m_pEditorState->m_pSelectedObjects[0];

            // if this has an animation player, render the first animation. generally the full timeline.
            ComponentAnimationPlayer* pAnim = pObject->GetAnimationPlayer();
            if( pAnim )
            {
                int backupindex = pAnim->m_AnimationIndex;
                float backuptime = pAnim->m_AnimationTime;

                pAnim->m_AnimationIndex = 0;
                pAnim->m_AnimationTime = (float)MyTime_GetUnpausedTime();
                pAnim->Tick( 0 );

                RenderSingleObject( pObject );

                pAnim->m_AnimationIndex = backupindex;
                pAnim->m_AnimationTime = backuptime;

                if( m_pDebugQuadSprite == 0 )
                    m_pDebugQuadSprite = MyNew MySprite( false );

                m_pDebugQuadSprite->CreateInPlace( "debug", 0.5f, 0.5f, 1.0f, 1.0f, 0, 1, 1, 0, Justify_Center, false );
                m_pMaterial_ClipSpaceTexture->SetTextureColor( m_pEditorState->m_pDebugViewFBO->m_pColorTexture );
                m_pDebugQuadSprite->SetMaterial( m_pMaterial_ClipSpaceTexture );
                m_pDebugQuadSprite->Draw( 0 );
            }

            // if it's a shadow cam, render the depth texture
            ComponentCameraShadow* pCamera = dynamic_cast<ComponentCameraShadow*>( pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera ) );
            if( pCamera )
            {
                if( m_pDebugQuadSprite == 0 )
                    m_pDebugQuadSprite = MyNew MySprite( false );

                m_pDebugQuadSprite->CreateInPlace( "debug", 0.5f, 0.5f, 1.0f, 1.0f, 0, 1, 1, 0, Justify_Center, false );
                m_pMaterial_ClipSpaceTexture->SetTextureColor( pCamera->m_pDepthFBO->m_pDepthTexture );
                m_pDebugQuadSprite->SetMaterial( m_pMaterial_ClipSpaceTexture );
                m_pDebugQuadSprite->Draw( 0 );
            }
        }
    }

    if( /*m_Debug_DrawSelectedMaterial &&*/ g_GLCanvasIDActive == 1 )
    {
        MaterialDefinition* pMaterial = g_pPanelMemory->GetSelectedMaterial();

        if( pMaterial && pEditorCamera )
        {
            if( m_pMaterialBallMesh == 0 )
            {
                m_pMaterialBallMesh = MyNew MyMesh();
                m_pSphereMeshFile = RequestFile( "DataEngine/Meshes/sphere.obj.mymesh" );
            }

            if( m_pMaterialBallMesh && m_pMaterialBallMesh->m_MeshReady == false )
            {
                if( m_pSphereMeshFile->m_FileLoadStatus == FileLoadStatus_Success )
                {
                    m_pMaterialBallMesh->CreateFromMyMeshFile( m_pSphereMeshFile );
                }
            }

            if( m_pMaterialBallMesh && m_pMaterialBallMesh->m_MeshReady )
            {
                m_pEditorState->m_pDebugViewFBO->Bind( true );

                glDisable( GL_SCISSOR_TEST );
                glViewport( 0, 0, m_pEditorState->m_pMousePickerFBO->m_Width, m_pEditorState->m_pMousePickerFBO->m_Height );

                glClearColor( 0, 0, 0, 0 );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

                m_pMaterialBallMesh->SetMaterial( pMaterial, 0 );

                MyMatrix matview;
                matview.SetIdentity();
#if MYFW_RIGHTHANDED
                matview.Translate( 0, 0, -4 );
#else
                matview.Translate( 0, 0, 4 );
#endif

                float aspect = (float)pEditorCamera->m_WindowWidth / pEditorCamera->m_WindowHeight;
                MyMatrix matproj;
                matproj.CreatePerspectiveVFoV( 45, aspect, 0.01f, 100 );

                MyMatrix matviewproj = matproj * matview;
                m_pMaterialBallMesh->Draw( &matviewproj, 0, 0, 0, 0, 0, 0, 0 );

                m_pEditorState->m_pDebugViewFBO->Unbind( true );
            }

            if( m_pDebugQuadSprite == 0 )
                m_pDebugQuadSprite = MyNew MySprite( false );

            float u = (float)m_pEditorState->m_pMousePickerFBO->m_Width / m_pEditorState->m_pMousePickerFBO->m_TextureWidth;
            float v = (float)m_pEditorState->m_pMousePickerFBO->m_Height / m_pEditorState->m_pMousePickerFBO->m_TextureHeight;
            m_pDebugQuadSprite->CreateInPlace( "debug", 0.5f, 0.5f, 1.0f, 1.0f, 0, u, v, 0, Justify_Center, false );
            m_pMaterial_ClipSpaceTexture->SetTextureColor( m_pEditorState->m_pDebugViewFBO->m_pColorTexture );
            m_pDebugQuadSprite->SetMaterial( m_pMaterial_ClipSpaceTexture );
            m_pDebugQuadSprite->Draw( 0 );
        }
    }

    if( m_Debug_DrawGLStats && m_pDebugTextMesh )// && g_GLCanvasIDActive == 1 )
    {
        if( m_pDebugTextMesh->GetMaterial( 0 ) == 0 )
        {
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( "DataEngine/Materials/Nevis60.mymaterial" );
            MyAssert( pMaterial );
            if( pMaterial )
            {
                m_pDebugTextMesh->SetMaterial( pMaterial, 0 );
                pMaterial->Release();
            }
        }

        //m_pDebugTextMesh->CreateStringWhite( false, 15, m_WindowStartX+m_WindowWidth, m_WindowStartY+m_WindowHeight, Justify_TopRight, Vector2(0,0),
        //                                     "GLStats - buffers(%0.2fM) - draws(%d) - fps(%d)", g_pBufferManager->CalculateTotalMemoryUsedByBuffers()/1000000.0f, g_GLStats.GetNumDrawCallsLastFrameForCurrentCanvasID(), (int)m_DebugFPS );
        m_pDebugTextMesh->CreateStringWhite( false, 10, m_WindowStartX+m_WindowWidth, m_WindowStartY+m_WindowHeight, Justify_TopRight, Vector2(0,0),
            "GL - draws(%d) - fps(%d)", g_GLStats.GetNumDrawCallsLastFrameForCurrentCanvasID(), (int)m_DebugFPS );

        {
            int megs = m_LuaMemoryUsedThisFrame/1000000;
            int kilos = (m_LuaMemoryUsedThisFrame - megs*1000000)/1000;
            int bytes = m_LuaMemoryUsedThisFrame%1000;

            int change = m_LuaMemoryUsedThisFrame - m_LuaMemoryUsedLastFrame;

            if( megs == 0 )
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, m_WindowStartX+m_WindowWidth, m_WindowStartY+m_WindowHeight-10, Justify_TopRight, Vector2(0,0),
                    "Lua - memory(%d,%03d) - (%d)", kilos, bytes, change );
            }
            else
            {
                m_pDebugTextMesh->CreateStringWhite( true, 10, m_WindowStartX+m_WindowWidth, m_WindowStartY+m_WindowHeight-10, Justify_TopRight, Vector2(0,0),
                    "Lua - memory(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
            }
        }

        MyMatrix mat;
        mat.CreateOrtho( m_WindowStartX, m_WindowStartX+m_WindowWidth, m_WindowStartY, m_WindowStartY+m_WindowHeight, 1, -1 );
        glDisable( GL_DEPTH_TEST );
        m_pDebugTextMesh->Draw( &mat, 0,0,0,0,0,0,0 );
        glEnable( GL_DEPTH_TEST );
    }
#endif
}

void EngineCore::OnFileRenamed(const char* fullpathbefore, const char* fullpathafter)
{
    g_pComponentSystemManager->OnFileRenamed( fullpathbefore, fullpathafter );
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
            ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();

            // prefer 0,0 at bottom left.
            y = pCamera->m_WindowHeight - y;

            if( m_pEditorState->m_pTransformGizmo->HandleInput( this, -1, -1, action, id, x, y, pressure ) )
                return true;

            if( HandleEditorInput( -1, -1, action, id, x, y, pressure ) )
                return true;

            return false;
        }
    }

    if( g_GLCanvasIDActive != 0 )
    {
        return false;
    }
#endif

    // mouse moving without button down.
    if( id == -1 )
        return false;

    // TODO: get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    if( pCamera == 0 )
        return false;

    // prefer 0,0 at bottom left.
    y = pCamera->m_WindowHeight - y;

    // convert mouse to x/y in Camera2D space. TODO: put this in camera component.
    x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - pCamera->m_WindowStartX) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
    y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + pCamera->m_WindowStartY) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;

    m_LastMousePos.Set( x, y );

    return m_pComponentSystemManager->OnTouch( action, id, x, y, pressure, size );
}

bool EngineCore::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    GameCore::OnButtons( action, id );

    return m_pComponentSystemManager->OnButtons( action, id );
}

bool EngineCore::OnKeys(GameCoreButtonActions action, int keycode, int unicodechar)
{
    if( GameCore::OnKeys( action, keycode, unicodechar ) )
        return true;

    if( action == GCBA_Down )
    {
#if MYFW_USING_WX
        if( m_EditorMode )
        {
            if( keycode == 344 ) // F5
            {
                int filesupdated = g_pFileManager->ReloadAnyUpdatedFiles( OnFileUpdated_CallbackFunction );

                if( filesupdated )
                {
                    g_pShaderManager->InvalidateAllShaders( true );
                    //g_pTextureManager->InvalidateAllTextures( true );
                    //g_pBufferManager->InvalidateAllBuffers( true );
                }
            }
        }

        if( g_GLCanvasIDActive == 1 )
        {
            if( HandleEditorInput( action, keycode, -1, -1, -1, -1, -1 ) )
                return true;

            return false;
        }
        else
#endif
        {
            if( m_pComponentSystemManager->OnKeys( GCBA_Down, keycode, unicodechar ) )
                return true;
        }
    }

    if( action == GCBA_Up )
    {
#if MYFW_USING_WX
        if( g_GLCanvasIDActive == 1 )
        {
            if( HandleEditorInput( action, keycode, -1, -1, -1, -1, -1 ) )
                return true;

            return false;
        }
        else
#endif
        {
            if( m_pComponentSystemManager->OnKeys( GCBA_Up, keycode, unicodechar ) )
                return true;
        }
    }

    if( action == GCBA_Held )
    {
#if MYFW_USING_WX
        if( g_GLCanvasIDActive == 1 )
        {
            if( HandleEditorInput( action, keycode, -1, -1, -1, -1, -1 ) )
                return true;

            return false;
        }
        //else
#endif
    }

    return false;
}

void EngineCore::OnModeTogglePlayStop()
{
    if( m_EditorMode )
    {
        OnModePlay();
    }
    else
    {
        OnModeStop();
    }
}

void EngineCore::OnModePlay()
{
    if( m_EditorMode )
    {
#if MYFW_USING_WX
        g_pMaterialManager->SaveAllMaterials();
        //g_pComponentSystemManager->AddAllMaterialsToFilesList();
        Editor_QuickSaveScene( "temp_editor_onplay.scene" );
        m_EditorMode = false;
        m_Paused = false;
        g_pEngineMainFrame->SetWindowPerspectiveToDefault();
        m_pComponentSystemManager->OnPlay();

        RegisterGameplayButtons();
#endif
    }
}

void EngineCore::OnModeStop()
{
    if( m_EditorMode == false )
    {
        // Call OnStop() for all components.
        m_pComponentSystemManager->OnStop();

        // Unload all runtime created objects.
        UnloadScene( 0, false );

#if MYFW_USING_WX
        Editor_QuickLoadScene( "temp_editor_onplay.scene" );
#endif

        m_EditorMode = true;
        m_Paused = false;

#if MYFW_USING_WX
        g_pEngineMainFrame->SetWindowPerspectiveToDefault();
        m_pEditorState->ClearKeyAndActionStates();
        //m_pEditorState->ClearSelectedObjectsAndComponents();
#endif

        m_pComponentSystemManager->SyncAllRigidBodiesToObjectTransforms();

        UnregisterGameplayButtons();
        return;
    }
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

bool EngineCore::HandleEditorInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
#if MYFW_USING_WX
    if( keycode == MYKEYCODE_LCTRL )
    {
        if( keyaction == GCBA_Down ) m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keyaction == GCBA_Up ) m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    }
    else if( keycode == MYKEYCODE_LALT )
    {
        if( keyaction == GCBA_Down ) m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
        if( keyaction == GCBA_Up ) m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
    }
    else if( keycode == MYKEYCODE_LSHIFT )
    {
        if( keyaction == GCBA_Down ) m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Shift;
        if( keyaction == GCBA_Up ) m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
    }
    else if( keycode == ' ' )
    {
        if( keyaction == GCBA_Down ) m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Space;
        if( keyaction == GCBA_Up ) m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Space;
    }
    if( mouseaction != -1 )
    {
        m_pEditorState->m_CurrentMousePosition.Set( x, y );
        //m_pEditorState->m_LastMousePosition.Set( x, y );

        if( mouseaction == GCBA_Down )
        {
            if( id == 0 )
            {
                m_pEditorState->m_MouseLeftDownLocation = m_pEditorState->m_CurrentMousePosition;
                m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_LeftMouse;
            }
            if( id == 1 )
            {
                m_pEditorState->m_MouseRightDownLocation = m_pEditorState->m_CurrentMousePosition;
                m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_RightMouse;
            }
            if( id == 2 )
            {
                m_pEditorState->m_MouseMiddleDownLocation = m_pEditorState->m_CurrentMousePosition;
                m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_MiddleMouse;
            }
        }
    }

    if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( mouseaction == GCBA_Down && id == 0 )
        {
            m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_GroupSelectingObjects;
        }

        // select an object when mouse is released and on the same pixel it went down.
        // TODO: make same "pixel test" a "total travel < small number" test.
        //if( mouseaction == GCBA_Up && id == 0 && m_pEditorState->m_CurrentMousePosition == m_pEditorState->m_MouseLeftDownLocation )
        //{
        //    m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;

        //    // find the object we clicked on.
        //    GameObject* pObject = GetObjectAtPixel( x, y, true );

        //    // don't select the gizmos. TODO: give object a 'selectable' flag or something.
        //    if( pObject != m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] &&
        //        pObject != m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[1] &&
        //        pObject != m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[2] )
        //    {
        //        // if control isn't held, then deselect all objects first.
        //        if( (m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control) == 0 )
        //        {
        //            m_pEditorState->ClearSelectedObjectsAndComponents();
        //        }

        //        if( pObject && m_pEditorState->IsObjectSelected( pObject ) == false )
        //        {
        //            m_pEditorState->m_pSelectedObjects.push_back( pObject );
        //            // select the object in the object tree.
        //            g_pPanelObjectList->SelectObject( pObject ); // passing in 0 will unselect all items.
        //        }
        //    }
        //}

        if( mouseaction == GCBA_Down && id == 0 )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( (unsigned int)x, (unsigned int)y, true );

            // reset mouse movement, so we can undo to this state after mouse goes up.
            m_pEditorState->m_DistanceTranslated.Set( 0, 0, 0 );
            //LOGInfo( LOGTag, "m_pEditorState->m_DistanceTranslated.Set zero( %f, %f, %f );\n", m_pEditorState->m_DistanceTranslated.x, m_pEditorState->m_DistanceTranslated.y, m_pEditorState->m_DistanceTranslated.z );

            bool selectedgizmo = false;

            // translate to the right.
            if( pObject == m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )
            {
                m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateX;
                selectedgizmo = true;
            }
            if( pObject == m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[1] )
            {
                m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateY;
                selectedgizmo = true;
            }
            if( pObject == m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[2] )
            {
                m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateZ;
                selectedgizmo = true;
            }

            // if shift is held, make a copy of the object and control that one.
            if( selectedgizmo && m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            {
                std::vector<GameObject*> selectedobjects = m_pEditorState->m_pSelectedObjects;
                m_pEditorState->ClearSelectedObjectsAndComponents();
                for( unsigned int i=0; i<selectedobjects.size(); i++ )
                {
                    GameObject* pNewObject = g_pComponentSystemManager->EditorCopyGameObject( selectedobjects[i], false );
                    if( m_EditorMode )
                    {
                        pNewObject->SetSceneID( selectedobjects[i]->GetSceneID() );
                    }

                    m_pEditorState->m_pSelectedObjects.push_back( pNewObject );
                    // select the object in the object tree.
                    g_pPanelObjectList->SelectObject( pNewObject );
                }
            }

            // if we didn't select the gizmo and gameplay is running.
            if( selectedgizmo == false && m_EditorMode == false )
            {
                // check if we selected a physics object then grab it.
                //if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                {
                    // Get the mouse click ray.
                    Vector3 raystart, rayend;
                    GetMouseRay( m_pEditorState->m_CurrentMousePosition, &raystart, &rayend );

                    btVector3 RayStart( raystart.x, raystart.y, raystart.z );
                    btVector3 RayEnd( rayend.x, rayend.y, rayend.z );

                    btCollisionWorld::ClosestRayResultCallback rayCallback( RayStart, RayEnd );
                    g_pBulletWorld->m_pDynamicsWorld->rayTest( RayStart, RayEnd, rayCallback );
                    if( rayCallback.hasHit() )
                    {
                        btVector3 pickPos = rayCallback.m_hitPointWorld;

                        m_pEditorState->m_MousePicker_PickedBody = 0;

                        //pickObject( pickPos, rayCallback.m_collisionObject );
                        btRigidBody* body = (btRigidBody*)btRigidBody::upcast( rayCallback.m_collisionObject );
                        if( body )
                        {
                            // other exclusions?
                            if( !(body->isStaticObject() || body->isKinematicObject()) )
                            {
                                m_pEditorState->m_MousePicker_PickedBody = body;
                                m_pEditorState->m_MousePicker_PickedBody->setActivationState( DISABLE_DEACTIVATION );

                                //printf("pickPos=%f,%f,%f\n",pickPos.getX(),pickPos.getY(),pickPos.getZ());

                                btVector3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;

                                if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift ) //(m_modifierKeys & BT_ACTIVE_SHIFT) != 0 )
                                {
                                    btTransform tr;
                                    tr.setIdentity();
                                    tr.setOrigin(localPivot);
                                    btGeneric6DofConstraint* dof6 = new btGeneric6DofConstraint(*body, tr,false);
                                    dof6->setLinearLowerLimit( btVector3(0,0,0) );
                                    dof6->setLinearUpperLimit( btVector3(0,0,0) );
                                    dof6->setAngularLowerLimit( btVector3(0,0,0) );
                                    dof6->setAngularUpperLimit( btVector3(0,0,0) );

                                    g_pBulletWorld->m_pDynamicsWorld->addConstraint(dof6,true);
                                    m_pEditorState->m_MousePicker_PickConstraint = dof6;

                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 0 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 1 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 2 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 3 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 4 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 5 );

                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 0 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 1 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 2 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 3 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 4 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 5 );
                                }
                                else
                                {
                                    btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*body,localPivot);
                                    g_pBulletWorld->m_pDynamicsWorld->addConstraint(p2p,true);
                                    m_pEditorState->m_MousePicker_PickConstraint = p2p;
                                    btScalar mousePickClamping = 30.f;
                                    p2p->m_setting.m_impulseClamp = mousePickClamping;
                                    //very weak constraint for picking
                                    p2p->m_setting.m_tau = 0.001f;
                                    /*
                                    p2p->setParam(BT_CONSTRAINT_CFM,0.8,0);
                                    p2p->setParam(BT_CONSTRAINT_CFM,0.8,1);
                                    p2p->setParam(BT_CONSTRAINT_CFM,0.8,2);
                                    p2p->setParam(BT_CONSTRAINT_ERP,0.1,0);
                                    p2p->setParam(BT_CONSTRAINT_ERP,0.1,1);
                                    p2p->setParam(BT_CONSTRAINT_ERP,0.1,2);
                                    */
                                }

                                //save mouse position for dragging
                            }
                        }

                        //gOldPickingPos = RayEnd;
                        //gHitPos = pickPos;

                        m_pEditorState->m_MousePicker_OldPickingDist = (pickPos-RayStart).length();
                        mouseaction = -1;
                    }
                }
            }
        }

        if( mouseaction == GCBA_Held && id == 0 )
        {
            // gameplay is running and we picked up a physics object in the editor view, so move it around.
            if( m_pEditorState->m_MousePicker_PickConstraint && g_pBulletWorld->m_pDynamicsWorld )
            {
                // move the constraint pivot
                if( m_pEditorState->m_MousePicker_PickConstraint->getConstraintType() == D6_CONSTRAINT_TYPE )
                {
                    btGeneric6DofConstraint* pickCon = static_cast<btGeneric6DofConstraint*>( m_pEditorState->m_MousePicker_PickConstraint );
                    if( pickCon )
                    {
                        //keep it at the same picking distance

                        // Get the mouse click ray.
                        Vector3 raystart, rayend;
                        GetMouseRay( m_pEditorState->m_CurrentMousePosition, &raystart, &rayend );

                        btVector3 newRayTo( rayend.x, rayend.y, rayend.z );
                        btVector3 rayFrom;
                        btVector3 oldPivotInB = pickCon->getFrameOffsetA().getOrigin();

                        btVector3 newPivotB;
                        //if( m_ortho )
                        //{
                        //    newPivotB = oldPivotInB;
                        //    newPivotB.setX(newRayTo.getX());
                        //    newPivotB.setY(newRayTo.getY());
                        //}
                        //else
                        {
                            rayFrom = btVector3( raystart.x, raystart.y, raystart.z );
                            btVector3 dir = newRayTo - rayFrom;
                            dir.normalize();
                            dir *= m_pEditorState->m_MousePicker_OldPickingDist;

                            newPivotB = rayFrom + dir;
                        }

                        pickCon->getFrameOffsetA().setOrigin(newPivotB);
                    }
                }
                else
                {
                    btPoint2PointConstraint* pickCon = static_cast<btPoint2PointConstraint*>( m_pEditorState->m_MousePicker_PickConstraint );
                    if (pickCon)
                    {
                        //keep it at the same picking distance

                        // Get the mouse click ray.
                        Vector3 raystart, rayend;
                        GetMouseRay( m_pEditorState->m_CurrentMousePosition, &raystart, &rayend );

                        btVector3 newRayTo( rayend.x, rayend.y, rayend.z );
                        btVector3 rayFrom;
                        btVector3 oldPivotInB = pickCon->getPivotInB();
                        btVector3 newPivotB;

                        //if( m_ortho )
                        //{
                        //    newPivotB = oldPivotInB;
                        //    newPivotB.setX(newRayTo.getX());
                        //    newPivotB.setY(newRayTo.getY());
                        //}
                        //else
                        {
                            rayFrom = btVector3( raystart.x, raystart.y, raystart.z );
                            btVector3 dir = newRayTo - rayFrom;
                            dir.normalize();
                            dir *= m_pEditorState->m_MousePicker_OldPickingDist;

                            newPivotB = rayFrom + dir;
                        }

                        pickCon->setPivotB(newPivotB);
                    }
                }

                //float dx, dy;
                //dx = btScalar(x) - m_mouseOldX;
                //dy = btScalar(y) - m_mouseOldY;


                ///only if ALT key is pressed (Maya style)
                //if (m_modifierKeys& BT_ACTIVE_ALT)
                //{
                //    if(m_mouseButtons & 2)
                //    {
                //        btVector3 hor = getRayTo(0,0)-getRayTo(1,0);
                //        btVector3 vert = getRayTo(0,0)-getRayTo(0,1);
                //        btScalar multiplierX = btScalar(0.001);
                //        btScalar multiplierY = btScalar(0.001);
                //        if (m_ortho)
                //        {
                //            multiplierX = 1;
                //            multiplierY = 1;
                //        }


                //        m_cameraTargetPosition += hor* dx * multiplierX;
                //        m_cameraTargetPosition += vert* dy * multiplierY;
                //    }

                //    if(m_mouseButtons & (2 << 2) && m_mouseButtons & 1)
                //    {
                //    }
                //    else if(m_mouseButtons & 1)
                //    {
                //        m_azi += dx * btScalar(0.2);
                //        m_azi = fmodf(m_azi, btScalar(360.f));
                //        m_ele += dy * btScalar(0.2);
                //        m_ele = fmodf(m_ele, btScalar(180.f));
                //    }
                //    else if(m_mouseButtons & 4)
                //    {
                //        m_cameraDistance -= dy * btScalar(0.02f);
                //        if (m_cameraDistance<btScalar(0.1))
                //            m_cameraDistance = btScalar(0.1);


                //    }
                //}


                //m_mouseOldX = x;
                //m_mouseOldY = y;
                //updateCamera();

                mouseaction = -1;
            }
        }

        if( mouseaction == GCBA_Up && id == 0 )
        {
            m_pEditorState->ClearConstraint();
        }

        // check if the mouse moved.
        Vector3 mousedragdir = m_pEditorState->m_CurrentMousePosition - m_pEditorState->m_LastMousePosition;

        if( mousedragdir.LengthSquared() != 0 )
        {
            // If the mouse moved, move the selected objects along a plane or axis
            m_pEditorState->m_pTransformGizmo->TranslateSelectedObjects( this, m_pEditorState );
        }
    }

    // if mouse message. down, up or dragging.
    if( mouseaction != -1 )
    {
        unsigned int mods = m_pEditorState->m_ModifierKeyStates;

        // get the editor camera's local transform.
        ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

        // move camera in/out if mousewheel spinning
        if( mouseaction == GCBA_Wheel )
        {
            // pressure is also mouse wheel movement rate in wx configurations.
#if MYFW_RIGHTHANDED
            Vector3 dir = Vector3( 0, 0, 1 ) * -(pressure/fabs(pressure));
#else
            Vector3 dir = Vector3( 0, 0, 1 ) * (pressure/fabs(pressure));
#endif
            float speed = 100.0f;
            if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                speed *= 5;

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * speed * m_TimePassedUnpausedLastFrame );
        }

        // if left mouse down, reset the transform gizmo tool.
        if( mouseaction == GCBA_Down && id == 0 )
        {
            m_pEditorState->m_pTransformGizmo->m_LastIntersectResultIsValid = false;
        }

        // if space is held, left button will pan the camera around.  or just middle button
        if( ( (mods & MODIFIERKEY_LeftMouse) && (mods & MODIFIERKEY_Space) ) || (mods & MODIFIERKEY_MiddleMouse) )
        {
            Vector2 dir = m_pEditorState->m_LastMousePosition - m_pEditorState->m_CurrentMousePosition;

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * 0.05f );
        }
        else if( mouseaction == GCBA_Held &&
                 m_pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects &&
                 (mods & MODIFIERKEY_LeftMouse) )
        {
            // TODO: draw a box in the foreground.
            //int smallerx = m_pEditorState->m_CurrentMousePosition.x > m_pEditorState->m_MouseLeftDownLocation.x ? m_pEditorState->m_MouseLeftDownLocation.x : m_pEditorState->m_CurrentMousePosition.x;
            //int biggerx = m_pEditorState->m_CurrentMousePosition.x < m_pEditorState->m_MouseLeftDownLocation.x ? m_pEditorState->m_MouseLeftDownLocation.x : m_pEditorState->m_CurrentMousePosition.x;

            //int smallery = m_pEditorState->m_CurrentMousePosition.y > m_pEditorState->m_MouseLeftDownLocation.y ? m_pEditorState->m_MouseLeftDownLocation.y : m_pEditorState->m_CurrentMousePosition.y;
            //int biggery = m_pEditorState->m_CurrentMousePosition.y < m_pEditorState->m_MouseLeftDownLocation.y ? m_pEditorState->m_MouseLeftDownLocation.y : m_pEditorState->m_CurrentMousePosition.y;

            //LOGInfo( LOGTag, "group selecting: %d,%d  %d,%d\n", smallerx, smallery, biggerx, biggery );

            //m_pEditorState->ClearSelectedObjectsAndComponents();
            //for( int y=smallery; y<biggery; y++ )
            //{
            //    for( int x=smallerx; x<biggerx; x++ )
            //    {
            //        GameObject* pObject = GetObjectAtPixel( x, y, false );
            //        if( pObject )
            //            g_pPanelObjectList->SelectObject( pObject ); // passing in 0 will unselect all items.
            //    }
            //}
        }
        // if left or right mouse is down, rotate the camera.
        else if( m_pEditorState->m_EditorActionState == EDITORACTIONSTATE_None &&
                 //( (mods & MODIFIERKEY_LeftMouse) || (mods & MODIFIERKEY_RightMouse) ) )
                 (mods & MODIFIERKEY_RightMouse) )
        {
            // rotate the camera around selected object or a point 10 units in front of the camera.
            Vector2 dir = m_pEditorState->m_CurrentMousePosition - m_pEditorState->m_LastMousePosition;

            Vector3 pivot;
            float distancefrompivot;

            //if( mods & MODIFIERKEY_LeftMouse && m_pEditorState->m_pSelectedObjects.size() > 0 && m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )//m_pEditorState->m_pSelectedObjects[0] )
            if( mods & MODIFIERKEY_Alt && m_pEditorState->m_pSelectedObjects.size() > 0 && m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )
            {
                // pivot around the transform gizmo
                pivot = m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0]->m_pComponentTransform->m_Transform.GetTranslation();
                //m_pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->m_Transform.GetTranslation();
                distancefrompivot = (matLocalCamera->GetTranslation() - pivot).Length();
            }
            else
            {
                if( mods & MODIFIERKEY_RightMouse )
                {
                    // pivot on the camera, just change rotation.
                    pivot = matLocalCamera->GetTranslation();
                    distancefrompivot = 0;
                }
                else
                {
                    // TODO: try to pivot from distance of object at mouse
                    MyMatrix mattemp = *matLocalCamera;
#if MYFW_RIGHTHANDED
                    mattemp.TranslatePreRotScale( 0, 0, -10 );
#else
                    mattemp.TranslatePreRotScale( 0, 0, 10 );
#endif
                    pivot = mattemp.GetTranslation();
                    distancefrompivot = 10;
                }
            }

            if( dir.LengthSquared() > 0 )
            {
                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                //Vector3 pos = matLocalCamera->GetTranslation();
                Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();

                // todo: make this degrees per inch
                float degreesperpixel = 1.0f;

                dir.Normalize();

#if MYFW_RIGHTHANDED
                angle.y += dir.x * degreesperpixel;
                angle.x -= dir.y * degreesperpixel;
#else
                angle.y -= dir.x * degreesperpixel;
                angle.x += dir.y * degreesperpixel;
#endif
                MyClamp( angle.x, -90.0f, 90.0f );

                matLocalCamera->SetIdentity();
#if MYFW_RIGHTHANDED
                matLocalCamera->Translate( 0, 0, distancefrompivot );
#else
                matLocalCamera->Translate( 0, 0, -distancefrompivot );
#endif
                matLocalCamera->Rotate( angle.x, 1, 0, 0 );
                matLocalCamera->Rotate( angle.y, 0, 1, 0 );
                matLocalCamera->Translate( pivot );
            }
        }

        // pull the pos/angle from the local matrix and update the values for the watch window.
        pCamera->m_pComponentTransform->UpdatePosAndRotFromLocalMatrix();
    }

    // handle editor keys
    if( keyaction == GCBA_Held )
    {
        // get the editor camera's local transform.
        ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

        // WASD to move camera
        Vector3 dir( 0, 0, 0 );
        if( keycode == 'W' ) dir.z +=  1;
        if( keycode == 'A' ) dir.x += -1;
        if( keycode == 'S' ) dir.z += -1;
        if( keycode == 'D' ) dir.x +=  1;
        if( keycode == 'Q' ) dir.y +=  1;
        if( keycode == 'Z' ) dir.y -=  1;

        float speed = 7.0f;
        if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            speed *= 5;

        if( dir.LengthSquared() > 0 )
            matLocalCamera->TranslatePreRotScale( dir * speed * m_TimePassedUnpausedLastFrame );
    }

    if( keyaction == GCBA_Up )
    {
        // Lock to current object, 
        if( keycode == 'L' )
        {
            if( m_pEditorState->m_CameraState == EditorCameraState_Default && m_pEditorState->m_pSelectedObjects.size() > 0 )
            {
                GameObject* pSelectedObject = m_pEditorState->m_pSelectedObjects[0];
                m_pEditorState->LockCameraToGameObject( pSelectedObject );
            }
            else
            {
                m_pEditorState->LockCameraToGameObject( 0 );
            }
        }
    }

    // check for mouse ups and clear the states.
    if( mouseaction != -1 )
    {
        if( mouseaction == GCBA_Up )
        {
            if( id == 0 )
            {
                // when mouse up, select all object in the box.
                if( m_pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects )
                {
                    SelectObjectsInRectangle( (unsigned int)m_pEditorState->m_MouseLeftDownLocation.x, (unsigned int)m_pEditorState->m_MouseLeftDownLocation.y,
                                              (unsigned int)m_pEditorState->m_CurrentMousePosition.x, (unsigned int)m_pEditorState->m_CurrentMousePosition.y );
                }

                m_pEditorState->m_MouseLeftDownLocation = Vector2( -1, -1 );
                m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_LeftMouse;
                m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;

                // GIZMOTRANSLATE: add translation to undo stack, action itself is done each frame.  We only want to undo to last mouse down.
                if( m_pEditorState->m_pSelectedObjects.size() > 0 && m_pEditorState->m_DistanceTranslated.LengthSquared() != 0 )
                {
                    // Create a new list of selected objects, don't include objects that have parents that are selected.
                    std::vector<GameObject*> selectedobjects;
                    for( unsigned int i=0; i<m_pEditorState->m_pSelectedObjects.size(); i++ )
                    {
                        ComponentTransform* pTransform = m_pEditorState->m_pSelectedObjects[i]->m_pComponentTransform;

                        // if this object has a selected parent, don't move it, only move the parent.
                        if( pTransform->IsAnyParentInList( m_pEditorState->m_pSelectedObjects ) == false )
                        {
                            selectedobjects.push_back( m_pEditorState->m_pSelectedObjects[i] );
                        }
                    }

                    if( selectedobjects.size() > 0 )
                    {
                        g_pEngineMainFrame->m_pCommandStack->Add( MyNew EditorCommand_MoveObjects( m_pEditorState->m_DistanceTranslated, selectedobjects ) );
                    }
                }
            }
            else if( id == 1 )
            {
                m_pEditorState->m_MouseRightDownLocation = Vector2( -1, -1 );
                m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_RightMouse;
            }
            else if( id == 2 )
            {
                m_pEditorState->m_MouseMiddleDownLocation = Vector2( -1, -1 );
                m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_MiddleMouse;
            }
        }
    }

    m_pEditorState->m_LastMousePosition = m_pEditorState->m_CurrentMousePosition;
#endif //MYFW_USING_WX

    return false;
}

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
            pComponentMesh->SetVisible( true ); // manually drawn when in editor mode.
            pComponentMesh->SetMaterial( m_pMaterial_3DGrid, 0 ); //( m_pShader_TransformGizmo );
            pComponentMesh->SetLayersThisExistsOn( Layer_Editor | Layer_EditorUnselectable );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorLineGridXZ( Vector3(0,0,0), 1, 5 );
            // TODOMaterials: put this back for plane.
            //pComponentMesh->m_pMesh->m_Tint.Set( 150, 150, 150, 255 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
        }

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        MyAssert( m_pEditorState->m_p3DGridPlane == 0 );
        m_pEditorState->m_p3DGridPlane = pGameObject;
    }

    // create a 3d transform gizmo for each axis.
    m_pEditorState->m_pTransformGizmo->CreateAxisObjects( ENGINE_SCENE_ID, 0.03f, m_pMaterial_TransformGizmoX, m_pMaterial_TransformGizmoY, m_pMaterial_TransformGizmoZ, m_pEditorState );

    // create a 3D editor camera, renders editor view.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false, ENGINE_SCENE_ID ); // not managed.
        pGameObject->SetName( "Editor Camera" );
#if MYFW_RIGHTHANDED
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 10 ) );
#else
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, -10 ) );
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
#if MYFW_RIGHTHANDED
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 10 ) );
#else
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, -10 ) );
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

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = true;
        pComponentCamera->m_LayersToRender = Layer_HUD;
        pComponentCamera->m_ClearColorBuffer = false;
        pComponentCamera->m_ClearDepthBuffer = false;
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

void EngineCore::UnloadScene(unsigned int sceneid, bool cleareditorobjects)
{
    // reset the editorstate structure.
#if MYFW_USING_WX
    if( sceneid != 0 )
    {
        g_pEngineMainFrame->m_pCommandStack->ClearStacks();
        g_pEngineMainFrame->m_StackDepthAtLastSave = 0;
    }
    m_pEditorState->ClearEditorState( false );
#endif //MYFW_USING_WX

    g_pComponentSystemManager->UnloadScene( sceneid, false );

    if( sceneid == UINT_MAX && m_FreeAllMaterialsAndTexturesWhenUnloadingScene )
    {
        // temp code while RTQGlobals is a thing.
        SAFE_RELEASE( g_pRTQGlobals->m_pMaterial );

        //g_pComponentSystemManager->FreeAllDataFiles();
        //g_pMaterialManager->FreeAllMaterials();
        //g_pTextureManager->FreeAllTextures( false );
    }
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

            jsonstr = MyNew char[length+1];
            fread( jsonstr, length, 1, filehandle );
            jsonstr[length] = 0;
        }

        fclose( filehandle );

        const char* filenamestart;
        int i;
        for( i=strlen(fullpath)-1; i>=0; i-- )
        {
            if( fullpath[i] == '\\' || fullpath[i] == '/' )
                break;
        }
        filenamestart = &fullpath[i+1];

        LoadScene( filenamestart, jsonstr, sceneid );
        strcpy_s( g_pComponentSystemManager->m_pSceneInfoMap[sceneid].fullpath, MAX_PATH, fullpath );

        delete[] jsonstr;
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

            jsonstr = MyNew char[length+1];
            fread( jsonstr, length, 1, filehandle );
            jsonstr[length] = 0;
        }

        fclose( filehandle );

        LoadScene( fullpath, jsonstr, UINT_MAX );

        delete[] jsonstr;
    }
}
#endif //MYFW_USING_WX

void EngineCore::LoadScene(const char* scenename, const char* jsonstr, unsigned int sceneid)
{
    // reset the editorstate structure.
#if MYFW_USING_WX
    m_pEditorState->ClearEditorState( false );
#endif //MYFW_USING_WX

    g_pComponentSystemManager->LoadSceneFromJSON( scenename, jsonstr, sceneid );

    // Tell all the cameras loaded in the scene the dimensions of the window. // TODO: move this into camera's onload.
    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );

    // Finish loading, calls onload and onplay.
#if MYFW_USING_WX
    g_pComponentSystemManager->FinishLoading( false, sceneid, m_AllowGameToRunInEditorMode );
#else
    g_pComponentSystemManager->FinishLoading( false, sceneid, true );
#endif

#if MYFW_USING_WX
    m_EditorMode = true;

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
            for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
            {
                ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
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
void EngineCore::RenderObjectIDsToFBO()
{
    if( m_pEditorState->m_pMousePickerFBO->m_FullyLoaded == false )
        return;

    // bind our FBO so we can render the scene to it.
    m_pEditorState->m_pMousePickerFBO->Bind( true );

    m_pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, m_pEditorState->m_pMousePickerFBO->m_Width, m_pEditorState->m_pMousePickerFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw all editor camera components.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
        if( pCamera )
        {
            m_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, m_pShader_TintColor );
            glClear( GL_DEPTH_BUFFER_BIT );
        }
    }

    m_pEditorState->m_pMousePickerFBO->Unbind( true );

    m_pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );
}

GameObject* EngineCore::GetObjectAtPixel(unsigned int x, unsigned int y, bool createnewbitmap)
{
    if( m_pEditorState->m_pMousePickerFBO->m_FullyLoaded == false )
        return 0;

    if( createnewbitmap )
    {
        RenderObjectIDsToFBO();
    }

    // bind our FBO so we can render sample from it.
    m_pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
        break;
    }

    MyAssert( pCamera );
    if( pCamera == 0 )
        return 0;

    // get a pixel from the FBO... use m_WindowStartX/m_WindowStartY from any camera component.
    unsigned char pixel[4];
    glReadPixels( x - (unsigned int)pCamera->m_WindowStartX, y - (unsigned int)pCamera->m_WindowStartY,
                  1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel );

    uint64_t id = pixel[0] + pixel[1]*256 + pixel[2]*256*256 + pixel[3]*256*256*256;
    id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,
    //LOGInfo( LOGTag, "pixel - %d, %d, %d, %d - id - %d\n", pixel[0], pixel[1], pixel[2], pixel[3], id );

    unsigned int sceneid = (unsigned int)(id / 100000);
    id = id % 100000;

    // find the object clicked on.
    GameObject* pGameObject = m_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

    // if we didn't click on something, check if it's the transform gizmo.
    //   has to be checked manually since they are unmanaged.
    if( pGameObject == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            if( m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[i]->GetID() == id )
            {
                pGameObject = m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[i];
            }
        }
    }

    m_pEditorState->m_pMousePickerFBO->Unbind( true );

    return pGameObject;
}

void EngineCore::SelectObjectsInRectangle(unsigned int sx, unsigned int sy, unsigned int ex, unsigned int ey)
{
    int smallerx = sx > ex ? ex : sx;
    int biggerx = sx < ex ? ex : sx;

    int smallery = sy > ey ? ey : sy;
    int biggery = sy < ey ? ey : sy;

    //LOGInfo( LOGTag, "group selecting: %d,%d  %d,%d\n", smallerx, smallery, biggerx, biggery );

    // render to the FBO.
    RenderObjectIDsToFBO();

    // bind our FBO so we can sample from it.
    m_pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
        break;
    }

    MyAssert( pCamera );
    if( pCamera == 0 )
        return;

    // get a pixel from the FBO... use m_WindowStartX/m_WindowStartY from any camera component.
    unsigned int fbowidth = m_pEditorState->m_pMousePickerFBO->m_Width;
    unsigned int fboheight = m_pEditorState->m_pMousePickerFBO->m_Height;
    unsigned char* pixels = MyNew unsigned char[fbowidth * fboheight * 4];
    glReadPixels( 0, 0, fbowidth, fboheight, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    m_pEditorState->m_pMousePickerFBO->Unbind( true );

    bool controlheld = m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control ? true : false;

    // if user isn't holding control, then clear objects and items selected in tree.
    if( controlheld == false )
        m_pEditorState->ClearSelectedObjectsAndComponents();

    // potentially about to multi-select, so disable tree callbacks.
    g_pPanelObjectList->m_UpdatePanelWatchOnSelection = false;

    //unsigned int pixelsbeingtested = (biggerx - smallerx) * (biggery - smallery);

    // check whether or not the first object clicked on was selected, we only care if control is held.
    bool firstobjectwasselected = false;
    if( controlheld )
    {
        unsigned int offset = (sy*fbowidth + sx)*4;
        unsigned long long id = pixels[offset+0] + pixels[offset+1]*256 + pixels[offset+2]*256*256 + pixels[offset+3]*256*256*256;
        id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

        unsigned int sceneid = (unsigned int)id / 100000;
        id = id % 100000;

        // if the object's not already selected, select it.
        GameObject* pObject = m_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

        if( pObject && m_pEditorState->IsGameObjectSelected( pObject ) )
            firstobjectwasselected = true;
    }

    for( int y=smallery; y<=biggery; y++ )
    {
        for( int x=smallerx; x<=biggerx; x++ )
        {
            unsigned int offset = (y*fbowidth + x)*4;
            unsigned long long id = pixels[offset+0] + pixels[offset+1]*256 + pixels[offset+2]*256*256 + pixels[offset+3]*256*256*256;
            id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

            unsigned int sceneid = (unsigned int)id / 100000;
            id = id % 100000;

            // if the object's not already selected, select it.
            GameObject* pObject = m_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

            if( pObject )
            {
                bool objectselected = m_pEditorState->IsGameObjectSelected( pObject );

                // if we're selecting objects, then select the unselected objects.
                if( firstobjectwasselected == false )
                {
                    if( objectselected == false )
                    {
                        // select the object
                        m_pEditorState->SelectGameObject( pObject );

                        // select the object in the object tree.
                        g_pPanelObjectList->SelectObject( pObject );
                    }
                }
                else if( controlheld ) // if the first object was already selected, deselect all dragged if control is held.
                {
                    if( objectselected == true )
                    {
                        m_pEditorState->UnselectGameObject( pObject );
                        g_pPanelObjectList->UnselectObject( pObject );
                    }
                }
            }
        }
    }

    g_pPanelObjectList->m_UpdatePanelWatchOnSelection = true;
    UpdatePanelWatchWithSelectedItems(); // will reset and update m_pEditorState->m_pSelectedObjects

    //LOGInfo( LOGTag, "Done selecting objects.\n" );

    delete[] pixels;
}

void EngineCore::RenderSingleObject(GameObject* pObject)
{
    // render the scene to a FBO.
    m_pEditorState->m_pDebugViewFBO->Bind( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, m_pEditorState->m_pMousePickerFBO->m_Width, m_pEditorState->m_pMousePickerFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw all editor camera components.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
        if( pCamera )
        {
            MyMatrix matView;
            matView.CreateLookAtView( Vector3(0, 15, 15), Vector3(0,1,0), Vector3(0,5,0) );

            MyMatrix matProj;
            matProj.CreatePerspectiveHFoV( 45, 1, 0.1f, 1000.0f );

            MyMatrix matViewProj = matProj * matView;

            m_pComponentSystemManager->DrawSingleObject( &matViewProj, pObject );

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
    Vector3 raystart = pCamera->m_pComponentTransform->GetPosition();
    Vector3 rayend = raystart + rayworld * 10000;

    start->Set( raystart.x, raystart.y, raystart.z );
    end->Set( rayend.x, rayend.y, rayend.z );
}
#endif //MYFW_USING_WX

#if MYFW_USING_WX
void EngineCore::OnObjectListTreeSelectionChanged()
{
    if( m_pEditorState )
    {
        //LOGInfo( LOGTag, "Clearing Selected Objects\n" );
        m_pEditorState->m_pSelectedObjects.clear();
        m_pEditorState->m_pSelectedComponents.clear();
    }
}
#endif //MYFW_USING_WX
