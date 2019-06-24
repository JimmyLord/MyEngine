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
#include "GUI/EditorIcons.h"
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Commands/HeightmapEditorCommands.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

EditorInterface_HeightmapEditor::EditorInterface_HeightmapEditor(EngineCore* pEngineCore)
: EditorInterface( pEngineCore )
{
    m_pHeightmap = nullptr;

    m_CurrentTool = Tool::Raise;
    m_CurrentToolState = ToolState::Idle;

    m_pPoint = nullptr;

    m_PositionMouseWentDown.Set( 0, 0 );

    m_IndexOfPointBeingDragged = -1;
    m_NewMousePress = false;

    m_WorldSpaceMousePosition.Set( 0, 0, 0 );
    m_WorldSpaceMousePositionWhenToolStarted.Set( 0, 0, 0 );

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        m_pMaterials[i] = nullptr;
    }

    m_HeightmapNormalsNeedRebuilding = false;

    // Editor settings.
    m_BrushSoftness = 0.1f;
    m_RaiseAmount = 0.1f;
    m_RaiseRadius = 1.0f;
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
        m_pMaterials[Mat_Lines] = MyNew MaterialDefinition( pMaterialManager, m_pEngineCore->GetShader_TintColor(), ColorByte(255,0,0,255) );
    if( m_pMaterials[Mat_Points] == nullptr )
        m_pMaterials[Mat_Points] = MyNew MaterialDefinition( pMaterialManager, m_pEngineCore->GetShader_TintColor(), ColorByte(255,255,0,255) );
    if( m_pMaterials[Mat_SelectedPoint] == nullptr )
        m_pMaterials[Mat_SelectedPoint] = MyNew MaterialDefinition( pMaterialManager, m_pEngineCore->GetShader_TintColor(), ColorByte(255,255,255,255) );
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
        pGameObject->GetTransform()->SetLocalRotation( Vector3( -90, 0, 0 ) );

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

    // TEST: Draw a circle at the mouse position.
    {
        Vector3 worldPos = m_WorldSpaceMousePosition;

        m_pPoint->GetTransform()->SetLocalPosition( worldPos );
        //m_pPoint->GetTransform()->SetLocalRotation( Vector3( -90, 0, 0 ) );

        ComponentCamera* pCamera = m_pEngineCore->GetEditorState()->GetEditorCamera();
        MyMatrix* pEditorMatProj = &pCamera->m_Camera3D.m_matProj;
        MyMatrix* pEditorMatView = &pCamera->m_Camera3D.m_matView;

        //float distance = (pCamera->m_pComponentTransform->GetLocalPosition() - worldPos).Length();
        //m_pPoint->GetTransform()->SetLocalScale( Vector3( distance / 15.0f ) );

        g_pComponentSystemManager->DrawSingleObject( pEditorMatProj, pEditorMatView, m_pPoint, nullptr );
    }

    // Show some heightmap editor controls.
    ImGui::SetNextWindowSize( ImVec2(150,200), ImGuiSetCond_FirstUseEver );
    ImGui::Begin( "Heightmap Editor" );

    // Icon bar to select tools.
    {
        // TODO: Make new icons.
        ImVec2 normalSize = ImVec2( 20, 20 );
        ImVec2 selectedSize = ImVec2( 30, 30 );
        if( ImGuiExt::ButtonWithTooltip( EditorIcon_Prefab, m_CurrentTool == Tool::Raise ? selectedSize : normalSize, "Raise" ) )
        {
            m_CurrentTool = Tool::Raise;
        }
        ImGui::SameLine();
        if( ImGuiExt::ButtonWithTooltip( EditorIcon_Folder, m_CurrentTool == Tool::Lower ? selectedSize : normalSize, "Lower" ) )
        {
            m_CurrentTool = Tool::Lower;
        }
    }

    if( ImGui::CollapsingHeader( "Brush", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::DragFloat( "Softness", &m_BrushSoftness, 0.01f, 0.0f, 1.0f );
    }

    if( ImGui::CollapsingHeader( "Raise/Lower", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::DragFloat( "Amount", &m_RaiseAmount, 0.01f, 0.0f, FLT_MAX );
        ImGui::DragFloat( "Radius", &m_RaiseRadius, 0.01f, 0.0f, FLT_MAX );
    }

    if( ImGui::Button( "TODO: Save" ) )
    {
    }

    ImGui::End();
}

void EditorInterface_HeightmapEditor::CancelCurrentOperation()
{
    m_CurrentToolState = ToolState::Idle;
    g_pGameCore->GetCommandStack()->Undo( 1 );
}

bool EditorInterface_HeightmapEditor::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    bool inputHandled = false;

    EditorState* pEditorState = m_pEngineCore->GetEditorState();

    // Deal with keys.
    if( keyAction != -1 )
    {
        // Escape to cancel current tool or exit heightmap editor.
        if( keyAction == GCBA_Up && keyCode == MYKEYCODE_ESC )
        {
            if( m_CurrentToolState == ToolState::Active )
                CancelCurrentOperation();
            else
                m_pEngineCore->SetEditorInterface( EditorInterfaceType::SceneManagement );        
        }
    }

    // Deal with mouse.
    if( mouseAction != -1 )
    {
        // Find the mouse intersection point on the heightmap.
        Vector3 start, end;
        m_pEngineCore->GetMouseRay( Vector2( x, y ), &start, &end );

        Vector3 mouseIntersectionPoint;
        bool mouseRayIntersected = m_pHeightmap->RayCast( start, end, &mouseIntersectionPoint );

        // Show a 2d circle at that point. 
        m_WorldSpaceMousePosition = mouseIntersectionPoint;
        //LOGInfo( LOGTag, "RayCast result is (%0.2f, %0.2f, %0.2f)\n", mouseIntersectionPoint.x, mouseIntersectionPoint.y, mouseIntersectionPoint.z );

        EditorInterface::SetModifierKeyStates( keyAction, keyCode, mouseAction, id, x, y, pressure );

        if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
        {
            // Right mouse button to cancel current operation.
            if( mouseAction == GCBA_Down && id == 1 )
            {
                CancelCurrentOperation();
                inputHandled = true;
            }

            // Left mouse button down.
            if( mouseAction == GCBA_Down && id == 0 )
            {
                m_PositionMouseWentDown.Set( x, y );

                if( mouseRayIntersected )
                {
                    m_CurrentToolState = ToolState::Active;
                    m_WorldSpaceMousePositionWhenToolStarted = mouseIntersectionPoint;
                    ApplyCurrentTool( mouseIntersectionPoint, mouseAction );
                }
            }

            if( mouseAction == GCBA_Up && id == 0 ) // Left mouse button up.
            {
                // Rebuild heightmap normals when mouse is lifted after 'raise' command.
                if( m_HeightmapNormalsNeedRebuilding )
                {
                    m_CurrentToolState = ToolState::Idle;
                    m_HeightmapNormalsNeedRebuilding = false;
                    m_pHeightmap->GenerateHeightmapMesh( false, false, true );
                }
            }

            if( mouseAction == GCBA_Held && id == 0 ) // Left mouse button moved.
            {
                if( m_CurrentToolState == ToolState::Active )
                {
                    // Once the tool is active, intersect with the y=height plane of the initial ray.
                    // This will prevent the raise tool from moving around as the height changes.
                    Plane plane;
                    float heightWanted = m_WorldSpaceMousePositionWhenToolStarted.y;
                    plane.Set( Vector3( 0, 1, 0 ), Vector3( 0, heightWanted, 0 ) );

                    Vector3 flatIntersectPoint;
                    if( plane.IntersectRay( start, end, &flatIntersectPoint ) )
                    {
                        ApplyCurrentTool( flatIntersectPoint, mouseAction );                
                        m_WorldSpaceMousePosition = flatIntersectPoint;
                    }
                }
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

bool EditorInterface_HeightmapEditor::ExecuteHotkeyAction(HotKeyAction action)
{
    EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

#pragma warning( push )
#pragma warning( disable : 4062 )
    switch( action )
    {
    case HotKeyAction::HeightmapEditor_Tool_Raise:    { m_CurrentTool = Tool::Raise; return true; }
    case HotKeyAction::HeightmapEditor_Tool_Lower:    { m_CurrentTool = Tool::Lower; return true; }
    }
#pragma warning( pop )

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

void EditorInterface_HeightmapEditor::ApplyCurrentTool(Vector3 mouseIntersectionPoint, int mouseAction)
{
    if( m_CurrentToolState != ToolState::Active )
        return;

    switch( m_CurrentTool )
    {
    case Tool::Raise:
    case Tool::Lower:
        {
            float amount = m_RaiseAmount;
            if( m_CurrentTool == Tool::Lower )
                amount *= -1;

            // Raise the terrain and add to command stack.
            if( m_pHeightmap->Raise( mouseIntersectionPoint, amount, m_RaiseRadius, m_BrushSoftness, true ) )
            {
                EditorCommand_Heightmap_Raise* pCommand = MyNew EditorCommand_Heightmap_Raise( m_pHeightmap, mouseIntersectionPoint, amount, m_RaiseRadius, m_BrushSoftness );

                // Attach to previous 'raise' command if this is a held message,
                //   otherwise it's a mouse down so create a new message.
                bool attachToPrevious = mouseAction == GCBA_Held ? true : false;
                m_pEngineCore->GetCommandStack()->Add( pCommand, attachToPrevious );

                m_HeightmapNormalsNeedRebuilding = true;
            }
        }
        break;

    case Tool::None:
        break;
    }
}
