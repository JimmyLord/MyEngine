//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

GameEntityComponentTest::GameEntityComponentTest()
{
    m_pComponentSystemManager = 0;

#if MYFW_USING_WX
    m_EditorMode = true;
#else
    m_EditorMode = false;
#endif

    m_TimeSinceLastPhysicsStep = 0;

    //m_pShader_TintColor = 0;
    //m_pShader_TestNormals = 0;
    //m_pShader_Texture = 0;
    m_pShader_TransformGizmo = 0;
    m_pShader_MousePicker = 0;

    m_GameWidth = 0;
    m_GameHeight = 0;

    //for( int i=0; i<4; i++ )
    //    m_pOBJTestFiles[i] = 0;

    //for( int i=0; i<4; i++ )
    //    m_pTextures[i] = 0;

    //for( int i=0; i<4; i++ )
    //    m_pScriptFiles[i] = 0;

    m_pSceneFileToLoad = 0;
    m_SceneLoaded = false;

    m_pBulletWorld = MyNew BulletWorld();

    m_pLuaGameState = 0;

#if MYFW_USING_WX
    m_pEditorState = MyNew EditorState;

    g_pPanelObjectList->m_pCallbackFunctionObject = this;
    g_pPanelObjectList->m_pOnTreeSelectionChangedFunction = StaticOnObjectListTreeSelectionChanged;
#endif //MYFW_USING_WX
}

GameEntityComponentTest::~GameEntityComponentTest()
{
    SAFE_DELETE( m_pLuaGameState );

    //SAFE_RELEASE( m_pShader_TintColor );
    //SAFE_RELEASE( m_pShader_TestNormals );
    //SAFE_RELEASE( m_pShader_Texture );
    SAFE_RELEASE( m_pShader_TransformGizmo );
    SAFE_RELEASE( m_pShader_MousePicker );

    //for( int i=0; i<4; i++ )
    //{
    //    if( m_pOBJTestFiles[i] )
    //        g_pFileManager->FreeFile( m_pOBJTestFiles[i] );
    //}

    //for( int i=0; i<4; i++ )
    //{
    //    SAFE_RELEASE( m_pTextures[i] );
    //}

    //for( int i=0; i<4; i++ )
    //{
    //    if( m_pScriptFiles[i] )
    //        g_pFileManager->FreeFile( m_pScriptFiles[i] );
    //}

#if MYFW_USING_WX
    SAFE_DELETE( m_pEditorState );
#endif //MYFW_USING_WX

    SAFE_DELETE( m_pComponentSystemManager );
    SAFE_DELETE( m_pBulletWorld );
}

void GameEntityComponentTest::OneTimeInit()
{
    GameCore::OneTimeInit();

    GameObject* pPlayer = 0;
    GameObject* pGameObject;
    ComponentCamera* pComponentCamera;
    //ComponentSprite* pComponentSprite;
#if MYFW_USING_WX
    ComponentMesh* pComponentMesh;
#endif
    //ComponentMeshOBJ* pComponentMeshOBJ;
    //ComponentAIChasePlayer* pComponentAIChasePlayer;
    //ComponentCollisionObject* pComponentCollisionObject;
    //ComponentLuaScript* pComponentLuaScript;

    //m_pOBJTestFiles[0] = g_pFileManager->RequestFile( "Data/OBJs/cube.obj" );
    ////m_pOBJTestFiles[1] = g_pFileManager->RequestFile( "Data/OBJs/Teapot.obj" );
    ////m_pOBJTestFiles[2] = g_pFileManager->RequestFile( "Data/OBJs/humanoid_tri.obj" );
    ////m_pOBJTestFiles[3] = g_pFileManager->RequestFile( "Data/OBJs/teapot2.obj" );

    ////m_pTextures[0] = g_pTextureManager->CreateTexture( "Data/Textures/test1.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );
    ////m_pTextures[1] = g_pTextureManager->CreateTexture( "Data/Textures/test2.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );
    ////m_pTextures[2] = g_pTextureManager->CreateTexture( "Data/Textures/test3.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );
    ////m_pTextures[3] = g_pTextureManager->CreateTexture( "Data/Textures/test4.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

    //m_pScriptFiles[0] = g_pFileManager->RequestFile( "Data/Scripts/Player.lua" );
    //m_pScriptFiles[1] = g_pFileManager->RequestFile( "Data/Scripts/Enemy.lua" );
    ////m_pScriptFiles[2] = g_pFileManager->RequestFile( "Data/Scripts/TestScript.lua" );
    ////m_pScriptFiles[3] = g_pFileManager->RequestFile( "Data/Scripts/TestScript.lua" );

#if MYFW_USING_WX
    m_pEditorState->m_pMousePickerFBO = g_pTextureManager->CreateFBO( 0, 0, GL_NEAREST, GL_NEAREST, false, false, false );
#endif //MYFW_USING_WX

    // setup our shaders
    //m_pShader_TintColor = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Tint Color" );
    //m_pShader_TestNormals = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Test-Normals" );
    //m_pShader_Texture = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Texture" );
    m_pShader_TransformGizmo = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Editor - Transform Gizmo" );
    m_pShader_MousePicker = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Editor - Mouse Picker" );

    //m_pShader_TintColor->SetFileForAllPasses( "Data/Shaders/Shader_TintColor" );
    //m_pShader_TestNormals->SetFileForAllPasses( "Data/Shaders/Shader_TestNormals" );
    //m_pShader_Texture->SetFileForAllPasses( "Data/Shaders/Shader_Texture" );
    m_pShader_TransformGizmo->SetFileForAllPasses( "Data/Shaders/Shader_TintColor" );
    m_pShader_MousePicker->SetFileForAllPasses( "Data/Shaders/Shader_TintColor" );

    // Initialize our component system.
    m_pComponentSystemManager = MyNew ComponentSystemManager( MyNew GameComponentTypeManager );

    // initialize lua state and register any variables needed.
    m_pLuaGameState = MyNew LuaGameState;

    // create a 3D camera, renders first... created first so GetFirstCamera() will get the game cam.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetSceneID( 1 );
        pGameObject->SetName( "Main Camera" );
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 10 ) );

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = false;
        pComponentCamera->m_LayersToRender = Layer_MainScene;
    }

#if MYFW_USING_WX
    // create a 3D X/Z plane grid
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 99999 );
        pGameObject->SetName( "3D Grid Plane" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, 99999 );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true; // manually drawn when in editor mode.
            pComponentMesh->SetShader( m_pShader_TransformGizmo );
            pComponentMesh->m_LayersThisExistsOn = Layer_Editor;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorLineGridXZ( Vector3(0,0,0), 1, 5 );
            pComponentMesh->m_pMesh->m_Tint.Set( 150, 150, 150, 255 );
            pComponentMesh->m_PrimitiveType = pComponentMesh->m_pMesh->m_PrimitiveType;
        }

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_pEditorState->m_p3DGridPlane = pGameObject;
    }

    // create a 3d transform gizmo for each axis.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 99999 );
        pGameObject->SetName( "3D Transform Gizmo - x-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, 99999 );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetShader( m_pShader_TransformGizmo );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.1f, ColorByte(255, 100, 100, 255) );
            pComponentMesh->m_PrimitiveType = pComponentMesh->m_pMesh->m_PrimitiveType;
        }
        //pGameObject->m_pComponentTransform->SetRotation( Vector3( 0, 90, 0 ) );

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] = pGameObject;
    }
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 99999 );
        pGameObject->SetName( "3D Transform Gizmo - y-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, 99999 );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetShader( m_pShader_TransformGizmo );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.1f, ColorByte(100, 255, 100, 255) );
            pComponentMesh->m_PrimitiveType = pComponentMesh->m_pMesh->m_PrimitiveType;
        }
        //pGameObject->m_pComponentTransform->SetRotation( Vector3( 0, 0, 90 ) );

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[1] = pGameObject;
    }
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 99999 );
        pGameObject->SetName( "3D Transform Gizmo - z-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, 99999 );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetShader( m_pShader_TransformGizmo );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.1f, ColorByte(100, 100, 255, 255) );
            pComponentMesh->m_PrimitiveType = pComponentMesh->m_pMesh->m_PrimitiveType;
        }
        //pGameObject->m_pComponentTransform->SetRotation( Vector3( -90, 0, 0 ) );

        //m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[2] = pGameObject;
    }

    // create a 3D editor camera, renders editor view.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 1 );
        pGameObject->SetName( "Editor Camera" );
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 10 ) );

        // add an editor scene camera
        {
            pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
            pComponentCamera->SetDesiredAspectRatio( 640, 960 );
            pComponentCamera->m_Orthographic = false;
            pComponentCamera->m_LayersToRender = Layer_Editor | Layer_MainScene;
            pComponentCamera->m_ClearColorBuffer = true;
            pComponentCamera->m_ClearDepthBuffer = true;

            // add the camera component to the list, but disabled, so it won't render.
            pComponentCamera->m_Enabled = false;
            //m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        // add a foreground camera for the transform gizmo only ATM.
        {
            pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
            pComponentCamera->SetDesiredAspectRatio( 640, 960 );
            pComponentCamera->m_Orthographic = false;
            pComponentCamera->m_LayersToRender = Layer_EditorFG;
            pComponentCamera->m_ClearColorBuffer = false;
            pComponentCamera->m_ClearDepthBuffer = true;

            // add the camera component to the list, but disabled, so it won't render.
            pComponentCamera->m_Enabled = false;
            //m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        m_pEditorState->m_pEditorCamera = pGameObject;
    }
#endif

    // create a 2D camera, renders after 3d, for hud.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetSceneID( 1 );
        pGameObject->SetName( "Hud Camera" );

        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera, 1 );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = true;
        pComponentCamera->m_LayersToRender = Layer_HUD;
        pComponentCamera->m_ClearColorBuffer = false;
        pComponentCamera->m_ClearDepthBuffer = false;
    }

    //// create a player game object and attach a mesh(sprite) component to it.
    //{
    //    pGameObject = m_pComponentSystemManager->CreateGameObject();
    //    pGameObject->SetSceneID( 1 );
    //    pGameObject->SetName( "Player Object" );

    //    pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( ComponentType_Sprite );
    //    if( pComponentSprite )
    //    {
    //        pComponentSprite->SetSceneID( 1 );
    //        pComponentSprite->SetShader( m_pShader_TintColor );
    //        pComponentSprite->m_Size.Set( 50.0f, 50.0f );
    //        pComponentSprite->m_Tint.Set( 255, 0, 0, 255 );
    //        pComponentSprite->m_LayersThisExistsOn = Layer_HUD;
    //    }
    //    pComponentLuaScript = (ComponentLuaScript*)pGameObject->AddNewComponent( ComponentType_LuaScript );
    //    if( pComponentLuaScript )
    //    {
    //        pComponentLuaScript->SetSceneID( 1 );
    //        pComponentLuaScript->SetScriptFile( m_pScriptFiles[0] );
    //    }
    //    //pGameObject->AddNewComponent( ComponentType_InputTrackMousePos ); // replace with lua script
    //    pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 0 ) );//m_GameWidth/2, m_GameHeight/2, 0 ) );

    //    pPlayer = pGameObject;
    //}

    //// create an enemy game object and attach a mesh(sprite) and AI component to it.
    //{
    //    pGameObject = m_pComponentSystemManager->CreateGameObject();
    //    pGameObject->SetSceneID( 1 );
    //    pGameObject->SetName( "Second Object" );

    //    pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( ComponentType_Sprite );
    //    if( pComponentSprite )
    //    {
    //        pComponentSprite->SetSceneID( 1 );
    //        pComponentSprite->SetShader( m_pShader_TintColor );
    //        pComponentSprite->m_Size.Set( 50.0f, 50.0f );
    //        pComponentSprite->m_Tint.Set( 0, 255, 0, 255 );
    //        pComponentSprite->m_LayersThisExistsOn = Layer_HUD;
    //    }
    //    pComponentLuaScript = (ComponentLuaScript*)pGameObject->AddNewComponent( ComponentType_LuaScript );
    //    if( pComponentLuaScript )
    //    {
    //        pComponentLuaScript->SetSceneID( 1 );
    //        pComponentLuaScript->SetScriptFile( m_pScriptFiles[1] );
    //    }
    //    //pComponentAIChasePlayer = (ComponentAIChasePlayer*)pGameObject->AddNewComponent( ComponentType_AIChasePlayer );
    //    //if( pComponentAIChasePlayer )
    //    //{
    //    //    pComponentAIChasePlayer->SetSceneID( 1 );
    //    //    pComponentAIChasePlayer->m_pPlayerComponentTransform = pPlayer->m_pComponentTransform;
    //    //}
    //}

    //// create a cube in the 3d scene.
    //{
    //    pGameObject = m_pComponentSystemManager->CreateGameObject();
    //    pGameObject->SetSceneID( 1 );
    //    pGameObject->SetName( "Cube" );

    //    pComponentMeshOBJ = (ComponentMeshOBJ*)pGameObject->AddNewComponent( ComponentType_MeshOBJ );
    //    if( pComponentMeshOBJ )
    //    {
    //        pComponentMeshOBJ->SetSceneID( 1 );
    //        pComponentMeshOBJ->SetShader( m_pShader_TestNormals );
    //        pComponentMeshOBJ->m_pOBJFile = m_pOBJTestFiles[0];
    //        pComponentMeshOBJ->m_LayersThisExistsOn = Layer_MainScene;
    //    }
    //    pComponentCollisionObject = (ComponentCollisionObject*)pGameObject->AddNewComponent( ComponentType_CollisionObject );
    //    pComponentCollisionObject->SetSceneID( 1 );
    //    pComponentCollisionObject->m_Mass = 1;
    //}

    //// create a plane in the 3d scene.
    //{
    //    pGameObject = m_pComponentSystemManager->CreateGameObject();
    //    pGameObject->SetSceneID( 1 );
    //    pGameObject->SetName( "Plane" );
    //    pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, -3, 0 ) );
    //    pGameObject->m_pComponentTransform->SetScale( Vector3( 10, 0.1f, 10 ) );

    //    pComponentMeshOBJ = (ComponentMeshOBJ*)pGameObject->AddNewComponent( ComponentType_MeshOBJ );
    //    if( pComponentMeshOBJ )
    //    {
    //        pComponentMeshOBJ->SetSceneID( 1 );
    //        pComponentMeshOBJ->SetShader( m_pShader_TintColor );
    //        pComponentMeshOBJ->m_pOBJFile = m_pOBJTestFiles[0];
    //        pComponentMeshOBJ->m_LayersThisExistsOn = Layer_MainScene;
    //    }
    //    pComponentCollisionObject = (ComponentCollisionObject*)pGameObject->AddNewComponent( ComponentType_CollisionObject );
    //    pComponentCollisionObject->SetSceneID( 1 );
    //}

    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );

#if !MYFW_USING_WX
    m_pSceneFileToLoad = RequestFile( "Data/Scenes/World.scene" );
    m_SceneLoaded = false;
#endif
}

double GameEntityComponentTest::Tick(double TimePassed)
{
    if( TimePassed > 0.2 )
        TimePassed = 0.2;

    //if( TimePassed == 0 && m_EditorMode == false )
    //    LOGInfo( LOGTag, "Tick: %f\n", TimePassed );
    //LOGInfo( LOGTag, "Tick: %f\n", TimePassed );

    float TimeUnpaused = TimePassed;

    GameCore::Tick( TimePassed );

#if !MYFW_USING_WX
    if( m_SceneLoaded == false && m_pSceneFileToLoad && m_pSceneFileToLoad->m_FileReady )
    {
        LoadScene( m_pSceneFileToLoad->m_pBuffer, 1 );
        m_pComponentSystemManager->OnPlay();
        SAFE_RELEASE( m_pSceneFileToLoad );
    }
#endif

#if MYFW_USING_WX
    m_pEditorState->m_pTransformGizmo->Tick( TimePassed, m_pEditorState );
#endif

    // tick all components.
    {
        if( m_EditorMode )
            TimePassed = 0;
        
        if( m_Paused )
            TimePassed = m_PauseTimeToAdvance;

        m_PauseTimeToAdvance = 0;

        m_pComponentSystemManager->Tick( TimePassed );
    }

    if( m_EditorMode == false )
    {
        m_TimeSinceLastPhysicsStep += TimePassed;

        while( m_TimeSinceLastPhysicsStep > 1/60.0f )
        {
            m_TimeSinceLastPhysicsStep -= 1/60.0f;
            m_pBulletWorld->PhysicsStep();
            //LOGInfo( LOGTag, "m_pBulletWorld->PhysicsStep()\n" );
        }
    }

    // update the global unpaused time.
    if( m_EditorMode )
        return TimeUnpaused;
    else
        return TimePassed;
}

void GameEntityComponentTest::OnDrawFrame()
{
    GameCore::OnDrawFrame();

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 )
    {
        // draw editor camera and editorFG camera
        for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
        {
            ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );

            if( pCamera )
                pCamera->OnDrawFrame();
        }
    }
    else
#endif
    {
        // draw all components.
        m_pComponentSystemManager->OnDrawFrame();
    }
}

void GameEntityComponentTest::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    GameCore::OnTouch( action, id, x, y, pressure, size );

#if MYFW_USING_WX
    //if( m_EditorMode )
    {
        if( g_GLCanvasIDActive == 1 )
        {
            ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();

            // prefer 0,0 at bottom left.
            y = pCamera->m_WindowHeight - y;

            if( m_pEditorState->m_pTransformGizmo->HandleInput( this, -1, -1, action, id, x, y, pressure ) )
                return;

            HandleEditorInput( -1, -1, action, id, x, y, pressure );

            return;
        }
    }

    if( g_GLCanvasIDActive != 0 )
    {
        return;
    }
#endif

    // mouse moving without button down.
    if( id == -1 )
        return;

    // TODO: get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();

    // prefer 0,0 at bottom left.
    y = pCamera->m_WindowHeight - y;

    // convert mouse to x/y in Camera2D space. TODO: put this in camera component.
    x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - pCamera->m_WindowStartX) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
    y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + pCamera->m_WindowStartY) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;

    m_pComponentSystemManager->OnTouch( action, id, x, y, pressure, size );
}

void GameEntityComponentTest::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    GameCore::OnButtons( action, id );
}

void GameEntityComponentTest::OnKey(GameCoreButtonActions action, int keycode, int unicodechar)
{
    GameCore::OnKey( action, keycode, unicodechar );

    if( action == GCBA_Down )
    {
#if MYFW_USING_WX
        if( m_EditorMode )
        {
            if( keycode == 'P' ||
                keycode == '.' || 
                keycode == '[' || 
                keycode == ']' ) // if press "Play"
            {
                SaveScene( "temp_editor_onplay.scene" );
                m_EditorMode = false;
                m_Paused = true;
                if( keycode == 'P' )
                    m_Paused = false;
                g_pGameMainFrame->SetWindowPerspectiveToDefault();
                m_pComponentSystemManager->OnPlay();
                return;
            }
        }
        else //if( m_EditorMode == false )
        {
            if( keycode == 'P' ) // if press "Stop"
            {
                // Clear out the component manager of all components and gameobjects
                UnloadScene( 0 ); // unload runtime created objects only.
                LoadSceneFromFile( "temp_editor_onplay.scene", 1 );
                m_EditorMode = true;
                m_Paused = false;
                g_pGameMainFrame->SetWindowPerspectiveToDefault();
                m_pEditorState->ClearSelectedObjectsAndComponents();
                m_pComponentSystemManager->OnStop();

                m_pComponentSystemManager->SyncAllRigidBodiesToObjectTransforms();
                return;
            }

            if( keycode == '.' )
            {
                m_Paused = !m_Paused;
                return;
            }

            if( keycode == ']' )
            {
                m_Paused = true;
                m_PauseTimeToAdvance = 1/60.0f;
                return;
            }

            if( keycode == '[' )
            {
                m_Paused = true;
                m_PauseTimeToAdvance = 1.0f;
                return;
            }
        }

        if( g_GLCanvasIDActive == 1 )
        {
            HandleEditorInput( action, keycode, -1, -1, -1, -1, -1 );
            return;
        }
        else
#endif
        {
            // TODO: hack, need to fix, did this for GGJ to make keys work
            m_pComponentSystemManager->OnButtons( GCBA_Down, (GameCoreButtonIDs)keycode );
        }
    }

    if( action == GCBA_Up )
    {
#if MYFW_USING_WX
        if( g_GLCanvasIDActive == 1 )
        {
            HandleEditorInput( action, keycode, -1, -1, -1, -1, -1 );
            return;
        }
        else
#endif
        {
            // TODO: hack, need to fix, did this for GGJ to make keys work
            m_pComponentSystemManager->OnButtons( GCBA_Up, (GameCoreButtonIDs)keycode );
        }
    }

    if( action == GCBA_Held )
    {
#if MYFW_USING_WX
        if( g_GLCanvasIDActive == 1 )
        {
            HandleEditorInput( action, keycode, -1, -1, -1, -1, -1 );
            return;
        }
        //else
#endif
    }
}

void GameEntityComponentTest::HandleEditorInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
#if MYFW_USING_WX
    if( keycode == MYKEYCODE_LCTRL )
    {
        if( keyaction == GCBA_Down ) m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keyaction == GCBA_Up ) m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    }
    //else if( keycode == MYKEYCODE_LALT )
    //{
    //    if( keyaction == GCBA_Down ) m_pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
    //    if( keyaction == GCBA_Up ) m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
    //}
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
        // select an object when mouse is released and on the same pixel it went down.
        // TODO: make same "pixel test" a "total travel < small number" test.
        if( mouseaction == GCBA_Up && id == 0 && m_pEditorState->m_CurrentMousePosition == m_pEditorState->m_MouseLeftDownLocation )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( x, y );

            // don't select the gizmos. TODO: give object a 'selectable' flag or something.
            if( pObject != m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] &&
                pObject != m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[1] &&
                pObject != m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[2] )
            {
                // if control isn't held, then deselect all objects first.
                if( (m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control) == 0 )
                {
                    m_pEditorState->ClearSelectedObjectsAndComponents();
                }

                if( pObject && m_pEditorState->IsObjectSelected( pObject ) == false )
                {
                    m_pEditorState->m_pSelectedObjects.push_back( pObject );
                    // select the object in the object tree.
                    g_pPanelObjectList->SelectObject( pObject ); // passing in 0 will unselect all items.
                }
            }
        }

        if( mouseaction == GCBA_Down && id == 0 )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( x, y );

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
                    GameObject* pNewObject = g_pComponentSystemManager->EditorCopyGameObject( selectedobjects[i] );
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
            Vector3 dir = Vector3( 0, 0, 1 ) * -(pressure/abs(pressure));
            float speed = 600.0f;
            if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                speed *= 5;

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * speed * m_TimePassedUnpausedLastFrame );
        }

        // if space is held, left button will pan the camera around.  or just middle button
        if( ( mods & MODIFIERKEY_LeftMouse && mods & MODIFIERKEY_Space ) ||
            mods & MODIFIERKEY_MiddleMouse
          )
        {
            Vector2 dir = m_pEditorState->m_LastMousePosition - m_pEditorState->m_CurrentMousePosition;

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * 0.05f );
        }
        // if left or right mouse is down, rotate the camera.
        else if( m_pEditorState->m_EditorActionState == EDITORACTIONSTATE_None &&
                 ( mods & MODIFIERKEY_LeftMouse || mods & MODIFIERKEY_RightMouse ) )
        {
            // rotate the camera around selected object or a point 10 units in front of the camera.
            Vector2 dir = m_pEditorState->m_CurrentMousePosition - m_pEditorState->m_LastMousePosition;

            Vector3 pivot;
            float distancefrompivot;

            if( mods & MODIFIERKEY_LeftMouse && m_pEditorState->m_pSelectedObjects.size() > 0 && m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )//m_pEditorState->m_pSelectedObjects[0] )
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
                    mattemp.TranslatePreRotScale( 0, 0, -10 );
                    pivot = mattemp.GetTranslation();
                    distancefrompivot = 10;
                }
            }

            if( dir.LengthSquared() > 0 )
            {
                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                Vector3 pos = matLocalCamera->GetTranslation();
                Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();
                float speed = 200.0f;

                angle.y += dir.x * speed * m_TimePassedUnpausedLastFrame;
                angle.x -= dir.y * speed * m_TimePassedUnpausedLastFrame;
                MyClamp( angle.x, -90.0f, 90.0f );

                matLocalCamera->SetIdentity();
                matLocalCamera->Translate( 0, 0, distancefrompivot );
                matLocalCamera->Rotate( angle.x, 1, 0, 0 );
                matLocalCamera->Rotate( angle.y, 0, 1, 0 );
                matLocalCamera->Translate( pivot );

                // pull the pos/angle from the local matrix and update the values for the watch window.
                {
                    pCamera->m_pComponentTransform->UpdatePosAndRotFromLocalMatrix();
                    //Vector3 pos = matLocalCamera->GetTranslation();
                    //Vector3 angle = matLocalCamera->GetEulerAngles() * 180.0f/PI;
                    //pCamera->m_pComponentTransform->SetPosition( pos );
                    //pCamera->m_pComponentTransform->SetRotation( angle );

                    //LOGInfo( LOGTag, "cam pos (%0.2f, %0.2f, %0.2f)\n", pos.x, pos.y, pos.z );
                }
            }
        }
    }

    // handle editor keys
    if( keyaction == GCBA_Held )
    {
        // get the editor camera's local transform.
        ComponentCamera* pCamera = m_pEditorState->GetEditorCamera();
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

        // WASD to move camera
        Vector3 dir( 0, 0, 0 );
        if( keycode == 'W' ) dir.z += -1;
        if( keycode == 'A' ) dir.x += -1;
        if( keycode == 'S' ) dir.z +=  1;
        if( keycode == 'D' ) dir.x +=  1;
        if( keycode == 'Q' ) dir.y +=  1;
        if( keycode == 'Z' ) dir.y -=  1;

        float speed = 7.0f;
        if( m_pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            speed *= 5;

        if( dir.LengthSquared() > 0 )
            matLocalCamera->TranslatePreRotScale( dir * speed * m_TimePassedUnpausedLastFrame );
    }

    // check for mouse ups and clear the states.
    if( mouseaction != -1 )
    {
        if( mouseaction == GCBA_Up )
        {
            if( id == 0 )
            {
                m_pEditorState->m_MouseLeftDownLocation = Vector2( -1, -1 );
                m_pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_LeftMouse;
                m_pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;

                // add translation to undo stack, action itself is done each frame.  We only want to undo to last mouse down.
                if( m_pEditorState->m_DistanceTranslated.LengthSquared() != 0 )
                {
                    g_pGameMainFrame->m_pCommandStack->Add( MyNew EditorCommand_MoveObjects( m_pEditorState->m_DistanceTranslated, m_pEditorState->m_pSelectedObjects ) );
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
}

void GameEntityComponentTest::SaveScene(const char* fullpath)
{
    char* savestring = g_pComponentSystemManager->SaveSceneToJSON();

    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t error = fopen_s( &filehandle, fullpath, "w" );//"Data/Scenes/test.scene", "w" );
#else
    filehandle = fopen( fullpath, "w" );//"Data/Scenes/test.scene", "w" );
#endif
    if( filehandle )
    {
        fprintf( filehandle, "%s", savestring );
        fclose( filehandle );
    }

    free( savestring );
}

void GameEntityComponentTest::UnloadScene(unsigned int sceneid)
{
    // reset the editorstate structure.
#if MYFW_USING_WX
    m_pEditorState->UnloadScene();
#endif //MYFW_USING_WX

    g_pComponentSystemManager->UnloadScene( false, sceneid );
}

#if MYFW_USING_WX
void GameEntityComponentTest::LoadSceneFromFile(const char* fullpath, unsigned int sceneid)
{
    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t err = fopen_s( &filehandle, fullpath, "rb" );//"Data/Scenes/test.scene", "rb" );
#else
    filehandle = fopen( fullpath, "rb" );//"Data/Scenes/test.scene", "rb" );
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

        LoadScene( jsonstr, sceneid );

        delete[] jsonstr;
    }
}
#endif //MYFW_USING_WX

void GameEntityComponentTest::LoadScene(const char* buffer, unsigned int sceneid)
{
    // reset the editorstate structure.
#if MYFW_USING_WX
    m_pEditorState->UnloadScene();
#endif //MYFW_USING_WX

    g_pComponentSystemManager->LoadSceneFromJSON( buffer, sceneid );

#if MYFW_USING_WX
    m_EditorMode = true;

    g_pGameMainFrame->ResizeViewport();
#endif

    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );
}

void GameEntityComponentTest::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    GameCore::OnSurfaceChanged( startx, starty, width, height );

    glEnable( GL_CULL_FACE );
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

    m_GameWidth = 640.0f;
    m_GameHeight = 960.0f;

    // reset the viewport sizes of the game or editor cameras.
    if( m_pComponentSystemManager )
    {
#if MYFW_USING_WX
        if( g_GLCanvasIDActive != 0 )
        {
            for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
            {
                ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
                pCamera->OnSurfaceChanged( startx, starty, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
            }

            if( m_pEditorState->m_pMousePickerFBO )
            {
                if( m_pEditorState->m_pMousePickerFBO->m_TextureWidth < width || m_pEditorState->m_pMousePickerFBO->m_TextureHeight < height )
                {
                    // the FBO will be recreated during the texturemanager tick.
                    g_pTextureManager->InvalidateFBO( m_pEditorState->m_pMousePickerFBO );
                    m_pEditorState->m_pMousePickerFBO->Setup( width, height, GL_NEAREST, GL_NEAREST, true, true, false );
                }
                else
                {
                    m_pEditorState->m_pMousePickerFBO->m_Width = width;
                    m_pEditorState->m_pMousePickerFBO->m_Height = height;
                }
            }
        }
        else
#endif
        {
            m_pComponentSystemManager->OnSurfaceChanged( startx, starty, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
        }
    }
}

#if MYFW_USING_WX
GameObject* GameEntityComponentTest::GetObjectAtPixel(unsigned int x, unsigned int y)
{
    // render the scene to a FBO.
    m_pEditorState->m_pMousePickerFBO->Bind();

    m_pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, m_pEditorState->m_pMousePickerFBO->m_Width, m_pEditorState->m_pMousePickerFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    for( unsigned int i=0; i<m_pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_pEditorState->m_pEditorCamera->m_Components[i] );
        m_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, m_pShader_MousePicker );
        glClear( GL_DEPTH_BUFFER_BIT );
    }

    m_pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );

    // get a pixel from the FBO.
    unsigned char pixel[4];
    glReadPixels( x - (unsigned int)m_WindowStartX, y - (unsigned int)m_WindowStartY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel );

    unsigned int id = pixel[0] + pixel[1]*256 + pixel[2]*256*256 + pixel[3]*256*256*256;
    //LOGInfo( LOGTag, "pixel - %d, %d, %d, %d - id - %d\n", pixel[0], pixel[1], pixel[2], pixel[3], id );

    m_pEditorState->m_pMousePickerFBO->Unbind();

    // find the object clicked on.
    GameObject* pGameObject = m_pComponentSystemManager->FindGameObjectByID( id );

    // if we didn't click on something, check if it's the transform gizmo.
    //   has to be checked manually since they are unmanaged.
    if( pGameObject == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            if( m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[i]->GetID() == id )
            {
                return m_pEditorState->m_pTransformGizmo->m_pTransformGizmos[i];
            }
        }
    }

    return pGameObject;
}

void GameEntityComponentTest::GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end)
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
    Vector3 rayworld = (invView * Vector4( rayview.x, rayview.y, -1, 0 )).XYZ();

    // define the ray.
    Vector3 raystart = pCamera->m_pComponentTransform->GetPosition();
    Vector3 rayend = raystart + rayworld * 10000;

    start->Set( raystart.x, raystart.y, raystart.z );
    end->Set( rayend.x, rayend.y, rayend.z );
}
#endif //MYFW_USING_WX

#if MYFW_USING_WX
void GameEntityComponentTest::OnObjectListTreeSelectionChanged()
{
    if( m_pEditorState )
    {
        m_pEditorState->m_pSelectedObjects.clear();
        m_pEditorState->m_pSelectedComponents.clear();
    }
}
#endif //MYFW_USING_WX
