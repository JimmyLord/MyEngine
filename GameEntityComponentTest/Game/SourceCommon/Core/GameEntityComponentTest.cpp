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

    m_pShader_TintColor = 0;
    m_pShader_TestNormals = 0;
}

GameEntityComponentTest::~GameEntityComponentTest()
{
    SAFE_DELETE( m_pShader_TintColor );
    SAFE_DELETE( m_pShader_TestNormals );

    SAFE_DELETE( m_pComponentSystemManager );

    delete m_pTestOBJMesh;
}

void GameEntityComponentTest::OneTimeInit()
{
    GameCore::OneTimeInit();

    GameObject* pPlayer = 0;
    GameObject* pGameObject;
    ComponentSprite* pComponentSprite;
    ComponentAIChasePlayer* pComponentAIChasePlayer;

    m_pTestOBJMesh = MyNew MyMesh;
    char* PlatformSpecific_LoadFile(const char* filename, int* length = 0, const char* file = __FILE__, unsigned long line = __LINE__);
    char* objbuffer = PlatformSpecific_LoadFile( "Data/OBJs/cube.obj" );
    //char* objbuffer = PlatformSpecific_LoadFile( "Data/OBJs/alfa147.obj" );
    //char* objbuffer = PlatformSpecific_LoadFile( "Data/OBJs/humanoid_tri.obj" );
    //char* objbuffer = PlatformSpecific_LoadFile( "Data/OBJs/Teapot2.obj" );
    m_pTestOBJMesh->CreateFromOBJBuffer( objbuffer );
    delete[] objbuffer;

    glEnable( GL_CULL_FACE );

    // setup our shaders
    m_pShader_TintColor = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Tint Color" );
    m_pShader_TestNormals = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Test-Normals" );
    m_pShader_TintColor->SetFileForAllPasses( "Data/Shaders/Shader_TintColor" );
    m_pShader_TestNormals->SetFileForAllPasses( "Data/Shaders/Shader_TestNormals" );

    // Initialize our component system.
    m_pComponentSystemManager = MyNew ComponentSystemManager( MyNew GameComponentTypeManager );

    // create a player game object and attach a mesh(sprite) component to it.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Player Object" );
        pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( ComponentType_Sprite );
        if( pComponentSprite )
        {
            pComponentSprite->SetShader( m_pShader_TintColor );
            pComponentSprite->m_Tint.Set( 255, 0, 0, 255 );
        }
        pGameObject->AddNewComponent( ComponentType_InputTrackMousePos );

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
            pComponentSprite->m_Tint.Set( 0, 0, 255, 255 );
        }
        pComponentAIChasePlayer = (ComponentAIChasePlayer*)pGameObject->AddNewComponent( ComponentType_AIChasePlayer );
        if( pComponentAIChasePlayer )
        {
            pComponentAIChasePlayer->m_pPlayerComponentTransform = pPlayer->m_pComponentTransform;
        }
    }
}

static float totaltimepassed;

void GameEntityComponentTest::Tick(double TimePassed)
{
    GameCore::Tick( TimePassed );

    totaltimepassed += (float)TimePassed * 180;

    // tick all components.
    m_pComponentSystemManager->Tick( TimePassed );
}

void GameEntityComponentTest::OnDrawFrame()
{
    GameCore::OnDrawFrame();

    glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw all components.
    m_pComponentSystemManager->OnDrawFrame();

    // draw obj mesh
    {
        MyMatrix matworld;
        matworld.CreateSRT( 1.0f, Vector3( totaltimepassed, totaltimepassed, 0 ), Vector3( 0, 0, -10 ) );

        MyMatrix matproj;
        matproj.CreatePerspective( 45, 640.0f/960.0f, 1, 10000 );
        //matproj.SetOrtho( -2, 2, -2, 2, -2, 2 );

        MyMatrix matfinal = matproj * matworld;

        m_pTestOBJMesh->SetShaderAndTexture( m_pShader_TestNormals, 0 );
        //m_pTestOBJMesh->SetShaderAndTexture( m_pShader_TintColor, 0 );
        m_pTestOBJMesh->Draw( &matfinal, 0, 0, 0, 0, 0, 0 );
    }
}

void GameEntityComponentTest::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    GameCore::OnTouch( action, id, x, y, pressure, size );

    // prefer 0,0 at bottom left.
    y = m_WindowHeight - y;

    m_pComponentSystemManager->OnTouch( action, id, x, y, pressure, size );
}

void GameEntityComponentTest::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    GameCore::OnButtons( action, id );
}
