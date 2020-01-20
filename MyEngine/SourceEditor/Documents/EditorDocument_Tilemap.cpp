//
// Copyright (c) 2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorDocument_Tilemap.h"
//#include "Camera/Camera3D.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
//#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "GUI/EditorIcons.h"
#include "GUI/ImGuiExtensions.h"
//#include "../SourceEditor/EditorState.h"
//#include "../SourceEditor/Commands/EngineEditorCommands.h"
//#include "../SourceEditor/Commands/HeightmapEditorCommands.h"
//#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
//#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"
//#include "../SourceEditor/Prefs/EditorPrefs.h"
//#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

EditorDocument_Tilemap::EditorDocument_Tilemap(EngineCore* pEngineCore)//, ComponentHeightmap* pHeightmap)
: EditorDocument( pEngineCore )
{
    m_pCamera = MyNew ComponentCamera( pEngineCore, nullptr );
    m_pCamera->Reset();
    m_pCameraTransform = MyNew ComponentTransform( pEngineCore, nullptr );
    m_pCameraTransform->Reset();
    m_pCamera->m_pComponentTransform = m_pCameraTransform;

    m_pCameraTransform->SetWorldPosition( Vector3( -1, 5, -5 ) );
    m_pCameraTransform->SetWorldRotation( Vector3( -30, -30, 0 ) );

    m_pFBO = nullptr;
    m_WindowPos.Set( -1, -1 );
    m_WindowSize.Set( 0, 0 );
    m_WindowHovered = false;
    m_WindowFocused = false;
    m_WindowVisible = false;

    //if( pHeightmap )
    //{
    //    m_HeightmapOwnedByUs = false;

    //    m_pHeightmap = pHeightmap;
    //}
    //else
    //{
    //    m_HeightmapOwnedByUs = true;

    //    m_pHeightmap = MyNew ComponentHeightmap( pEngineCore, nullptr );
    //    m_pHeightmap->Reset();
    //    m_pHeightmap->m_Size.Set( 5, 5 );
    //    m_pHeightmap->m_VertCount.Set( 128, 128 );
    //    MaterialDefinition* pMaterial = pEngineCore->GetManagers()->GetMaterialManager()->GetFirstMaterial();
    //    m_pHeightmap->SetMaterial( pMaterial, 0 );
    //    m_pHeightmap->CreateHeightmap();
    //    m_pHeightmap->RegisterCallbacks();
    //}

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

    // Warnings.
    m_ShowWarning_CloseEditor = false;

    // Editor settings.
    m_TileTypeSelected = 0;

    Initialize();
}

EditorDocument_Tilemap::~EditorDocument_Tilemap()
{
    //if( m_HeightmapOwnedByUs )
    //{
    //    m_pHeightmap->SetEnabled( false );
    //    delete m_pHeightmap;
    //}

    SAFE_DELETE( m_pPoint );

    for( int i=0; i<Mat_NumMaterials; i++ )
    {
        SAFE_RELEASE( m_pMaterials[i] );
    }

    SAFE_RELEASE( m_pFBO );

    m_pCameraTransform->SetEnabled( false );
    m_pCamera->SetEnabled( false );
    SAFE_DELETE( m_pCameraTransform );
    SAFE_DELETE( m_pCamera );
}

void EditorDocument_Tilemap::Initialize()
{
    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;
    MaterialManager* pMaterialManager = pEngineCore->GetManagers()->GetMaterialManager();

    if( m_pMaterials[Mat_Point] == nullptr )
        m_pMaterials[Mat_Point] = MyNew MaterialDefinition( pMaterialManager, pEngineCore->GetShader_TintColor(), ColorByte(255,255,0,255) );
    if( m_pMaterials[Mat_BrushOverlay] == nullptr )
        m_pMaterials[Mat_BrushOverlay] = pMaterialManager->LoadMaterial( "Data/DataEngine/Materials/HeightmapBrush.mymaterial" );

    // Create a gameobject for the point that we'll draw.
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
            pComponentMesh->SetMaterial( m_pMaterials[Mat_Point], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->Create2DCircle( 0.25f, 20 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;

            pComponentMesh->OnLoad();
        }

        m_pPoint = pGameObject;
    }
}

bool EditorDocument_Tilemap::IsBusy()
{
    return false;
}

void EditorDocument_Tilemap::OnDrawFrame() //unsigned int canvasID)
{
}

void EditorDocument_Tilemap::AddImGuiOverlayItems()
{
}

void EditorDocument_Tilemap::CancelCurrentOperation()
{
    m_CurrentToolState = ToolState::Idle;
    m_pCommandStack->Undo( 1 );
}

void EditorDocument_Tilemap::GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end)
{
    // Convert mouse coord into clip space.
    Vector2 mouseClip;
    mouseClip.x = (mousepos.x / m_WindowSize.x) * 2.0f - 1.0f;
    mouseClip.y = (mousepos.y / m_WindowSize.y) * 2.0f - 1.0f;

    // Compute the inverse view projection matrix.
    MyMatrix invVP = ( m_pCamera->m_Camera3D.m_matProj * m_pCamera->m_Camera3D.m_matView ).GetInverse();

    // Store the camera position as the near world point.
    Vector3 nearWorldPoint = m_pCameraTransform->GetWorldPosition();

    // Calculate the world position of the far clip plane where the mouse is pointing.
    Vector4 farClipPoint4 = Vector4( mouseClip, 1, 1 );
    Vector4 farWorldPoint4 = invVP * farClipPoint4;
    Vector3 farWorldPoint = farWorldPoint4.XYZ() / farWorldPoint4.w;

    *start = nearWorldPoint;
    *end = farWorldPoint;
}

bool EditorDocument_Tilemap::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    // Base document class handled undo/redo/save.
    bool inputHandled = EditorDocument::HandleInput( keyAction, keyCode, mouseAction, id, x, y, pressure );
    if( inputHandled )
        return true;

    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;
    EditorState* pEditorState = pEngineCore->GetEditorState();

    // Deal with keys.
    if( keyAction != -1 )
    {
        // Escape to cancel current tool or exit heightmap editor.
        if( keyAction == GCBA_Up && keyCode == MYKEYCODE_ESC )
        {
            if( m_CurrentToolState == ToolState::Active )
            {
                CancelCurrentOperation();
            }
        }
    }

    // Deal with mouse.
    if( mouseAction != -1 )
    {
    }

    // Handle camera movement, with both mouse and keyboard.
    if( inputHandled == false )
    {
        if( m_pCamera->HandleInputForEditorCamera( keyAction, keyCode, mouseAction, id, x, y, pressure ) )
        {
            return true;
        }
    }

    return inputHandled;
}

bool EditorDocument_Tilemap::ExecuteHotkeyAction(HotkeyAction action)
{
    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;
    EditorPrefs* pEditorPrefs = pEngineCore->GetEditorPrefs();

#pragma warning( push )
#pragma warning( disable : 4062 )
    switch( action )
    {
    case HotkeyAction::HeightmapEditor_Tool_Raise:    { m_CurrentTool = Tool::Raise; return true; }
    case HotkeyAction::HeightmapEditor_Tool_Lower:    { m_CurrentTool = Tool::Lower; return true; }
    case HotkeyAction::HeightmapEditor_Tool_Level:    { m_CurrentTool = Tool::Level; return true; }
    }
#pragma warning( pop )

    return false;
}

void EditorDocument_Tilemap::Update()
{
    EditorDocument::Update();

    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;

    m_WindowVisible = true;

    ImVec2 min = ImGui::GetWindowContentRegionMin();
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    float w = max.x - min.x;
    float h = max.y - min.y;

    ImVec2 pos = ImGui::GetWindowPos();
    m_WindowPos.Set( pos.x + min.x, pos.y + min.y );
    m_WindowSize.Set( w, h );

    m_pCamera->m_Camera3D.SetupProjection( w/h, w/h, 45, 0.01f, 100.0f );
    m_pCamera->m_Camera3D.UpdateMatrices();

    //ImGui::Text( "Testing Heightmap EditorDocument." );

    if( m_pFBO )
    {
        // This will resize our FBO if the window is larger than it ever was.
        pEngineCore->GetManagers()->GetTextureManager()->ReSetupFBO( m_pFBO, (unsigned int)w, (unsigned int)h, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, true );

        if( m_pFBO->GetColorTexture( 0 ) )
        {
            TextureDefinition* tex = m_pFBO->GetColorTexture( 0 );
            ImGui::ImageButton( (void*)tex, ImVec2( w, h ), ImVec2(0,h/m_pFBO->GetTextureHeight()), ImVec2(w/m_pFBO->GetTextureWidth(),0), 0 );
        }
    }

    // Show some heightmap editor controls.
    ImGui::SetNextWindowSize( ImVec2(150,200), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowBgAlpha( 1.0f );
    ImGui::Begin( "Heightmap Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing );

    // Icon bar to select tools.
    {
        // TODO: Make new icons.
        ImVec2 normalSize = ImVec2( 20, 20 );
        ImVec2 selectedSize = ImVec2( 30, 30 );
        if( ImGuiExt::ButtonWithTooltip( EditorIconString_Prefab, m_CurrentTool == Tool::Raise ? selectedSize : normalSize, "Raise" ) )
        {
            m_CurrentTool = Tool::Raise;
        }
        ImGui::SameLine();
        if( ImGuiExt::ButtonWithTooltip( EditorIconString_Folder, m_CurrentTool == Tool::Lower ? selectedSize : normalSize, "Lower" ) )
        {
            m_CurrentTool = Tool::Lower;
        }
        ImGui::SameLine();
        if( ImGuiExt::ButtonWithTooltip( EditorIconString_GameObject, m_CurrentTool == Tool::Level ? selectedSize : normalSize, "Level" ) )
        {
            m_CurrentTool = Tool::Level;
        }
    }

    // General options.
    if( ImGui::CollapsingHeader( "General", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        //ImGui::Checkbox( "Always recalculate normals", &m_AlwaysRecalculateNormals );
    }

    // Tool specific values.
    if( ImGui::CollapsingHeader( "Brush", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        //ImGui::DragFloat( "Softness", &m_BrushSoftness, 0.01f, 0.0f, 1.0f );
        //ImGui::DragFloat( "Radius", &m_BrushRadius, 0.01f, 0.0f, FLT_MAX );
    }

    if( ImGui::CollapsingHeader( "Raise/Lower", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        //ImGui::DragFloat( "Amount", &m_RaiseAmount, 0.01f, 0.0f, FLT_MAX );
    }

    if( ImGui::CollapsingHeader( "Level", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        //ImGui::Checkbox( "Use brush height", &m_LevelUseBrushHeight );
        //ImGui::DragFloat( "Height", &m_LevelHeight, 0.01f, 0.0f, FLT_MAX );
    }

    // Other.
    ImGui::Separator();

    if( ImGui::Button( "Export as MyMesh" ) )
    {
        //m_pHeightmap->SaveAsMyMesh( "Data/Meshes/TestHeightmap.mymesh" );
    }

    if( ImGui::Button( "Save" ) )
    {
        Save();
    }

    ImGui::End();

    if( m_ShowWarning_CloseEditor )
    {
        m_ShowWarning_CloseEditor = false;
        ImGui::OpenPopup( "Close Heightmap Editor Warning" );
    }
    if( ImGui::BeginPopupModal( "Close Heightmap Editor Warning" ) )
    {
        ImGui::Text( "Some changes aren't saved." );
        ImGui::Dummy( ImVec2( 0, 10 ) );

        if( ImGui::Button( "Revert changes" ) )
        {
            while( m_pCommandStack->GetUndoStackSize() > 0 )
                m_pCommandStack->Undo( 1 );
            ImGui::CloseCurrentPopup();
        }

        if( ImGui::Button( "Save" ) )
        {
            ImGui::CloseCurrentPopup();
            Save();
        }

        if( ImGui::Button( "Keep changes without saving" ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::SetCursorPos( ImVec2( 8, 28 ) );
    AddImGuiOverlayItems();
}

//void EditorDocument_Tilemap::SetHeightmap(ComponentHeightmap* pHeightmap)
//{
//    m_pHeightmap = pHeightmap;
//
//    if( m_pHeightmap )
//    {
//        // TODO: If no heightmap is created, then make a default flat one.
//    }
//}

//MaterialDefinition* EditorDocument_Tilemap::GetMaterial(MaterialTypes type)
//{
//    MyAssert( type >= 0 && type < Mat_NumMaterials );
//
//    return m_pMaterials[type];
//}

void EditorDocument_Tilemap::ApplyCurrentTool(Vector3 localSpacePoint, int mouseAction)
{
    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;

    if( m_CurrentToolState != ToolState::Active )
        return;

    bool valuesChanged = false;

    switch( m_CurrentTool )
    {
    case Tool::Raise:
    case Tool::Lower:
        {
        }
        break;

    case Tool::Level:
        {
        }
        break;

    case Tool::None:
        break;
    }

    if( valuesChanged )
    {
    }
}

//====================================================================================================
// Protected Methods.
//====================================================================================================
void EditorDocument_Tilemap::Save()
{
    EditorDocument::Save();

    // Save NodeGraph as JSON string.
    {
        const char* filename = GetRelativePath();
        if( filename[0] == '\0' )
        {
            return;
        }

        //m_pHeightmap->SaveAsHeightmap( filename );
        //MyFileInfo* pFileInfo = m_pEngineCore->GetComponentSystemManager()->LoadDataFile( filename, SceneID::SCENEID_MainScene, nullptr, false );
        //m_pHeightmap->SetHeightmapFile( pFileInfo->GetFile() );
    }
}

void EditorDocument_Tilemap::Load()
{
    EditorDocument::Load();

    const char* filename = GetRelativePath();
    if( filename[0] == '\0' )
    {
        return;
    }

    //m_pHeightmap->LoadFromHeightmap( filename );
    //m_pHeightmap->GenerateHeightmapMesh( false, true, true );
}