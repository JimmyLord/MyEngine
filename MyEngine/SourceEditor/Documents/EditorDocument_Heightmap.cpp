//
// Copyright (c) 2019-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorDocument_Heightmap.h"
#include "Camera/Camera3D.h"
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
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

class Job_CalculateNormals : public MyJob
{
protected:
    EditorDocument_Heightmap* m_pHeightmapEditor;

public:
    Job_CalculateNormals()
    {
        m_pHeightmapEditor = nullptr;
    }
    virtual ~Job_CalculateNormals() {}

    void SetEditor(EditorDocument_Heightmap* pHeightmapEditor)
    {
        m_pHeightmapEditor = pHeightmapEditor;
    }

    virtual void DoWork()
    {
        m_pHeightmapEditor->GetHeightmapBeingEdited()->RecalculateNormals();
    }
};

EditorDocument_Heightmap::EditorDocument_Heightmap(EngineCore* pEngineCore, const char* relativePath, ComponentHeightmap* pHeightmap)
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

    if( pHeightmap )
    {
        m_HeightmapOwnedByUs = false;

        m_pHeightmap = pHeightmap;

        MyFileObject* pFile = m_pHeightmap->GetHeightmapFile();
        SetRelativePath( pFile->GetFullPath() );
    }
    else
    {
        m_HeightmapOwnedByUs = true;

        m_pHeightmap = MyNew ComponentHeightmap( pEngineCore, nullptr );
        m_pHeightmap->Reset();
        m_pHeightmap->m_Size.Set( 5, 5 );
        m_pHeightmap->m_VertCount.Set( 128, 128 );

        // Grab the first material that was loaded by the user.
        MaterialDefinition* pMaterial = pEngineCore->GetManagers()->GetMaterialManager()->GetFirstMaterial();
        if( pMaterial->GetFile() != nullptr )
        {
            m_pHeightmap->SetMaterial( pMaterial, 0 );
        }

        m_pHeightmap->CreateHeightmap();
        m_pHeightmap->RegisterCallbacks();

        if( relativePath != nullptr && relativePath[0] != '\0' )
        {
            SetRelativePath( relativePath );
            MyFileInfo* pFileInfo = m_pEngineCore->GetComponentSystemManager()->LoadDataFile( relativePath, SCENEID_MainScene, nullptr, false );
            m_pHeightmap->SetHeightmapFile( pFileInfo->GetFile() );
        }
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

    m_HeightmapNormalsNeedRebuilding = false;
    m_pJob_CalculateNormals = MyNew Job_CalculateNormals();
    m_pJob_CalculateNormals->SetEditor( this );

    // Warnings.
    m_ShowWarning_CloseEditor = false;

    // Editor settings.
    m_BrushSoftness = 0.1f;
    m_BrushRadius = 1.0f;
    m_RaiseAmount = 0.5f;
    m_LevelUseBrushHeight = true;
    m_LevelHeight = 0.0f;

    // Noise Settings.
    m_Noise_Dirty = true;
    m_Noise_AutoGenerate = true;
    m_Noise_Seed = 0;
    m_Noise_Amplitude = 1.5f;
    m_Noise_Frequency.Set( 0.05f, 0.05f );
    m_Noise_Offset.Set( 0.0f, 0.0f );
    m_Noise_Octaves = 3;
    m_Noise_Persistance = 0.08f;
    m_Noise_Lacunarity = 5.0f;
    m_Noise_Debug_Octave = -1;

    m_AlwaysRecalculateNormals = false;

    Initialize();
}

EditorDocument_Heightmap::~EditorDocument_Heightmap()
{
    if( m_HeightmapOwnedByUs )
    {
        m_pHeightmap->SetEnabled( false );
        delete m_pHeightmap;
    }

    SAFE_DELETE( m_pJob_CalculateNormals );

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

void EditorDocument_Heightmap::Initialize()
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

bool EditorDocument_Heightmap::IsBusy()
{
    if( m_pJob_CalculateNormals )
    {
        if( m_pJob_CalculateNormals->IsQueued() )
            return true;
    }

    return false;
}

void EditorDocument_Heightmap::OnDrawFrame()
{
    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;

    MyAssert( m_pHeightmap != nullptr );
    if( m_pHeightmap == nullptr )
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

        m_pCamera->Tick( 0.0f );

        if( m_pFBO->GetColorTexture( 0 ) )
        {
            uint32 previousFBO = g_GLStats.m_CurrentFramebuffer;

            m_pFBO->Bind( false );
            MyViewport viewport( 0, 0, (uint32)m_WindowSize.x, (uint32)m_WindowSize.y );
            pEngineCore->GetRenderer()->EnableViewport( &viewport, true );
        
            pEngineCore->GetRenderer()->SetClearColor( ColorFloat( 0.0f,0.1f,0.1f,1.0f ) );
            pEngineCore->GetRenderer()->ClearBuffers( true, true, true );

            //MyMatrix* pWorldMat = m_pHeightmap->GetGameObject()->GetTransform()->GetWorldTransform();
            //Vector3 localSpacePoint = pWorldMat->GetInverse() * m_WorldSpaceMousePosition;
            Vector3 localSpacePoint = m_WorldSpaceMousePosition;
            MyMatrix originalWorldMat;
            if( m_pHeightmap->GetGameObject() )
                originalWorldMat = *m_pHeightmap->GetGameObject()->GetTransform()->GetWorldTransform();
            else
                originalWorldMat.SetIdentity();
            MyMatrix* pEditorMatProj = &m_pCamera->m_Camera3D.m_matProj;
            MyMatrix* pEditorMatView = &m_pCamera->m_Camera3D.m_matView;

            bool wasVisible = m_pHeightmap->IsVisible();
            m_pHeightmap->SetVisible( true );

            if( m_pHeightmap->GetGameObject() )
                m_pHeightmap->GetGameObject()->GetTransform()->SetWorldTransform( &MyMatrix::Identity() );

            // Draw the heightmap.
            {
                g_pComponentSystemManager->DrawSingleComponent( pEditorMatProj, pEditorMatView, m_pHeightmap, nullptr, 0 );
            }

            // Draw the brush circle projected on the heightmap.
            {
                MaterialDefinition* pMaterial = m_pMaterials[Mat_BrushOverlay];
                if( pMaterial->IsFullyLoaded() )
                {
                    Vector2 size = m_pHeightmap->m_Size;
                    Vector2 scale = Vector2( m_BrushRadius, m_BrushRadius ) * 2;
                    pMaterial->SetUVScale( 1.0f/scale );
                    pMaterial->SetUVOffset( Vector2( -localSpacePoint.x + scale.x/2.0f, -localSpacePoint.z + scale.y/2.0f ) );
                    pEngineCore->GetRenderer()->SetTextureWrapModes( pMaterial->GetTextureColor(), MyRE::WrapMode_Clamp, MyRE::WrapMode_Clamp );
                    g_pComponentSystemManager->DrawSingleComponent( pEditorMatProj, pEditorMatView, m_pHeightmap, &pMaterial, 1 );
                }
            }

            if( m_pHeightmap->GetGameObject() )
                m_pHeightmap->GetGameObject()->GetTransform()->SetWorldTransform( &originalWorldMat );

            m_pHeightmap->SetVisible( wasVisible );

            // Draw a circle at the mouse position for the height desired by the level tool.
            pRenderable = (ComponentRenderable*)m_pPoint->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
            if( m_CurrentTool == Tool::Level && m_LevelUseBrushHeight == false )
            {
                pRenderable->SetVisible( true );

                Vector3 worldPos = m_WorldSpaceMousePositionAtDesiredHeight;

                m_pPoint->GetTransform()->SetLocalPosition( worldPos );

                MyMatrix* pEditorMatProj = &m_pCamera->m_Camera3D.m_matProj;
                MyMatrix* pEditorMatView = &m_pCamera->m_Camera3D.m_matView;

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

void EditorDocument_Heightmap::AddImGuiOverlayItems()
{
    EngineCore* pEngineCore = EditorDocument::m_pEngineCore;

    if( m_pHeightmap && m_pHeightmap->GetGameObject() )
    {
        ImGui::Text( "Editing Heightmap: %s", m_pHeightmap->GetGameObject()->GetName() );
    }

    if( m_pHeightmap->GetMaterial( 0 ) == nullptr )
    {
        ImGui::Text( "No material assigned, load and drop one on the window." );
    }

    //if( m_pCommandStack->GetUndoStackSize() > 0 )
    //{
    //    ImGui::Text( "Unsaved changes" );
    //}

    if( m_pJob_CalculateNormals && m_pJob_CalculateNormals->IsQueued() )
    {
        ImGui::Text( "Recalculating normals" );
    }
}

void EditorDocument_Heightmap::CancelCurrentOperation()
{
    m_CurrentToolState = ToolState::Idle;
    m_pCommandStack->Undo( 1 );
}

void EditorDocument_Heightmap::GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end)
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

bool EditorDocument_Heightmap::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
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
        // Find the mouse intersection point on the heightmap.
        Vector3 start, end;
        GetMouseRay( Vector2( x, y ), &start, &end );

        Vector3 localSpaceMousePosition;
        bool mouseRayIntersected = m_pHeightmap->RayCast( false, start, end, &localSpaceMousePosition );

        // Show the brush at that point.
        //MyMatrix* pWorldMat = m_pHeightmap->GetGameObject()->GetTransform()->GetWorldTransform();
        //m_WorldSpaceMousePosition = *pWorldMat * localSpaceMousePosition;
        m_WorldSpaceMousePosition = localSpaceMousePosition;

        if( m_CurrentTool == Tool::Level && m_LevelUseBrushHeight == false )
        {
            Vector3 localSpacePointAtDesiredHeight;

            m_pHeightmap->RayCastAtLocalHeight( false, start, end, m_LevelHeight, &localSpacePointAtDesiredHeight, &localSpaceMousePosition );

            //m_WorldSpaceMousePositionAtDesiredHeight = *pWorldMat * localSpacePointAtDesiredHeight;
            //m_WorldSpaceMousePosition = *pWorldMat * localSpaceMousePosition;
            m_WorldSpaceMousePositionAtDesiredHeight = localSpacePointAtDesiredHeight;
            m_WorldSpaceMousePosition = localSpaceMousePosition;
        }

        if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
        {
            // Right mouse button to cancel current operation.
            if( mouseAction == GCBA_Down && id == 1 && m_CurrentTool != Tool::None )
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
                    m_WorldSpaceMousePositionWhenToolStarted = m_WorldSpaceMousePosition;
                    ApplyCurrentTool( localSpaceMousePosition, mouseAction );
                }
            }

            if( mouseAction == GCBA_Up && id == 0 ) // Left mouse button up.
            {
                //MyMatrix* pWorldMat = m_pHeightmap->GetGameObject()->GetTransform()->GetWorldTransform();
                //Vector3 localSpacePoint = pWorldMat->GetInverse() * m_WorldSpaceMousePosition;
                Vector3 localSpacePoint = m_WorldSpaceMousePosition;
                ApplyCurrentTool( localSpacePoint, mouseAction );

                // Wait for any outstanding jobs to complete.
                pEngineCore->GetManagers()->GetJobManager()->WaitForJobToComplete( m_pJob_CalculateNormals );

                m_CurrentToolState = ToolState::Idle;

                // Rebuild heightmap normals when mouse is lifted after 'raise' command.
                if( m_HeightmapNormalsNeedRebuilding )
                {
                    m_HeightmapNormalsNeedRebuilding = false;
                    m_pHeightmap->RecalculateNormals();
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
                    if( plane.IntersectRay( start, start-end, &flatIntersectPoint ) )
                    {
                        //Vector3 localSpacePoint = pWorldMat->GetInverse() * flatIntersectPoint;
                        Vector3 localSpacePoint = flatIntersectPoint;
                        ApplyCurrentTool( localSpacePoint, mouseAction );

                        m_WorldSpaceMousePosition = flatIntersectPoint;
                    }
                }
            }
        }
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

bool EditorDocument_Heightmap::ExecuteHotkeyAction(HotkeyAction action)
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

void EditorDocument_Heightmap::Update()
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

            if( ImGui::BeginDragDropTarget() )
            {
                if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Material" ) )
                {
                    MaterialDefinition* pNewMat = (MaterialDefinition*)*(void**)payload->Data;

                    m_pHeightmap->SetMaterial( pNewMat, 0 );
                }
                ImGui::EndDragDropTarget();
            }
        }
    }

    // Show some heightmap editor controls.
    ImGui::SetNextWindowSize( ImVec2(150,200), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowBgAlpha( 1.0f );
    ImGui::Begin( "Heightmap Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing );

    if( ImGui::BeginTabBar( "HeightmapEditorTools" ) )
    {
        if( ImGui::BeginTabItem( "Paint" ) )
        {
            ImGui::EndTabItem();
            AddPaintTools();
        }
        if( ImGui::BeginTabItem( "Noise" ) )
        {
            ImGui::EndTabItem();

            m_CurrentTool = Tool::None;
            m_CurrentToolState = ToolState::Idle;

            AddNoiseTools();
        }

        ImGui::EndTabBar();
    }

    // If the editor controls window is focused, consider the entire editor document in focus.
    bool editorControlsWindowIsFocused = ImGui::IsWindowFocused( ImGuiFocusedFlags_RootAndChildWindows );
    if( m_WindowFocused == false )
    {
        m_WindowFocused = editorControlsWindowIsFocused;
    }

    ImGui::End(); // "Heightmap Editor" window.

    // Handle save warnings when closing the document window.
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

void EditorDocument_Heightmap::AddPaintTools()
{
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
        ImGui::Checkbox( "Always recalculate normals", &m_AlwaysRecalculateNormals );
    }

    // Tool specific values.
    if( ImGui::CollapsingHeader( "Brush", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::DragFloat( "Softness", &m_BrushSoftness, 0.01f, 0.0f, 1.0f );
        ImGui::DragFloat( "Radius", &m_BrushRadius, 0.01f, 0.0f, FLT_MAX );
    }

    if( ImGui::CollapsingHeader( "Raise/Lower", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::DragFloat( "Amount", &m_RaiseAmount, 0.01f, 0.0f, FLT_MAX );
    }

    if( ImGui::CollapsingHeader( "Level", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::Checkbox( "Use brush height", &m_LevelUseBrushHeight );
        ImGui::DragFloat( "Height", &m_LevelHeight, 0.01f, 0.0f, FLT_MAX );
    }

    // Other.
    ImGui::Separator();

    if( ImGui::Button( "Export as MyMesh" ) )
    {
        m_pHeightmap->SaveAsMyMesh( "Data/Meshes/TestHeightmap.mymesh" );
    }

    if( ImGui::Button( "Save" ) )
    {
        Save();
    }
}

void EditorDocument_Heightmap::AddNoiseTools()
{
    bool forceRebuild = false;

    ImGui::Checkbox( "Auto Rebuild", &m_Noise_AutoGenerate );

    if( ImGui::DragInt( "Seed", &m_Noise_Seed, 1, 0, INT_MAX ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragFloat( "Amplitude", &m_Noise_Amplitude, 0.01f, 0.01f, 100.0f ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragFloat2( "Frequency", &m_Noise_Frequency.x, 0.001f, 0.001f, 100.0f ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragFloat2( "Offset", &m_Noise_Offset.x, 0.01f ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragInt( "Octaves", &m_Noise_Octaves, 1, 1, 6 ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragFloat( "Persistance", &m_Noise_Persistance, 0.01f, 0.0f, 1.0f ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragFloat( "Lacunarity", &m_Noise_Lacunarity, 0.01f, 1.0f, 100.0f ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::DragInt( "Debug Octave", &m_Noise_Debug_Octave, 1, -1, m_Noise_Octaves ) )
    {
        m_Noise_Dirty = true;
    }
    if( ImGui::Button( "Generate" ) )
    {
        m_Noise_Dirty = true;
        forceRebuild = true;
    }

    if( m_Noise_Dirty && (m_Noise_AutoGenerate || forceRebuild) )
    {
        m_pHeightmap->FillWithNoise( m_Noise_Seed, m_Noise_Amplitude, m_Noise_Frequency, m_Noise_Offset, m_Noise_Octaves, m_Noise_Persistance, m_Noise_Lacunarity, m_Noise_Debug_Octave );
        m_Noise_Dirty = false;
    }
}

void EditorDocument_Heightmap::SetHeightmap(ComponentHeightmap* pHeightmap)
{
    m_pHeightmap = pHeightmap;

    if( m_pHeightmap )
    {
        // TODO: If no heightmap is created, then make a default flat one.
    }
}

MaterialDefinition* EditorDocument_Heightmap::GetMaterial(MaterialTypes type)
{
    MyAssert( type >= 0 && type < Mat_NumMaterials );

    return m_pMaterials[type];
}

void EditorDocument_Heightmap::ApplyCurrentTool(Vector3 localSpacePoint, int mouseAction)
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
            // Ignore mouse up's.
            if( mouseAction == GCBA_Up )
                break;

            float amount = m_RaiseAmount * 1/60.0f; // TODO: Get delta time.
            if( m_CurrentTool == Tool::Lower )
                amount *= -1;

            // Raise the terrain and add to command stack.
            if( m_pHeightmap->Tool_Raise( localSpacePoint, amount, m_BrushRadius, m_BrushSoftness, true ) )
            {
                EditorCommand_Heightmap_Raise* pCommand = MyNew EditorCommand_Heightmap_Raise( m_pHeightmap, localSpacePoint, amount, m_BrushRadius, m_BrushSoftness );

                // Attach to previous 'raise' command if this is a held message,
                //   otherwise it's a mouse down so create a new message.
                bool attachToPrevious = mouseAction == GCBA_Held ? true : false;
                m_pCommandStack->Add( pCommand, attachToPrevious );

                valuesChanged = true;
            }
        }
        break;

    case Tool::Level:
        {
            if( mouseAction == GCBA_Down )
            {
                EditorCommand_Heightmap_FullBackup* pCommand = MyNew EditorCommand_Heightmap_FullBackup( m_pHeightmap );
                m_pCommandStack->Add( pCommand, false );
            }
            if( mouseAction == GCBA_Up )
            {
                EditorCommand_Heightmap_FullBackup* pCommand = (EditorCommand_Heightmap_FullBackup*)m_pCommandStack->GetUndoCommandAtIndex( m_pCommandStack->GetUndoStackSize() - 1 );
                pCommand->CopyInFinalHeights();
                break;
            }

            float height = m_LevelUseBrushHeight ? localSpacePoint.y : m_LevelHeight;
            Vector3 point = m_LevelUseBrushHeight ? localSpacePoint : m_WorldSpaceMousePositionAtDesiredHeight;

            if( m_pHeightmap->Tool_Level( point, height, m_BrushRadius, m_BrushSoftness, true ) )
            {
                // TODO: Undo/Redo.

                valuesChanged = true;
            }
        }
        break;

    case Tool::None:
        break;
    }

    // Deal with normals.
    if( valuesChanged )
    {
        if( m_AlwaysRecalculateNormals )
        {
            m_pHeightmap->RecalculateNormals();
        }
        else
        {
            // Add a job to regenerate the normals on another thread.
            if( m_pJob_CalculateNormals->IsQueued() == false )
            {
                pEngineCore->GetManagers()->GetJobManager()->AddJob( m_pJob_CalculateNormals );
            }
            else
            {
                m_HeightmapNormalsNeedRebuilding = true;
            }
        }
    }
}

//====================================================================================================
// Protected Methods.
//====================================================================================================
void EditorDocument_Heightmap::Save()
{
    EditorDocument::Save();

    // Save NodeGraph as JSON string.
    {
        const char* filename = GetRelativePath();
        if( filename[0] == '\0' )
        {
            return;
        }

        m_pHeightmap->SaveAsHeightmap( filename );
        //MyFileInfo* pFileInfo = m_pEngineCore->GetComponentSystemManager()->LoadDataFile( filename, SceneID::SCENEID_MainScene, nullptr, false );
        //m_pHeightmap->SetHeightmapFile( pFileInfo->GetFile() );
    }
}

void EditorDocument_Heightmap::Load()
{
    EditorDocument::Load();

    const char* filename = GetRelativePath();
    if( filename[0] == '\0' )
    {
        return;
    }

    m_pHeightmap->LoadFromHeightmap( filename );
    m_pHeightmap->GenerateHeightmapMesh( true, true );
}