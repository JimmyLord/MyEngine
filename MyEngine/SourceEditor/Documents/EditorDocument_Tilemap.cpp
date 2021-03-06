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
#include "ComponentSystem/EngineComponents/ComponentTilemap.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "GUI/EditorIcons.h"
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/EditorState.h"
//#include "../SourceEditor/Commands/EngineEditorCommands.h"
//#include "../SourceEditor/Commands/HeightmapEditorCommands.h"
//#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
//#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"
//#include "../SourceEditor/Prefs/EditorPrefs.h"
//#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

EditorDocument_Tilemap::EditorDocument_Tilemap(EngineCore* pEngineCore, ComponentTilemap* pTilemap)
: EditorDocument( pEngineCore )
{
    m_pCamera = MyNew ComponentCamera( pEngineCore, nullptr );
    m_pCamera->Reset();
    m_pCameraTransform = MyNew ComponentTransform( pEngineCore, nullptr );
    m_pCameraTransform->Reset();
    m_pCamera->m_pComponentTransform = m_pCameraTransform;

    //m_pCameraTransform->SetWorldPosition( Vector3( -1, 5, -5 ) );
    //m_pCameraTransform->SetWorldRotation( Vector3( -30, -30, 0 ) );

    m_pCameraTransform->SetWorldPosition( Vector3( 0, 5, 0 ) );
    m_pCameraTransform->SetWorldRotation( Vector3( -90, 0, 0 ) );

    m_pCamera->m_Orthographic = true;
    m_pCamera->m_DesiredWidth = 5;
    m_pCamera->m_DesiredHeight = 5;
    m_pCamera->m_OrthoNearZ = 0;
    m_pCamera->m_OrthoFarZ = 1000;

    m_pFBO = nullptr;
    m_WindowPos.Set( -1, -1 );
    m_WindowSize.Set( 0, 0 );
    m_WindowHovered = false;
    m_WindowFocused = false;
    m_WindowVisible = false;

    if( pTilemap )
    {
        m_TilemapOwnedByUs = false;

        m_pTilemap = pTilemap;
    }
    else
    {
        m_TilemapOwnedByUs = true;

        m_pTilemap = MyNew ComponentTilemap( pEngineCore, nullptr );
        m_pTilemap->Reset();
        m_pTilemap->m_Size.Set( 5, 5 );
        m_pTilemap->m_TileCount.Set( 8, 8 );
        MaterialDefinition* pMaterial = pEngineCore->GetManagers()->GetMaterialManager()->GetFirstMaterial();
        m_pTilemap->SetMaterial( pMaterial, 0 );
        m_pTilemap->CreateTilemap();
        m_pTilemap->RegisterCallbacks();
    }

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
    if( m_TilemapOwnedByUs )
    {
        m_pTilemap->SetEnabled( false );
        delete m_pTilemap;
    }

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

ComponentBase* EditorDocument_Tilemap::GetComponentBeingEdited()
{
    return m_pTilemap;
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
        pGameObject->SetName( "Tilemap editor - point" );
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
    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;

    MyAssert( m_pTilemap != nullptr );
    if( m_pTilemap == nullptr )
        return;

    MyAssert( m_pPoint != nullptr );
    if( m_pPoint == nullptr )
        return;

    ComponentRenderable* pRenderable;

    if( m_WindowVisible && m_WindowSize.LengthSquared() != 0 )
    {
        if( m_pFBO == nullptr )
        {
            m_pFBO = pEngineCore->GetManagers()->GetTextureManager()->CreateFBO( 1024, 1024, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, true );
            m_pFBO->MemoryPanel_Hide();
        }

        // Setup an ortho camera. TODO: Camera is a mess, fix it.
        {
            float aspect = m_WindowSize.y/m_WindowSize.x;

            m_pCamera->m_Orthographic = true;
            //m_pCamera->m_DesiredWidth = 5.0f;
            m_pCamera->m_DesiredHeight = m_pCamera->m_DesiredWidth * aspect;
            m_pCamera->m_OrthoNearZ = 0;
            m_pCamera->m_OrthoFarZ = 10; // value of 1000 causes issues with inverse in GetWorldSpaceMousePosition, not sure why.
            m_pCamera->ComputeProjectionMatrices();

            // Force top down.
            m_pCameraTransform->SetWorldRotation( Vector3( -90, 0, 0 ) );
        }

        m_pCamera->Tick( 0.0f );

        if( m_pFBO->GetColorTexture( 0 ) )
        {
            uint32 previousFBO = g_GLStats.m_CurrentFramebuffer;

            m_pFBO->Bind( false );
            MyViewport viewport( 0, 0, (uint32)m_WindowSize.x, (uint32)m_WindowSize.y );
            pEngineCore->GetRenderer()->EnableViewport( &viewport, true );

            pEngineCore->GetRenderer()->SetClearColor( ColorFloat( 0.0f,0.1f,0.1f,1.0f ) );
            pEngineCore->GetRenderer()->ClearBuffers( true, true, true );

            MyMatrix originalWorldMat;
            if( m_pTilemap->GetGameObject() )
                originalWorldMat = *m_pTilemap->GetGameObject()->GetTransform()->GetWorldTransform();
            else
                originalWorldMat.SetIdentity();
            MyMatrix* pEditorMatProj = &m_pCamera->m_Camera2D.m_matProj;
            MyMatrix* pEditorMatView = &m_pCamera->m_Camera2D.m_matView;

            bool wasVisible = m_pTilemap->IsVisible();
            m_pTilemap->SetVisible( true );

            if( m_pTilemap->GetGameObject() )
                m_pTilemap->GetGameObject()->GetTransform()->SetWorldTransform( &MyMatrix::Identity() );

            // Draw the tilemap.
            {
                g_pComponentSystemManager->DrawSingleComponent( pEditorMatProj, pEditorMatView, m_pTilemap, nullptr, 0 );
            }

            if( m_pTilemap->GetGameObject() )
                m_pTilemap->GetGameObject()->GetTransform()->SetWorldTransform( &originalWorldMat );

            m_pTilemap->SetVisible( wasVisible );

            // Draw a circle at the mouse position for the height desired by the level tool.
            pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
            //if( m_CurrentTool == Tool::Level && m_LevelUseBrushHeight == false )

            if( m_pTilemap->AreTileCoordsInBounds( m_MouseTilePos ) )
            {
                pRenderable->SetVisible( true );

                Vector3 worldPos = m_pTilemap->GetWorldPosAtTileCoords( m_MouseTilePos, true );

                m_pPoint->GetTransform()->SetLocalPosition( worldPos );

                MyMatrix* pEditorMatProj = &m_pCamera->m_Camera2D.m_matProj;
                MyMatrix* pEditorMatView = &m_pCamera->m_Camera2D.m_matView;

                pEngineCore->GetRenderer()->SetDepthFunction( MyRE::DepthFunc_Always );
                g_pComponentSystemManager->DrawSingleObject( pEditorMatProj, pEditorMatView, m_pPoint, nullptr );
                pEngineCore->GetRenderer()->SetDepthFunction( MyRE::DepthFunc_LEqual );
            }
            else
            {
                pRenderable->SetVisible( false );
            }

            g_pRenderer->BindFramebuffer( previousFBO );
        }
    }
}

void EditorDocument_Tilemap::AddImGuiOverlayItems()
{
}

void EditorDocument_Tilemap::CancelCurrentOperation()
{
    m_CurrentToolState = ToolState::Idle;
    m_pCommandStack->Undo( 1 );
}

Vector3 EditorDocument_Tilemap::GetWorldSpaceMousePosition(Vector2 mousepos)
{
    // Convert mouse coord into clip space.
    Vector2 mouseClip;
    mouseClip.x = (mousepos.x / m_WindowSize.x) * 2.0f - 1.0f;
    mouseClip.y = (mousepos.y / m_WindowSize.y) * 2.0f - 1.0f;

    // Compute the inverse view projection matrix.
    MyMatrix invVP = ( m_pCamera->m_Camera2D.m_matProj * m_pCamera->m_Camera2D.m_matView ).GetInverse();

    // Store the camera position as the near world point.
    Vector4 clipPoint4 = Vector4( mouseClip, 0, 1 );
    Vector4 worldPoint4 = invVP * clipPoint4;
    Vector3 worldPoint = Vector3( worldPoint4.x, 0, worldPoint4.z );

    return worldPoint;
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
        // Escape to cancel current tool or exit tilemap editor.
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
        // Find the mouse intersection point on the tilemap.
        Vector3 mouse = GetWorldSpaceMousePosition( Vector2( x, y ) );

        m_WorldSpaceMousePosition = mouse;

        m_MouseTilePos.Set( -1, -1 );
        m_pTilemap->GetTileCoordsAtWorldXZ( mouse.x, mouse.z, &m_MouseTilePos, nullptr );
    }

    // Handle camera movement, with both mouse and keyboard.
    if( inputHandled == false )
    {
        if( m_pCamera->HandleInputFor2DTopDownEditorCamera( keyAction, keyCode, mouseAction, id, x, y, pressure ) )
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

    //m_pCamera->m_Camera3D.SetupProjection( w/h, w/h, 45, 0.01f, 100.0f );
    //m_pCamera->m_Camera3D.UpdateMatrices();

    //ImGui::Text( "Testing Tilemap EditorDocument." );

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

    // Show some tilemap editor controls.
    ImGui::SetNextWindowSize( ImVec2(150,200), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowBgAlpha( 1.0f );
    ImGui::Begin( "Tilemap Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing );

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

    if( ImGui::Button( "Change Component Properties" ) )
    {
        m_pEngineCore->GetEditorState()->SelectComponent( m_pTilemap );
    }

    if( ImGui::Button( "Export as MyMesh" ) )
    {
        m_pTilemap->SaveAsMyMesh( "Data/Meshes/TestTilemap.mymesh" );
    }

    if( ImGui::Button( "Save" ) )
    {
        Save();
    }

    ImGui::End();

    if( m_ShowWarning_CloseEditor )
    {
        m_ShowWarning_CloseEditor = false;
        ImGui::OpenPopup( "Close Tilemap Editor Warning" );
    }
    if( ImGui::BeginPopupModal( "Close Tilemap Editor Warning" ) )
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

void EditorDocument_Tilemap::SetTilemap(ComponentTilemap* pTilemap)
{
    m_pTilemap = pTilemap;

    if( m_pTilemap )
    {
        // TODO: If no tilemap is created, then make a default one.
    }
}

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

        m_pTilemap->SaveAsTilemap( filename );
        MyFileInfo* pFileInfo = m_pEngineCore->GetComponentSystemManager()->LoadDataFile( filename, SceneID::SCENEID_MainScene, nullptr, false );
        m_pTilemap->SetTilemapFile( pFileInfo->GetFile() );
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

    m_pTilemap->LoadFromTilemap( filename );
    m_pTilemap->GenerateTilemapMesh( false, true, true );
}
