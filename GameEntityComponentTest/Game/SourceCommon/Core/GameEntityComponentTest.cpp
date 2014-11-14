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

    m_pShader_White = 0;
}

GameEntityComponentTest::~GameEntityComponentTest()
{
    SAFE_DELETE( m_pShader_White );

    SAFE_DELETE( m_pComponentSystemManager );
}

void GameEntityComponentTest::OneTimeInit()
{
    GameCore::OneTimeInit();

    GameObject* pPlayer = 0;
    GameObject* pGameObject;
    ComponentSprite* pComponentSprite;
    ComponentAIChasePlayer* pComponentAIChasePlayer;

    // setup our shader.
    m_pShader_White = MyNew ShaderGroup( MyNew Shader_Base(ShaderPass_Main), 0, 0, "Vertex Color" );
    m_pShader_White->SetFileForAllPasses( "Data/Shaders/Shader_White" );

    // Initialize our component system.
    m_pComponentSystemManager = MyNew ComponentSystemManager( MyNew GameComponentTypeManager );

    // create a player game object and attach a mesh(sprite) component to it.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Player Object" );
        pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( MyNew ComponentSprite() );
        if( pComponentSprite )
        {
            pComponentSprite->SetShader( m_pShader_White );
            pComponentSprite->m_Tint.Set( 255, 0, 0, 255 );
        }
        pGameObject->AddNewComponent( MyNew ComponentInputTrackMousePos() );

        pPlayer = pGameObject;
    }

    // create an enemy game object and attach a mesh(sprite) and AI component to it.
    {
        pGameObject = m_pComponentSystemManager->CreateGameObject();
        pGameObject->SetName( "Second Object" );
        pComponentSprite = (ComponentSprite*)pGameObject->AddNewComponent( MyNew ComponentSprite() );
        if( pComponentSprite )
        {
            pComponentSprite->SetShader( m_pShader_White );
            pComponentSprite->m_Tint.Set( 0, 0, 255, 255 );
        }
        pComponentAIChasePlayer = (ComponentAIChasePlayer*)pGameObject->AddNewComponent( MyNew ComponentAIChasePlayer() );
        if( pComponentAIChasePlayer )
        {
            pComponentAIChasePlayer->m_pPlayerComponentTransform = pPlayer->m_pComponentTransform;
        }
    }
}

void GameEntityComponentTest::Tick(double TimePassed)
{
    GameCore::Tick( TimePassed );

    // tick all components.
    m_pComponentSystemManager->Tick( TimePassed );
}

void GameEntityComponentTest::OnDrawFrame()
{
    GameCore::OnDrawFrame();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw all components.
    m_pComponentSystemManager->OnDrawFrame();
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
