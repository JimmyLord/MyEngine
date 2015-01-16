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

    m_pShader_TintColor = 0;
    m_pShader_TestNormals = 0;
    m_pShader_Texture = 0;
    m_pShader_MousePicker = 0;

    m_GameWidth = 0;
    m_GameHeight = 0;

    for( int i=0; i<4; i++ )
        m_pOBJTestFiles[i] = 0;

    for( int i=0; i<4; i++ )
        m_pTextures[i] = 0;

    for( int i=0; i<4; i++ )
        m_pScriptFiles[i] = 0;

    m_pBulletWorld = MyNew BulletWorld();

    m_EditorState.m_pMousePickerFBO = 0;

    m_EditorState.m_p3DGridPlane = 0;
    for( int i=0; i<3; i++ )
    {
        m_EditorState.m_pTransformGizmos[i] = 0;
    }
    m_EditorState.m_pEditorCamera = 0;

    m_pLuaGameState = 0;
}

GameEntityComponentTest::~GameEntityComponentTest()
{
    SAFE_DELETE( m_pLuaGameState );

    SAFE_DELETE( m_pShader_TintColor );
    SAFE_DELETE( m_pShader_TestNormals );
    SAFE_DELETE( m_pShader_Texture );
    SAFE_DELETE( m_pShader_MousePicker );

    for( int i=0; i<4; i++ )
    {
        if( m_pOBJTestFiles[i] )
            g_pFileManager->FreeFile( m_pOBJTestFiles[i] );
    }

    for( int i=0; i<4; i++ )
    {
        SAFE_RELEASE( m_pTextures[i] );
    }

    for( int i=0; i<4; i++ )
    {
        if( m_pScriptFiles[i] )
            g_pFileManager->FreeFile( m_pScriptFiles[i] );
    }

    SAFE_DELETE( m_EditorState.m_p3DGridPlane );
    for( int i=0; i<3; i++ )
    {
        SAFE_DELETE( m_EditorState.m_pTransformGizmos[i] );
    }
    SAFE_DELETE( m_EditorState.m_pEditorCamera );

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

    m_EditorState.m_pMousePickerFBO = g_pTextureManager->CreateFBO( 0, 0, GL_NEAREST, GL_NEAREST, false, false, false );

    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );

    // setup our shaders
    m_pShader_TintColor = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Tint Color" );
    m_pShader_TestNormals = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Test-Normals" );
    m_pShader_Texture = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Texture" );
    m_pShader_MousePicker = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Mouse Picker" );

    m_pShader_TintColor->SetFileForAllPasses( "Data/Shaders/Shader_TintColor" );
    m_pShader_TestNormals->SetFileForAllPasses( "Data/Shaders/Shader_TestNormals" );
    m_pShader_Texture->SetFileForAllPasses( "Data/Shaders/Shader_Texture" );
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
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_Editor;
            pComponentMesh->m_pMesh->m_Tint.Set( 150, 150, 150, 255 );
            pComponentMesh->m_pMesh->CreateEditorLineGridXZ( Vector3(0,0,0), 1, 5 );
        }

        m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_EditorState.m_p3DGridPlane = pGameObject;
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
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.1f, ColorByte(255, 100, 100, 255) );
        }
        pGameObject->m_pComponentTransform->SetRotation( Vector3( 0, 90, 0 ) );

        m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_EditorState.m_pTransformGizmos[0] = pGameObject;
    }
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 99999 );
        pGameObject->SetName( "3D Transform Gizmo - y-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, 99999 );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.1f, ColorByte(100, 255, 100, 255) );
        }
        pGameObject->m_pComponentTransform->SetRotation( Vector3( -90, 0, 0 ) );

        m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_EditorState.m_pTransformGizmos[1] = pGameObject;
    }
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( 99999 );
        pGameObject->SetName( "3D Transform Gizmo - z-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, 99999 );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.1f, ColorByte(100, 100, 255, 255) );
        }
        pGameObject->m_pComponentTransform->SetRotation( Vector3( 0, 180, 0 ) );

        m_pComponentSystemManager->AddComponent( pComponentMesh );

        m_EditorState.m_pTransformGizmos[2] = pGameObject;
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
            m_pComponentSystemManager->AddComponent( pComponentCamera );
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
            m_pComponentSystemManager->AddComponent( pComponentCamera );
        }

        m_EditorState.m_pEditorCamera = pGameObject;
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
    LoadScene( "Data/Scenes/Test.scene", 1 );
    m_pComponentSystemManager->OnPlay();
#endif
}

void GameEntityComponentTest::Tick(double TimePassed)
{
    if( TimePassed > 0.2 )
        TimePassed = 0.2;

    GameCore::Tick( TimePassed );

    // tick all components.
    //if( m_EditorMode )
    {
        if( m_EditorMode )
            m_pComponentSystemManager->Tick( 0 );

#if MYFW_USING_WX
        for( int i=0; i<3; i++ )
        {
            ComponentRenderable* pRenderable = (ComponentRenderable*)m_EditorState.m_pTransformGizmos[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

            if( m_EditorState.m_pSelectedGameObject )
            {
                pRenderable->m_Visible = true;

                // move the gizmo to the object position.
                Vector3 pos = m_EditorState.m_pSelectedGameObject->m_pComponentTransform->GetPosition();
                m_EditorState.m_pTransformGizmos[i]->m_pComponentTransform->SetPosition( pos );

                // rotate the gizmo.
                MyMatrix matrot;
                matrot.SetIdentity();
                if( i == 0 )
                    matrot.Rotate( 90, 0, 1, 0 );
                if( i == 1 )
                    matrot.Rotate( -90, 1, 0, 0 );
                if( i == 2 )
                    matrot.Rotate( 180, 0, 1, 0 );

                MyMatrix* localtransform = m_EditorState.m_pSelectedGameObject->m_pComponentTransform->GetLocalTransform();
                MyMatrix matrotobj;
                matrotobj.SetIdentity();
                matrotobj.CreateSRT( Vector3(1,1,1), localtransform->GetEulerAngles(), Vector3(0,0,0) );

                matrot = matrotobj * matrot;

                Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI;

                m_EditorState.m_pTransformGizmos[i]->m_pComponentTransform->SetRotation( rot );

                float distance = (m_EditorState.m_pEditorCamera->m_pComponentTransform->GetPosition() - pos).Length();
                m_EditorState.m_pTransformGizmos[i]->m_pComponentTransform->SetScale( Vector3( distance / 15.0f ) );
            }
            else
            {
                pRenderable->m_Visible = false;
            }
        }
#endif
    }
    //else
    {
        if( m_EditorMode == false )
        {
            if( TimePassed != 0 )
                m_pComponentSystemManager->Tick( TimePassed );
        }
    }

    if( m_EditorMode == false )
    {
        m_TimeSinceLastPhysicsStep += TimePassed;
        
        while( m_TimeSinceLastPhysicsStep > 1/60.0f )
        {
            m_TimeSinceLastPhysicsStep -= 1/60.0f;
            m_pBulletWorld->PhysicsStep();
        }
    }
}

void GameEntityComponentTest::OnDrawFrame()
{
    GameCore::OnDrawFrame();

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 1 )
    {
        // draw editor camera and editorFG camera
        for( unsigned int i=0; i<m_EditorState.m_pEditorCamera->m_Components.Count(); i++ )
        {
            ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_EditorState.m_pEditorCamera->m_Components[i] );
            
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
            ComponentCamera* pCamera = m_EditorState.GetEditorCamera();

            // prefer 0,0 at bottom left.
            y = pCamera->m_WindowHeight - y;

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

void GameEntityComponentTest::OnKeyDown(int keycode, int unicodechar)
{
#if MYFW_USING_WX
    if( m_EditorMode )
    {
        if( keycode == 'P' ) // if press "Play"
        {
            SaveScene( "temp_editor_onplay.scene" );
            m_EditorMode = false;
            m_pComponentSystemManager->OnPlay();
            return;
        }

        HandleEditorInput( 1, keycode, -1, -1, -1, -1, -1 );
        return;
    }

    if( keycode == 'P' ) // Play/Stop
    {
        if( m_EditorMode ) // if press "Play"
        {
        }
        else // if press "Stop"
        {
            // Clear out the component manager of all components and gameobjects
            UnloadScene( 0 ); // unload runtime created objects only.
            LoadScene( "temp_editor_onplay.scene", 1 );
            m_EditorMode = true;
            m_EditorState.m_pSelectedGameObject = 0;
            m_pComponentSystemManager->OnStop();

            m_pComponentSystemManager->SyncAllRigidBodiesToObjectTransforms();
        }
    }
#endif
}

void GameEntityComponentTest::OnKeyUp(int keycode, int unicodechar)
{
#if MYFW_USING_WX
    if( m_EditorMode && g_GLCanvasIDActive == 1 )
    {
        HandleEditorInput( 0, keycode, -1, -1, -1, -1, -1 );
        return;
    }
#endif
}

void GameEntityComponentTest::HandleEditorInput(int keydown, int keycode, int action, int id, float x, float y, float pressure)
{
#if MYFW_USING_WX
    if( keycode == MYKEYCODE_LCTRL )
    {
        if( keydown == 1 ) m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keydown == 0 ) m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    }
    //else if( keycode == MYKEYCODE_LALT )
    //{
    //    if( keydown == 1 ) m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_Control;
    //    if( keydown == 0 ) m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    //}
    else if( keycode == MYKEYCODE_LSHIFT )
    {
        if( keydown == 1 ) m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_Shift;
        if( keydown == 0 ) m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
    }
    else if( keycode == ' ' )
    {
        if( keydown == 1 ) m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_Space;
        if( keydown == 0 ) m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_Space;
    }
    if( action != -1 )
    {
        m_EditorState.m_CurrentMousePosition.Set( x, y );
        //m_EditorState.m_LastMousePosition.Set( x, y );

        if( action == GCBA_Down )
        {
            if( id == 0 )
            {
                m_EditorState.m_MouseLeftDownLocation = m_EditorState.m_CurrentMousePosition;
                m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_LeftMouse;
            }
            if( id == 1 )
            {
                m_EditorState.m_MouseRightDownLocation = m_EditorState.m_CurrentMousePosition;
                m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_RightMouse;
            }
            if( id == 2 )
            {
                m_EditorState.m_MouseMiddleDownLocation = m_EditorState.m_CurrentMousePosition;
                m_EditorState.m_ModifierKeyStates |= MODIFIERKEY_MiddleMouse;
            }
        }
    }
   
    if( m_EditorState.m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        // select an object when mouse is released and on the same pixel it went down.
        // TODO: make same "pixel test" a "total travel < small number" test.
        if( action == GCBA_Up && id == 0 && m_EditorState.m_CurrentMousePosition == m_EditorState.m_MouseLeftDownLocation )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( x, y );

            // don't select the gizmos. TODO: give object a 'selectable' flag or something.
            if( pObject != m_EditorState.m_pTransformGizmos[0] &&
                pObject != m_EditorState.m_pTransformGizmos[1] &&
                pObject != m_EditorState.m_pTransformGizmos[2] )
            {
                m_EditorState.m_pSelectedGameObject = pObject;

                // select the object in the object tree.
                g_pPanelObjectList->SelectObject( m_EditorState.m_pSelectedGameObject );
            }
        }

        if( action == GCBA_Down && id == 0 )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( x, y );

            bool selectedgizmo = false;

            // translate to the right.
            if( pObject == m_EditorState.m_pTransformGizmos[0] )
            {
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_TranslateX;
                selectedgizmo = true;
            }
            if( pObject == m_EditorState.m_pTransformGizmos[1] )
            {
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_TranslateY;
                selectedgizmo = true;
            }
            if( pObject == m_EditorState.m_pTransformGizmos[2] )
            {
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_TranslateZ;
                selectedgizmo = true;
            }

            // if shift is held, make a copy of the object and control that one.
            if( selectedgizmo && m_EditorState.m_ModifierKeyStates & MODIFIERKEY_Shift )
            {
                GameObject* pNewObject = g_pComponentSystemManager->CopyGameObject( m_EditorState.m_pSelectedGameObject, "Duplicated Game Object" );
                if( m_EditorMode )
                {
                    pNewObject->SetSceneID( m_EditorState.m_pSelectedGameObject->GetSceneID() );
                }

                m_EditorState.m_pSelectedGameObject = pNewObject;

                // select the object in the object tree.
                g_pPanelObjectList->SelectObject( pNewObject );
            }
        }

        if( m_EditorState.m_pSelectedGameObject )
        {
            // check if the mouse moved.
            Vector3 mousedragdir = m_EditorState.m_CurrentMousePosition - m_EditorState.m_LastMousePosition;

            if( mousedragdir.LengthSquared() != 0 )
            {
                // If the mouse moved, move the object along a plane.
                if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateX ||
                    m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateY ||
                    m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateZ ||
                    m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateXY ||
                    m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateXZ ||
                    m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateYZ )
                {
                    ComponentCamera* pCamera = m_EditorState.GetEditorCamera();
                    MyMatrix* pObjectTransform = &m_EditorState.m_pSelectedGameObject->m_pComponentTransform->m_Transform;

                    // create a plane based on the axis we want.
                    Vector3 axisvector;
                    Plane plane;
                    {
                        Vector3 normal;
                        if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateXY ||
                            m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateX )
                        {
                            normal = Vector3(0,0,1);
                            axisvector = Vector3(1,0,0);
                        }
                        else if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateXZ ||
                                 m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateZ )
                        {
                            normal = Vector3(0,1,0);
                            axisvector = Vector3(0,0,1);
                        }
                        else if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateYZ ||
                                 m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateY )
                        {
                            normal = Vector3(1,0,0);
                            axisvector = Vector3(0,1,0);
                        }

                        // transform the normal into the selected objects space.
                        plane.Set( (*pObjectTransform * Vector4( normal, 0 )).XYZ(), pObjectTransform->GetTranslation() );
                    }
                
                    // create a ray from a mouse click.

                    // Convert mouse coord into clip space.
                    Vector2 mouseclip;
                    mouseclip.x = (m_EditorState.m_CurrentMousePosition.x / m_WindowWidth) * 2.0f - 1.0f;
                    mouseclip.y = (m_EditorState.m_CurrentMousePosition.y / m_WindowHeight) * 2.0f - 1.0f;

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

                    // find the intersection point of the plane.
                    Vector3 result;
                    if( plane.IntersectRay( raystart, rayend, &result ) )
                    {
                        // lock to one of the 3 axis.
                        if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateX ||
                            m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateY ||
                            m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateZ )
                        {
                            axisvector = (*pObjectTransform * Vector4( axisvector, 0 )).XYZ();
                            axisvector.Normalize();
                            result -= pObjectTransform->GetTranslation();
                            result = axisvector * result.Dot( axisvector );
                            result += pObjectTransform->GetTranslation();
                        }

                        m_EditorState.m_pSelectedGameObject->m_pComponentTransform->SetPosition( result );
                    }
                }
            }
        }
    }

    // if mouse message. down, up or dragging.
    if( action != -1 )
    {
        unsigned int mods = m_EditorState.m_ModifierKeyStates;

        // get the editor camera's local transform.
        ComponentCamera* pCamera = m_EditorState.GetEditorCamera();
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

        // move camera in/out if mousewheel spinning
        if( action == GCBA_Wheel )
        {
            // pressure is also mouse wheel movement rate in wx configurations.
            Vector3 dir = Vector3( 0, 0, 1 ) * -(pressure/abs(pressure));

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * 1.5f );
        }

        // if space is held, left button will pan the camera around.  or just middle button
        if( ( mods & MODIFIERKEY_LeftMouse && mods & MODIFIERKEY_Space ) ||
            mods & MODIFIERKEY_MiddleMouse
          )
        {
            Vector2 dir = m_EditorState.m_LastMousePosition - m_EditorState.m_CurrentMousePosition;

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * 0.05f );
        }
        // if left mouse if down, rotate the camera.
        else if( mods & MODIFIERKEY_LeftMouse && m_EditorState.m_EditorActionState == EDITORACTIONSTATE_None )
        {
            // rotate the camera around selected object or a point 10 units in front of the camera.
            Vector2 dir = m_EditorState.m_CurrentMousePosition - m_EditorState.m_LastMousePosition;

            Vector3 pivot;
            float distancefrompivot;

            if( m_EditorState.m_pSelectedGameObject )
            {
                pivot = m_EditorState.m_pSelectedGameObject->m_pComponentTransform->m_Transform.GetTranslation();
                distancefrompivot = (matLocalCamera->GetTranslation() - pivot).Length();
            }
            else
            {
                MyMatrix mattemp = *matLocalCamera;
                mattemp.TranslatePreRotScale( 0, 0, -10 );
                pivot = mattemp.GetTranslation();
                distancefrompivot = 10;
            }

            if( dir.LengthSquared() > 0 )
            {
                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                Vector3 pos = matLocalCamera->GetTranslation();
                Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();
                angle.y += dir.x;
                angle.x -= dir.y;
                MyClamp( angle.x, -90.0f, 90.0f );

                matLocalCamera->SetIdentity();
                matLocalCamera->Translate( 0, 0, distancefrompivot );
                matLocalCamera->Rotate( angle.x, 1, 0, 0 );
                matLocalCamera->Rotate( angle.y, 0, 1, 0 );
                matLocalCamera->Translate( pivot );

                // pull the pos/angle from the local matrix and update the values for the watch window.
                {
                    Vector3 pos = matLocalCamera->GetTranslation();
                    Vector3 angle = matLocalCamera->GetEulerAngles();
                    angle *= 180.0f/PI;
                    pCamera->m_pComponentTransform->SetPosition( pos );
                    pCamera->m_pComponentTransform->SetRotation( angle );

                    //LOGInfo( LOGTag, "cam pos (%0.2f, %0.2f, %0.2f)\n", pos.x, pos.y, pos.z );
                }
            }
        }
    }

    if( action != -1 )
    {
        if( action == GCBA_Up )
        {
            if( id == 0 )
            {
                m_EditorState.m_MouseLeftDownLocation = Vector2( -1, -1 );
                m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_LeftMouse;
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_None;
            }
            else if( id == 1 )
            {
                m_EditorState.m_MouseRightDownLocation = Vector2( -1, -1 );
                m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_RightMouse;
            }
            else if( id == 2 )
            {
                m_EditorState.m_MouseMiddleDownLocation = Vector2( -1, -1 );
                m_EditorState.m_ModifierKeyStates &= ~MODIFIERKEY_MiddleMouse;
            }
        }
    }

    m_EditorState.m_LastMousePosition = m_EditorState.m_CurrentMousePosition;
#endif //MYFW_USING_WX
}

void GameEntityComponentTest::SaveScene(const char* fullpath)
{
    char* savestring = g_pComponentSystemManager->SaveSceneToJSON();

    FILE* filehandle;
    errno_t error = fopen_s( &filehandle, fullpath, "w" );//"Data/Scenes/test.scene", "w" );
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
    m_EditorState.UnloadScene();

    g_pComponentSystemManager->UnloadScene( false, sceneid );
}

void GameEntityComponentTest::LoadScene(const char* fullpath, unsigned int sceneid)
{
    // reset the editorstate structure.
    m_EditorState.UnloadScene();

    FILE* filehandle;
    errno_t err = fopen_s( &filehandle, fullpath, "rb" );//"Data/Scenes/test.scene", "rb" );

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

        g_pComponentSystemManager->LoadSceneFromJSON( jsonstr, sceneid );

        delete[] jsonstr;
    }

#if MYFW_USING_WX
    m_EditorMode = true;

    g_pGameMainFrame->ResizeViewport();
#endif

    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );
}

void GameEntityComponentTest::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    GameCore::OnSurfaceChanged( startx, starty, width, height );

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
            for( unsigned int i=0; i<m_EditorState.m_pEditorCamera->m_Components.Count(); i++ )
            {
                ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_EditorState.m_pEditorCamera->m_Components[i] );
                pCamera->OnSurfaceChanged( startx, starty, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );
            }

            if( m_EditorState.m_pMousePickerFBO )
            {
                if( m_EditorState.m_pMousePickerFBO->m_TextureWidth < width || m_EditorState.m_pMousePickerFBO->m_TextureHeight < height )
                {
                    // the FBO will be recreated during the texturemanager tick.
                    g_pTextureManager->InvalidateFBO( m_EditorState.m_pMousePickerFBO );
                    m_EditorState.m_pMousePickerFBO->Setup( width, height, GL_NEAREST, GL_NEAREST, true, true, false );
                }
                else
                {
                    m_EditorState.m_pMousePickerFBO->m_Width = width;
                    m_EditorState.m_pMousePickerFBO->m_Height = height;
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

GameObject* GameEntityComponentTest::GetObjectAtPixel(unsigned int x, unsigned int y)
{
    // render the scene to a FBO.
    m_EditorState.m_pMousePickerFBO->Bind();

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, m_EditorState.m_pMousePickerFBO->m_Width, m_EditorState.m_pMousePickerFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    for( unsigned int i=0; i<m_EditorState.m_pEditorCamera->m_Components.Count(); i++ )
    {
        ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( m_EditorState.m_pEditorCamera->m_Components[i] );
        m_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, m_pShader_MousePicker );
        glClear( GL_DEPTH_BUFFER_BIT );
    }

    // get a pixel from the FBO.
    unsigned char pixel[4];
    glReadPixels( x - (unsigned int)m_WindowStartX, y - (unsigned int)m_WindowStartY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel );

    unsigned int id = pixel[0] + pixel[1]*256 + pixel[2]*256*256 + pixel[3]*256*256*256;
    //LOGInfo( LOGTag, "pixel - %d, %d, %d, %d - id - %d\n", pixel[0], pixel[1], pixel[2], pixel[3], id );

    m_EditorState.m_pMousePickerFBO->Unbind();

    // find the object clicked on.
    GameObject* pGameObject = m_pComponentSystemManager->FindGameObjectByID( id );

    // if we didn't click on something, check if it's the transform gizmo.
    //   has to be checked manually since they are unmanaged.
    if( pGameObject == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            if( m_EditorState.m_pTransformGizmos[i]->GetID() == id )
            {
                return m_EditorState.m_pTransformGizmos[i];
            }
        }
    }

    return pGameObject;
}
