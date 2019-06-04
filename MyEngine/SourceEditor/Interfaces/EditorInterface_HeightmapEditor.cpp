//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorInterface_HeightmapEditor.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/EngineEditorCommands.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

EditorInterface_HeightmapEditor::EditorInterface_HeightmapEditor(EngineCore* pEngineCore)
: EditorInterface( pEngineCore )
{
    m_pHeightmap = nullptr;

    m_pPoint = nullptr;

    m_PositionMouseWentDown.Set( 0, 0 );

    m_IndexOfPointBeingDragged = -1;
    m_NewMousePress = false;

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        m_pMaterials[i] = nullptr;
    }
}

EditorInterface_HeightmapEditor::~EditorInterface_HeightmapEditor()
{
    SAFE_DELETE( m_pPoint );

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        SAFE_RELEASE( m_pMaterials[i] );
    }
}

void EditorInterface_HeightmapEditor::Initialize()
{
    MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();

    if( m_pMaterials[Mat_Lines] == nullptr )
        m_pMaterials[Mat_Lines] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColor(), ColorByte(255,0,0,255) );
    if( m_pMaterials[Mat_Points] == nullptr )
        m_pMaterials[Mat_Points] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColor(), ColorByte(255,255,0,255) );
    if( m_pMaterials[Mat_SelectedPoint] == nullptr )
        m_pMaterials[Mat_SelectedPoint] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColor(), ColorByte(255,255,255,255) );
}

void EditorInterface_HeightmapEditor::OnActivated()
{
    // Create a gameobject for the points that we'll draw.
    if( m_pPoint == nullptr )
    {
        GameObject* pGameObject;
        ComponentMesh* pComponentMesh;

        pGameObject = g_pComponentSystemManager->CreateGameObject( false, SCENEID_EngineObjects ); // Not managed.
        pGameObject->SetName( "Heightmap editor - point" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, SCENEID_EngineObjects, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterials[Mat_Points], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->Create2DCircle( 0.25f, 20 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;

            pComponentMesh->OnLoad();
        }
        
        m_pPoint = pGameObject;
    }
}

void EditorInterface_HeightmapEditor::OnDeactivated()
{
    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( false );
}

void EditorInterface_HeightmapEditor::OnDrawFrame(unsigned int canvasID)
{
    // EditorInterface class will draw the main editor view.
    EditorInterface::OnDrawFrame( canvasID );

    MyAssert( m_pHeightmap != nullptr );
    if( m_pHeightmap == nullptr )
        return;

    MyAssert( m_pPoint != nullptr );
    if( m_pPoint == nullptr )
        return;

    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( true );

    // TEST: Draw a circle at 0,0,0.
    {
        Vector3 worldPos( 0, 0, 0 );

        m_pPoint->GetTransform()->SetLocalPosition( worldPos );

        ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
        MyMatrix* pEditorMatProj = &pCamera->m_Camera3D.m_matProj;
        MyMatrix* pEditorMatView = &pCamera->m_Camera3D.m_matView;

        float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - worldPos).Length();
        m_pPoint->GetTransform()->SetLocalScale( Vector3( distance / 15.0f ) );

        g_pComponentSystemManager->DrawSingleObject( pEditorMatProj, pEditorMatView, m_pPoint, nullptr );
    }
}

void EditorInterface_HeightmapEditor::CancelCurrentOperation()
{
}

bool EditorInterface_HeightmapEditor::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    bool inputHandled = false;

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyAction == GCBA_Up && keyCode == MYKEYCODE_ESC )
    {
        //if( m_IndexOfPointBeingDragged != -1 )
        //    CancelCurrentOperation();
        //else
            g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    // TODO: Store last used vertex and handle delete key.
    if( keyAction == GCBA_Up && keyCode == MYKEYCODE_DELETE )
    {
    }

    EditorInterface::SetModifierKeyStates( keyAction, keyCode, mouseAction, id, x, y, pressure );

    if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( mouseAction == GCBA_Down )
        {
            if( id == 1 ) // Right mouse button to cancel current operation.
            {
                LOGDebug( "HeightmapEditor", "Cancelled operation on point %d\n", m_IndexOfPointBeingDragged );
                CancelCurrentOperation();
                inputHandled = true;
            }
        }

        if( mouseAction == GCBA_Down && id == 0 ) // Left mouse button down.
        {
            m_PositionMouseWentDown.Set( -1, -1 );
        }

        if( mouseAction == GCBA_Up && id == 0 ) // Left mouse button up.
        {
        }

        if( mouseAction == GCBA_Held && id == 0 ) //id & 1 << 0 ) // Left mouse button moved.
        {
            if( m_PositionMouseWentDown != Vector2( x, y ) )
            {
                m_PositionMouseWentDown.Set( x, y );

                Vector3 start, end;
                g_pEngineCore->GetMouseRay( Vector2( x, y ), &start, &end );
                Vector3 result;
                if( m_pHeightmap->RayCast( start, end, &result ) )
                {
                    LOGInfo( LOGTag, "RayCast result is (%0.2f, %0.2f, %0.2f)", result.x, result.y, result.z );
                }

                //Plane plane;
                //plane.Set( Vector3( 0, 1, 0 ), Vector3( 0, 0, 0 ) );
                //Vector3 intersectPoint;
                //plane.IntersectRay( start, end, &intersectPoint );

                //float height = -1.0f;
                //if( m_pHeightmap->GetHeightAtWorldXZ( intersectPoint.x, intersectPoint.z, &height ) )
                //{
                //    LOGInfo( LOGTag, "Height at (%0.2f, %0.2f) is %f", intersectPoint.x, intersectPoint.z, height );
                //}
            }
        }
    }

    // Handle camera movement, with both mouse and keyboard.
    if( inputHandled == false )
    {
        EditorInterface::HandleInputForEditorCamera( keyAction, keyCode, mouseAction, id, x, y, pressure );
    }

    return false;
}

void EditorInterface_HeightmapEditor::SetHeightmap(ComponentHeightmap* pHeightmap)
{
    m_pHeightmap = pHeightmap;

    if( m_pHeightmap )
    {
        // TODO: If no heightmap is created, then make a default flat one.
    }
}

MaterialDefinition* EditorInterface_HeightmapEditor::GetMaterial(MaterialTypes type)
{
    MyAssert( type >= 0 && type < Mat_NumMaterials );

    return m_pMaterials[type];
}
