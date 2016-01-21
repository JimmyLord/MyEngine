//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EditorInterface_2DPointEditor::EditorInterface_2DPointEditor()
{
    m_pCollisionObject = 0;

    m_pPoint = 0;
}

EditorInterface_2DPointEditor::~EditorInterface_2DPointEditor()
{
    SAFE_DELETE( m_pPoint );
}

void EditorInterface_2DPointEditor::OnActivated()
{
}

void EditorInterface_2DPointEditor::OnDeactivated()
{
    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( false );
}

void EditorInterface_2DPointEditor::OnDrawFrame(unsigned int canvasid)
{
    // EditorInterface class will draw the main editor view
    EditorInterface::OnDrawFrame( canvasid );

    MyAssert( m_pCollisionObject != 0 );
    if( m_pCollisionObject == 0 )
        return;

    // create a gameobject for the points that we'll draw.
    if( m_pPoint == 0 )
    {
        GameObject* pGameObject;
        ComponentMesh* pComponentMesh;

        pGameObject = g_pComponentSystemManager->CreateGameObject( false, EngineCore::ENGINE_SCENE_ID ); // not managed.
        pGameObject->SetName( "2D point editor - point" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, EngineCore::ENGINE_SCENE_ID );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( g_pEngineCore->m_pMaterial_TransformGizmoY, 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->Create2DCircle( 0.25f, 20 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
        }

        m_pPoint = pGameObject;
    }

    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( true );

    // Draw a circle at each vertex position.
    for( unsigned int i=0; i<m_pCollisionObject->m_Vertices.size(); i++ )
    {
        b2Vec2 pos2d = m_pCollisionObject->m_Vertices[i];
        Vector3 pos3d( pos2d.x, pos2d.y, 0 );

        ComponentTransform* pParentTransformComponent = m_pCollisionObject->m_pGameObject->GetTransform();
        MyMatrix* pParentMatrix = pParentTransformComponent->GetTransform();
        Vector3 worldpos = pParentMatrix->GetTranslation() + pos3d;

        m_pPoint->m_pComponentTransform->SetPosition( worldpos );

        ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();
        MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

        g_pComponentSystemManager->DrawSingleObject( pEditorMatViewProj, m_pPoint );
    }
}

bool EditorInterface_2DPointEditor::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_ESC )
    {
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

#if MYFW_USING_WX
    EditorInterface::SetModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    // handle camera movement, with both mouse and keyboard.
    EditorInterface::HandleInputForEditorCamera( keyaction, keycode, mouseaction, id, x, y, pressure );

    // clear mouse button states.
    EditorInterface::ClearModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );
#endif //MYFW_USING_WX

    return false;
}

void EditorInterface_2DPointEditor::Set2DCollisionObjectToEdit(Component2DCollisionObject* pCollisionObject)
{
    m_pCollisionObject = pCollisionObject;
}
