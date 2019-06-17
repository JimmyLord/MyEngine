//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorInterface_2DPointEditor.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

EditorInterface_2DPointEditor::EditorInterface_2DPointEditor(EngineCore* pEngineCore)
: EditorInterface( pEngineCore )
{
    m_pCollisionObject = nullptr;

    m_pPoint = nullptr;

    m_PositionMouseWentDown.SetZero();

    m_IndexOfPointBeingDragged = -1;
    m_NewMousePress = false;
    m_AddedVertexWhenMouseWasDragged = false;

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        m_pMaterials[i] = nullptr;
    }
}

EditorInterface_2DPointEditor::~EditorInterface_2DPointEditor()
{
    SAFE_DELETE( m_pPoint );

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        SAFE_RELEASE( m_pMaterials[i] );
    }
}

void EditorInterface_2DPointEditor::Initialize()
{
    MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();

    if( m_pMaterials[Mat_Lines] == nullptr )
        m_pMaterials[Mat_Lines] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColor(), ColorByte(255,0,0,255) );
    if( m_pMaterials[Mat_Points] == nullptr )
        m_pMaterials[Mat_Points] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColor(), ColorByte(255,255,0,255) );
    if( m_pMaterials[Mat_SelectedPoint] == nullptr )
        m_pMaterials[Mat_SelectedPoint] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColor(), ColorByte(255,255,255,255) );
}

void EditorInterface_2DPointEditor::OnActivated()
{
    // Create a gameobject for the points that we'll draw.
    if( m_pPoint == nullptr )
    {
        GameObject* pGameObject;
        ComponentMesh* pComponentMesh;

        pGameObject = g_pComponentSystemManager->CreateGameObject( false, SCENEID_EngineObjects ); // Not managed.
        pGameObject->SetName( "2D point editor - point" );

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

void EditorInterface_2DPointEditor::OnDeactivated()
{
    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( false );
}

void EditorInterface_2DPointEditor::OnDrawFrame(unsigned int canvasID)
{
    // EditorInterface class will draw the main editor view.
    EditorInterface::OnDrawFrame( canvasID );

    MyAssert( m_pCollisionObject != nullptr );
    if( m_pCollisionObject == nullptr )
        return;

    MyAssert( m_pPoint != nullptr );
    if( m_pPoint == nullptr )
        return;

    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    pRenderable->SetVisible( true );

    // Draw a circle at each vertex position. // Lines are drawn by m_pCollisionObject's render callback.
    for( unsigned int i=0; i<m_pCollisionObject->m_Vertices.size(); i++ )
    {
        b2Vec2 pos2d = m_pCollisionObject->m_Vertices[i];
        Vector3 pos3d( pos2d.x, pos2d.y, 0 );

        ComponentTransform* pParentTransformComponent = m_pCollisionObject->GetGameObject()->GetTransform();
        MyMatrix* pParentMatrix = pParentTransformComponent->GetWorldTransform();
        Vector3 worldPos = pParentMatrix->GetTranslation() + pos3d;

        m_pPoint->GetTransform()->SetLocalPosition( worldPos );

        ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
        MyMatrix* pEditorMatProj = &pCamera->m_Camera3D.m_matProj;
        MyMatrix* pEditorMatView = &pCamera->m_Camera3D.m_matView;

        float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - worldPos).Length();
        m_pPoint->GetTransform()->SetLocalScale( Vector3( distance / 15.0f ) );

        // Change the material color if this is the selected dot.
        if( i == (unsigned int)m_IndexOfPointBeingDragged )
            m_pPoint->SetMaterial( m_pMaterials[Mat_SelectedPoint] );

        g_pComponentSystemManager->DrawSingleObject( pEditorMatProj, pEditorMatView, m_pPoint, nullptr );

        if( i == (unsigned int)m_IndexOfPointBeingDragged )
            m_pPoint->SetMaterial( m_pMaterials[Mat_Points] );
    }

    // Draw Box2D debug data.
    if( g_pEngineCore->GetEditorPrefs()->Get_Debug_DrawPhysicsDebugShapes() && g_GLCanvasIDActive == 1 )
    {
        for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse && g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld )
                g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->m_pWorld->DrawDebugData();
        }
    }
}

void EditorInterface_2DPointEditor::CancelCurrentOperation()
{
    if( m_IndexOfPointBeingDragged != -1 )
    {
        if( m_AddedVertexWhenMouseWasDragged )
        {
            g_pGameCore->GetCommandStack()->Undo( 1 );
        }
        else
        {
            m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged] = m_PositionMouseWentDown;
        }
        m_IndexOfPointBeingDragged = -1;
    }
}

bool EditorInterface_2DPointEditor::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    bool inputHandled = false;

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyAction == GCBA_Up && keyCode == MYKEYCODE_ESC )
    {
        if( m_IndexOfPointBeingDragged != -1 )
            CancelCurrentOperation();
        else
            g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    // TODO: Store last used vertex and handle delete key.
    if( keyAction == GCBA_Up && keyCode == MYKEYCODE_DELETE )
    {
        if( m_IndexOfPointBeingDragged != -1 && m_pCollisionObject->m_Vertices.size() > 2 )
        {
            // If the mouse is still down, undo will use the position of the vertex when the mouse went down,
            //     otherwise it will use the saved position of the vertex.
            b2Vec2 position = m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged];
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
                position = m_PositionMouseWentDown;

            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_Delete2DPoint( m_pCollisionObject, m_IndexOfPointBeingDragged, position ) );
            m_IndexOfPointBeingDragged = -1;
        }
    }

    EditorInterface::SetModifierKeyStates( keyAction, keyCode, mouseAction, id, x, y, pressure );

    if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( mouseAction == GCBA_Down )
        {
            if( id == 1 ) // Right mouse button to cancel current operation.
            {
                LOGDebug( "2DPointEditor", "Cancelled operation on point %d\n", m_IndexOfPointBeingDragged );
                CancelCurrentOperation();
                inputHandled = true;
            }
        }

        if( mouseAction == GCBA_Down && id == 0 ) // Left mouse button down.
        {
            m_NewMousePress = true;

            // Find the object we clicked on.
            unsigned int pixelID = GetIDAtPixel( (unsigned int)x, (unsigned int)y, true, true );

            m_IndexOfPointBeingDragged = pixelID - 1;
            m_AddedVertexWhenMouseWasDragged = false;

            // Reset mouse movement, so we can undo to this state after mouse goes up.
            if( m_IndexOfPointBeingDragged != -1 )
            {
                m_PositionMouseWentDown = m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged];
            }

            LOGDebug( "2DPointEditor", "Grabbed point %d\n", m_IndexOfPointBeingDragged );
        }

        if( mouseAction == GCBA_Up && id == 0 ) // Left mouse button up.
        {
            if( m_IndexOfPointBeingDragged != -1 )
            {
                LOGDebug( "2DPointEditor", "Released point %d\n", m_IndexOfPointBeingDragged );

                // Add command to stack for undo/redo.                    
                b2Vec2 distMoved = m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged] - m_PositionMouseWentDown;
                if( distMoved.LengthSquared() != 0 )
                {
                    g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_Move2DPoint( distMoved, m_pCollisionObject, m_IndexOfPointBeingDragged ), m_AddedVertexWhenMouseWasDragged );
                }

                m_IndexOfPointBeingDragged = -1;
            }
        }

        if( mouseAction == GCBA_Held && id == 0 ) //id & 1 << 0 ) // Left mouse button moved.
        {
            bool createNewVertex = false;
            if( m_NewMousePress && pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            {
                createNewVertex = true;
            }

            m_NewMousePress = false;

            if( m_IndexOfPointBeingDragged != -1 )
            {
                //LOGDebug( "2DPointEditor", "Moved point %d\n", m_IndexOfPointBeingDragged );

                // Create a plane based on the axis we want.
                Vector3 normal = Vector3(0,0,1);
                Vector3 axisVector = Vector3(1,0,0);

                ComponentTransform* pParentTransformComponent = m_pCollisionObject->GetGameObject()->GetTransform();
                MyMatrix* pParentMatrix = pParentTransformComponent->GetWorldTransform();
                Vector3 worldPos = pParentMatrix->GetTranslation();

                // Create a plane on Z = worldPos.z // TODO: If the Box2D world is on a rotated plane, fix this.
                Plane plane;
                plane.Set( normal, Vector3( 0, 0, worldPos.z ) );

                // Get the mouse click ray... current and last frame.
                Vector3 currentRayStart, currentRayEnd;
                g_pEngineCore->GetMouseRay( pEditorState->m_CurrentMousePosition, &currentRayStart, &currentRayEnd );

                Vector3 lastRayStart, lastRayEnd;
                g_pEngineCore->GetMouseRay( pEditorState->m_LastMousePosition, &lastRayStart, &lastRayEnd );

                //LOGDebug( "2DPointEditor", "current->(%0.0f,%0.0f) (%0.2f,%0.2f,%0.2f) (%0.2f,%0.2f,%0.2f)\n",
                //        pEditorState->m_CurrentMousePosition.x,
                //        pEditorState->m_CurrentMousePosition.y,
                //        currentRayStart.x,
                //        currentRayStart.y,
                //        currentRayStart.z,
                //        currentRayEnd.x,
                //        currentRayEnd.y,
                //        currentRayEnd.z
                //    );

                // Find the intersection point of the plane.
                Vector3 currentResult;
                Vector3 lastResult;
                if( plane.IntersectRay( currentRayStart, currentRayEnd, &currentResult ) &&
                    plane.IntersectRay( lastRayStart, lastRayEnd, &lastResult ) )
                {
                    //LOGDebug( "2DPointEditor", "currentResult( %f, %f, %f );", currentResult.x, currentResult.y, currentResult.z );
                    //LOGDebug( "2DPointEditor", "lastResult( %f, %f, %f );", lastResult.x, lastResult.y, lastResult.z );
                    //LOGDebug( "2DPointEditor", "axisVector( %f, %f, %f );\n", axisVector.x, axisVector.y, axisVector.z );

                    ComponentTransform* pParentTransformComponent = m_pCollisionObject->GetGameObject()->GetTransform();
                    MyMatrix* pParentMatrix = pParentTransformComponent->GetLocalTransform();

                    b2Vec2 newPos;
                    newPos.x = currentResult.x - pParentMatrix->GetTranslation().x;
                    newPos.y = currentResult.y - pParentMatrix->GetTranslation().y;

                    if( g_pEngineCore->GetEditorPrefs()->Get_Grid_SnapEnabled() )
                    {
                        // Snap point to grid.
                        newPos.x = MyRoundToMultipleOf( newPos.x, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepSize.x );
                        newPos.y = MyRoundToMultipleOf( newPos.y, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepSize.y );
                    }

                    if( createNewVertex )
                    {
                        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_Insert2DPoint( m_pCollisionObject, m_IndexOfPointBeingDragged ) );
                        m_AddedVertexWhenMouseWasDragged = true;
                    }
                    else
                    {
                        m_pCollisionObject->m_Vertices[m_IndexOfPointBeingDragged].Set( newPos.x, newPos.y );
                    }
                }
            }

            //m_IndexOfPointBeingDragged = -1;
        }
    }

    // Handle camera movement, with both mouse and keyboard.
    if( inputHandled == false )
    {
        EditorInterface::HandleInputForEditorCamera( keyAction, keyCode, mouseAction, id, x, y, pressure );
    }

    return false;
}

void EditorInterface_2DPointEditor::Set2DCollisionObjectToEdit(Component2DCollisionObject* pCollisionObject)
{
    m_pCollisionObject = pCollisionObject;

    if( m_pCollisionObject->m_Vertices.size() == 0 )
    {
        m_pCollisionObject->m_Vertices.push_back( b2Vec2(0,0) );
    }
}

void EditorInterface_2DPointEditor::RenderObjectIDsToFBO()
{
    // Draw a circle for each 2D point, so the mouse can select them.

    // Don't call the base class, which would draw all scene objects.
    //EditorInterface::RenderObjectIDsToFBO();

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return;

    // Bind our FBO so we can render stuff to it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    MyViewport viewport( 0, 0, pEditorState->m_pMousePickerFBO->GetWidth(), pEditorState->m_pMousePickerFBO->GetHeight() );
    g_pRenderer->EnableViewport( &viewport, true );

    g_pRenderer->SetClearColor( ColorFloat( 0, 0, 0, 0 ) );
    g_pRenderer->ClearBuffers( true, true, false );

    // Draw a circle at each vertex position.
    {
        ShaderGroup* pShaderOverride = g_pEngineCore->GetShader_TintColor();
        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    
        if( pShader->Activate() )
        {
            ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
            MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

            for( unsigned int i=0; i<m_pCollisionObject->m_Vertices.size(); i++ )
            {
                b2Vec2 pos2d = m_pCollisionObject->m_Vertices[i];
                Vector3 pos3d( pos2d.x, pos2d.y, 0 );

                ComponentTransform* pParentTransformComponent = m_pCollisionObject->GetGameObject()->GetTransform();
                MyMatrix* pParentMatrix = pParentTransformComponent->GetLocalTransform();
                Vector3 worldPos = pParentMatrix->GetTranslation() + pos3d;

                m_pPoint->GetTransform()->SetLocalPosition( worldPos );

                ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
                MyMatrix* pEditorMatProj = &pCamera->m_Camera3D.m_matProj;
                MyMatrix* pEditorMatView = &pCamera->m_Camera3D.m_matView;

                float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - pos3d).Length();
                m_pPoint->GetTransform()->SetLocalScale( Vector3( distance / 15.0f ) );

                ColorByte tint( 0, 0, 0, 0 );
                    
                unsigned int id = UINT_MAX - (i+1) * 641; // 1, 641, 6700417, 4294967297, 

                if( 1 )                 tint.r = id%256;
                if( id > 256 )          tint.g = (id>>8)%256;
                if( id > 256*256 )      tint.b = (id>>16)%256;
                if( id > 256*256*256 )  tint.a = (id>>24)%256;

                pShader->ProgramTint( tint );

                g_pComponentSystemManager->DrawSingleObject( pEditorMatProj, pEditorMatView, m_pPoint, pShaderOverride );
            }
        }
    }

    pEditorState->m_pMousePickerFBO->Unbind( true );
}

MaterialDefinition* EditorInterface_2DPointEditor::GetMaterial(MaterialTypes type)
{
    MyAssert( type >= 0 && type < Mat_NumMaterials );

    return m_pMaterials[type];
}
