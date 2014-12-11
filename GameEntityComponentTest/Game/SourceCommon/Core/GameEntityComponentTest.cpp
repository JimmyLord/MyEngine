//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
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

    m_pBulletWorld = MyNew BulletWorld();

    m_EditorState.m_pMousePickerFBO = 0;
}

GameEntityComponentTest::~GameEntityComponentTest()
{
    SAFE_DELETE( m_pShader_TintColor );
    SAFE_DELETE( m_pShader_TestNormals );
    SAFE_DELETE( m_pShader_Texture );
    SAFE_DELETE( m_pShader_MousePicker );

    SAFE_DELETE( m_pComponentSystemManager );

    for( int i=0; i<4; i++ )
    {
        if( m_pOBJTestFiles[i] )
            g_pFileManager->FreeFile( m_pOBJTestFiles[i] );
    }

    for( int i=0; i<4; i++ )
    {
        SAFE_RELEASE( m_pTextures[i] );
    }

    SAFE_DELETE( m_pBulletWorld );

    SAFE_DELETE( m_EditorState.m_p3DGridPlane );
    for( int i=0; i<3; i++ )
    {
        SAFE_DELETE( m_EditorState.m_pTransformWidgets[i] );
    }
}

void GameEntityComponentTest::OneTimeInit()
{
    GameCore::OneTimeInit();

    GameObject* pPlayer = 0;
    GameObject* pGameObject;
    ComponentCamera* pComponentCamera;
    ComponentSprite* pComponentSprite;
    ComponentMesh* pComponentMesh;
    ComponentMeshOBJ* pComponentMeshOBJ;
    ComponentAIChasePlayer* pComponentAIChasePlayer;
    ComponentCollisionObject* pComponentCollisionObject;

    m_pOBJTestFiles[0] = g_pFileManager->RequestFile( "Data/OBJs/cube.obj" );
    //m_pOBJTestFiles[1] = g_pFileManager->RequestFile( "Data/OBJs/Teapot.obj" );
    //m_pOBJTestFiles[2] = g_pFileManager->RequestFile( "Data/OBJs/humanoid_tri.obj" );
    //m_pOBJTestFiles[3] = g_pFileManager->RequestFile( "Data/OBJs/teapot2.obj" );

    //m_pTextures[0] = g_pTextureManager->CreateTexture( "Data/Textures/test1.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );
    //m_pTextures[1] = g_pTextureManager->CreateTexture( "Data/Textures/test2.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );
    //m_pTextures[2] = g_pTextureManager->CreateTexture( "Data/Textures/test3.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );
    //m_pTextures[3] = g_pTextureManager->CreateTexture( "Data/Textures/test4.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

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

#if MYFW_USING_WX
    // create a 3D X/Z plane grid
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false );
        pGameObject->SetName( "3D Grid Plane" );
        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = false; // manually drawn when in editor mode.
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_MainScene;
            pComponentMesh->m_pMesh->m_Tint.Set( 150, 150, 150, 255 );
            pComponentMesh->m_pMesh->CreateEditorLineGridXZ( Vector3(0,0,0), 1, 5 );
        }
        m_EditorState.m_p3DGridPlane = pGameObject;
    }

    // create a 3d transform widget for each axis.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false );
        pGameObject->SetName( "3D Transform Widget - x-axis" );
        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = false;
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_MainScene;
            pComponentMesh->m_pMesh->CreateEditorTransformWidgetAxis( 3, 0.1f, ColorByte(255, 100, 100, 255) );
        }
        pGameObject->m_pComponentTransform->SetRotation( Vector3( 0, 90, 0 ) );
        m_EditorState.m_pTransformWidgets[0] = pGameObject;
    }
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false );
        pGameObject->SetName( "3D Transform Widget - y-axis" );
        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = false;
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_MainScene;
            pComponentMesh->m_pMesh->CreateEditorTransformWidgetAxis( 3, 0.1f, ColorByte(100, 255, 100, 255) );
        }
        pGameObject->m_pComponentTransform->SetRotation( Vector3( -90, 0, 0 ) );
        m_EditorState.m_pTransformWidgets[1] = pGameObject;
    }
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject( false );
        pGameObject->SetName( "3D Transform Widget - z-axis" );
        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = false;
            pComponentMesh->SetShader( m_pShader_TintColor );
            pComponentMesh->m_LayersThisExistsOn = Layer_MainScene;
            pComponentMesh->m_pMesh->CreateEditorTransformWidgetAxis( 3, 0.1f, ColorByte(100, 100, 255, 255) );
        }
        pGameObject->m_pComponentTransform->SetRotation( Vector3( 0, 0, 0 ) );
        m_EditorState.m_pTransformWidgets[2] = pGameObject;
    }
#endif

    // create a 3D camera, renders first.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Main Camera" );
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 10 ) );
        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = false;
        pComponentCamera->m_LayersToRender = Layer_MainScene;
    }

    // create a 2D camera, renders after 3d, for hud.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Hud Camera" );
        pComponentCamera = (ComponentCamera*)pGameObject->AddNewComponent( ComponentType_Camera );
        pComponentCamera->SetDesiredAspectRatio( 640, 960 );
        pComponentCamera->m_Orthographic = true;
        pComponentCamera->m_LayersToRender = Layer_HUD;
    }

    // create a player game object and attach a mesh(sprite) component to it.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Player Object" );
        pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( ComponentType_Sprite );
        if( pComponentSprite )
        {
            pComponentSprite->SetShader( m_pShader_TintColor );
            pComponentSprite->m_Size.Set( 50.0f, 50.0f );
            pComponentSprite->m_Tint.Set( 255, 0, 0, 255 );
            pComponentSprite->m_LayersThisExistsOn = Layer_HUD;
        }
        pGameObject->AddNewComponent( ComponentType_InputTrackMousePos );
        pGameObject->m_pComponentTransform->SetPosition( Vector3( 0, 0, 0 ) );//m_GameWidth/2, m_GameHeight/2, 0 ) );

        pPlayer = pGameObject;
    }

    // create an enemy game object and attach a mesh(sprite) and AI component to it.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Second Object" );
        pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( ComponentType_Sprite );
        if( pComponentSprite )
        {
            pComponentSprite->SetShader( m_pShader_TintColor );
            pComponentSprite->m_Size.Set( 50.0f, 50.0f );
            pComponentSprite->m_Tint.Set( 0, 255, 0, 255 );
            pComponentSprite->m_LayersThisExistsOn = Layer_HUD;
        }
        pComponentAIChasePlayer = (ComponentAIChasePlayer*)pGameObject->AddNewComponent( ComponentType_AIChasePlayer );
        if( pComponentAIChasePlayer )
        {
            pComponentAIChasePlayer->m_pPlayerComponentTransform = pPlayer->m_pComponentTransform;
        }
    }

    // create a cube in the 3d scene.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Cube" );
        pComponentMeshOBJ = (ComponentMeshOBJ*)pGameObject->AddNewComponent( ComponentType_MeshOBJ );
        if( pComponentMeshOBJ )
        {
            pComponentMeshOBJ->SetShader( m_pShader_TestNormals );
            pComponentMeshOBJ->m_pOBJFile = m_pOBJTestFiles[0];
            pComponentMeshOBJ->m_LayersThisExistsOn = Layer_MainScene;
        }
        pComponentCollisionObject = (ComponentCollisionObject*)pGameObject->AddNewComponent( ComponentType_CollisionObject );
    }

    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );
}

static float totaltimepassed;

void GameEntityComponentTest::Tick(double TimePassed)
{
    GameCore::Tick( TimePassed );

    totaltimepassed += (float)TimePassed;

    // tick all components.
    if( m_EditorMode )
    {
        m_pComponentSystemManager->Tick( 0 );

        for( int i=0; i<3; i++ )
        {
            ComponentRenderable* pRenderable = (ComponentRenderable*)m_EditorState.m_pTransformWidgets[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

            if( m_EditorState.m_pSelectedGameObject )
            {
                //pRenderable->m_Visible = true;
                Vector3 pos = m_EditorState.m_pSelectedGameObject->m_pComponentTransform->GetPosition();
                m_EditorState.m_pTransformWidgets[i]->m_pComponentTransform->SetPosition( pos );
            }
            else
            {
                //pRenderable->m_Visible = false;
            }
        }
    }
    else
    {
        if( TimePassed != 0 )
            m_pComponentSystemManager->Tick( TimePassed );
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

    glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw the 3d grid.
    if( m_EditorMode )
    {
        // TODO: get the camera properly.
        ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_EditorState.m_p3DGridPlane->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        pRenderable->Draw( &pCamera->m_Camera3D.m_matViewProj );
    }

    // draw all components.
    m_pComponentSystemManager->OnDrawFrame();

    // draw the transform widget in the foreground
    if( m_EditorMode && m_EditorState.m_pSelectedGameObject )
    {
        glClear( GL_DEPTH_BUFFER_BIT );

        // TODO: get the camera properly.
        ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();

        for( int i=0; i<3; i++ )
        {
            ComponentRenderable* pRenderable = (ComponentRenderable*)m_EditorState.m_pTransformWidgets[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
            pRenderable->Draw( &pCamera->m_Camera3D.m_matViewProj );
        }
    }


    //ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    //MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();
    //m_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, m_pShader_MousePicker );
}

void GameEntityComponentTest::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    GameCore::OnTouch( action, id, x, y, pressure, size );

    // prefer 0,0 at bottom left.
    y = m_WindowHeight - y;

#if MYFW_USING_WX
    if( m_EditorMode )
    {
        HandleEditorInput( -1, -1, action, id, x, y, pressure );
        return;
    }
#endif

    // mouse moving without button down.
    if( id == -1 )
        return;

    // TODO: get the camera properly.
    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();

    // convert mouse to x/y in Camera2D space. TODO: put this in camera component.
    x = (x - pCamera->m_Camera2D.m_ScreenOffsetX - m_WindowStartX) / pCamera->m_Camera2D.m_ScreenWidth * m_GameWidth;
    y = (y - pCamera->m_Camera2D.m_ScreenOffsetY + m_WindowStartY) / pCamera->m_Camera2D.m_ScreenHeight * m_GameHeight;

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
            SaveScene();
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
            LoadScene();
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
    if( m_EditorMode )
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
            m_EditorState.m_pSelectedGameObject = GetObjectAtPixel( x, y );

            // select the object in the object tree.
            g_pPanelObjectList->SelectObject( m_EditorState.m_pSelectedGameObject );
        }

        if( action == GCBA_Down && id == 0 )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( x, y );

            bool selectedwidget = false;

            // translate to the right.
            if( pObject == m_EditorState.m_pTransformWidgets[0] )
            {
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_TranslateX;
                selectedwidget = true;
            }
            if( pObject == m_EditorState.m_pTransformWidgets[1] )
            {
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_TranslateY;
                selectedwidget = true;
            }
            if( pObject == m_EditorState.m_pTransformWidgets[2] )
            {
                m_EditorState.m_EditorActionState = EDITORACTIONSTATE_TranslateZ;
                selectedwidget = true;
            }

            // if shift is held, make a copy of the object and control that one.
            if( selectedwidget && m_EditorState.m_ModifierKeyStates & MODIFIERKEY_Shift )
            {
                m_EditorState.m_pSelectedGameObject = g_pComponentSystemManager->CopyGameObject( m_EditorState.m_pSelectedGameObject );
            }
        }

        if( m_EditorState.m_pSelectedGameObject )
        {
            // todo, rotate the mouse movement with the camera.
            Vector3 pos = m_EditorState.m_pSelectedGameObject->m_pComponentTransform->GetPosition();
            if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateX )
            {
                float diff = m_EditorState.m_CurrentMousePosition.x - m_EditorState.m_LastMousePosition.x;
                pos.x += diff * 0.05f;
                m_EditorState.m_pSelectedGameObject->m_pComponentTransform->SetPosition( pos );
            }
            if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateY )
            {
                float diff = m_EditorState.m_CurrentMousePosition.y - m_EditorState.m_LastMousePosition.y;
                pos.y += diff * 0.05f;
                m_EditorState.m_pSelectedGameObject->m_pComponentTransform->SetPosition( pos );
            }
            if( m_EditorState.m_EditorActionState == EDITORACTIONSTATE_TranslateZ )
            {
                float diff = m_EditorState.m_CurrentMousePosition.y - m_EditorState.m_LastMousePosition.y;
                pos.z -= diff * 0.05f;
                m_EditorState.m_pSelectedGameObject->m_pComponentTransform->SetPosition( pos );
            }
        }
    }

    // move camera in/out if mousewheel spinning
    if( action != -1 )
    {
        unsigned int mods = m_EditorState.m_ModifierKeyStates;

        // TODO: get the camera properly.
        ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

        if( action == GCBA_Wheel )
        {
            // pressure is also mouse wheel movement rate in WX.
            Vector3 dir = Vector3( 0, 0, 1 ) * -(pressure/abs(pressure));

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * 1.5f );
        }

        // if space is held, left button will pan the camera around.  or just middle button
        if( ( mods & MODIFIERKEY_LeftMouse && mods & MODIFIERKEY_Space ) ||
            mods & MODIFIERKEY_MiddleMouse
          )
        {
            Vector2 dir = m_EditorState.m_CurrentMousePosition - m_EditorState.m_LastMousePosition;

            if( dir.LengthSquared() > 0 )
                matLocalCamera->TranslatePreRotScale( dir * 0.05f );
        }
        else if( mods & MODIFIERKEY_LeftMouse && m_EditorState.m_EditorActionState == EDITORACTIONSTATE_None )
        { // if left mouse if down, rotate the camera.
            // rotate the camera around a point 10 units in front of the camera.
            Vector2 dir = m_EditorState.m_CurrentMousePosition - m_EditorState.m_LastMousePosition;

            float distancefromselectedobject = 10;
            if( m_EditorState.m_pSelectedGameObject )
            {
                distancefromselectedobject = 
                    ( m_EditorState.m_pSelectedGameObject->m_pComponentTransform->m_Transform.GetTranslation() -
                    pCamera->m_pComponentTransform->m_Transform.GetTranslation() ).Length();
            }

            if( dir.LengthSquared() > 0 )
            {
                matLocalCamera->TranslatePreRotScale( 0, 0, -distancefromselectedobject );

                matLocalCamera->Rotate( dir.y, 1, 0, 0 );
                matLocalCamera->Rotate( dir.x, 0, 1, 0 );

                matLocalCamera->TranslatePreRotScale( 0, 0, distancefromselectedobject );
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

void GameEntityComponentTest::SaveScene()
{
    char* savestring = g_pComponentSystemManager->SaveSceneToJSON();

    FILE* filehandle;
    errno_t error = fopen_s( &filehandle, "test.scene", "w" );
    if( filehandle )
    {
        fprintf( filehandle, "%s", savestring );
        fclose( filehandle );
    }

    free( savestring );
}

void GameEntityComponentTest::LoadScene()
{
    FILE* filehandle;
    errno_t err = fopen_s( &filehandle, "test.scene", "rb" );

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

        g_pComponentSystemManager->LoadSceneFromJSON( jsonstr );

        delete[] jsonstr;
    }

#if MYFW_USING_WX
    m_EditorMode = true;
#endif

    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );
}

void GameEntityComponentTest::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    GameCore::OnSurfaceChanged( startx, starty, width, height );

    if( height == 0 || width == 0 )
        return;

    float devicewidth = m_WindowWidth;
    float deviceheight = m_WindowHeight;
    float deviceratio = devicewidth / deviceheight;

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

    if( m_pComponentSystemManager )
        m_pComponentSystemManager->OnSurfaceChanged( startx, starty, width, height, (unsigned int)m_GameWidth, (unsigned int)m_GameHeight );

    if( m_EditorState.m_pMousePickerFBO )
    {
        if( m_EditorState.m_pMousePickerFBO->m_Width != width || m_EditorState.m_pMousePickerFBO->m_Height != height )
        {
            m_EditorState.m_pMousePickerFBO->Invalidate( true );
            m_EditorState.m_pMousePickerFBO->Setup( width, height, GL_NEAREST, GL_NEAREST, true, true, false );
            m_EditorState.m_pMousePickerFBO->Create();
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

    ComponentCamera* pCamera = m_pComponentSystemManager->GetFirstCamera();
    MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();
    m_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, m_pShader_MousePicker );

    // draw the transform widget object into the mouse picker buffer.
    // todo: clean this up, find better way to draw editor tools in fg/bg... additional layers/cameras
    if( m_EditorMode && m_EditorState.m_pSelectedGameObject )
    {
        Shader_Base* pShader = (Shader_Base*)m_pShader_MousePicker->GlobalPass();
        if( pShader->ActivateAndProgramShader() )
        {
            glClear( GL_DEPTH_BUFFER_BIT );

            for( int i=0; i<3; i++ )
            {
                ComponentRenderable* pRenderable = (ComponentRenderable*)m_EditorState.m_pTransformWidgets[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

                ColorByte tint( 0, 0, 0, 0 );

                unsigned int id = pRenderable->m_pGameObject->m_ID;

                if( 1 )                 tint.r = id%256;
                if( id > 256 )          tint.g = (id-256)%256;
                if( id > 256*256 )      tint.b = (id-256*256)%256;
                if( id > 256*256*256 )  tint.a = (id-256*256*256)%256;

                pShader->ProgramTint( tint );

                pRenderable->Draw( &pCamera->m_Camera3D.m_matViewProj, m_pShader_MousePicker );
            }

            pShader->DeactivateShader();
        }
    }

    // get a pixel from the FBO.
    unsigned char pixel[4];
    glReadPixels( x - m_WindowStartX, y - m_WindowStartY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel );

    unsigned int id = pixel[0] + pixel[1]*256 + pixel[2]*256*256 + pixel[3]*256*256*256;
    LOGInfo( LOGTag, "pixel - %d, %d, %d, %d - id - %d\n", pixel[0], pixel[1], pixel[2], pixel[3], id );

    m_EditorState.m_pMousePickerFBO->Unbind();

    // reset glViewport and scissor region.
    OnSurfaceChanged( (unsigned int)m_WindowStartX, (unsigned int)m_WindowStartY, (unsigned int)m_WindowWidth, (unsigned int)m_WindowHeight );

    GameObject* pGameObject = m_pComponentSystemManager->FindGameObjectByID( id );

    // if we didn't click on something, check if it's the transform widget.
    if( pGameObject == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            if( m_EditorState.m_pTransformWidgets[i]->m_ID == id )
            {
                return m_EditorState.m_pTransformWidgets[i];
            }
        }
    }
    return pGameObject;
}
