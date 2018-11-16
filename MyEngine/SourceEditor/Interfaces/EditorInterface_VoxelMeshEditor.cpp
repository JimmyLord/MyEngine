//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "../../SourceCommon/Voxels/VoxelBlock.h"
#include "../../SourceCommon/Voxels/VoxelChunk.h"
#include "../../SourceCommon/Voxels/VoxelWorld.h"

EditorInterface_VoxelMeshEditor::EditorInterface_VoxelMeshEditor()
{
    m_pVoxelWorld = 0;
    m_pVoxelMesh = 0;

    m_CapturedRightMouse = false;

    m_CurrentBlockType = 1;
}

EditorInterface_VoxelMeshEditor::~EditorInterface_VoxelMeshEditor()
{
}

void EditorInterface_VoxelMeshEditor::Initialize()
{
    //EditorInterface::Initialize();
}

void EditorInterface_VoxelMeshEditor::OnActivated()
{
    EditorInterface::OnActivated();
}

void EditorInterface_VoxelMeshEditor::OnDeactivated()
{
    EditorInterface::OnDeactivated();
}

void EditorInterface_VoxelMeshEditor::OnDrawFrame(unsigned int canvasid)
{
    EditorInterface::OnDrawFrame( canvasid );

    if( g_GLCanvasIDActive != 1 )
        return;

    VoxelChunk* pChunk = 0;
    if( m_pVoxelMesh )
        pChunk = m_pVoxelMesh->GetChunk();

#if !MYFW_OPENGLES2
    return;
#endif

    ImGui::SetNextWindowSize( ImVec2(150,50), ImGuiSetCond_FirstUseEver );
    ImGui::Begin( "Voxel Mesh Editor" );

    if( pChunk )
    {
        if( ImGui::Button( "Fill" ) )
        {
            pChunk->ChangeBlockState( Vector3Int(0,0,0), m_CurrentBlockType, true );
            pChunk->RebuildMesh( 1 );
        }

        if( ImGui::Button( "Clear" ) )
        {
            pChunk->ChangeBlockState( Vector3Int(0,0,0), m_CurrentBlockType, false );
            pChunk->RebuildMesh( 1 );
        }
    }

    if( ImGui::Button( "Save" ) )
    {
        SaveVoxelMesh();
    }

    ImVec4 selectedcolor = ImVec4( 0.5f, 1.0f, 0.5f, 1.0f );

    ImGui::Columns( 5, 0, true );

    int buttonpressed = 0;

    if( m_CurrentBlockType == 1 ) ImGui::PushStyleColor( ImGuiCol_Button, selectedcolor );
    if( ImGui::Button( "1" ) ) buttonpressed = 1;
    if( m_CurrentBlockType == 1 ) ImGui::PopStyleColor();

    ImGui::NextColumn();
    if( m_CurrentBlockType == 2 ) ImGui::PushStyleColor( ImGuiCol_Button, selectedcolor );
    if( ImGui::Button( "2" ) ) buttonpressed = 2;
    if( m_CurrentBlockType == 2 ) ImGui::PopStyleColor();

    ImGui::NextColumn();
    if( m_CurrentBlockType == 3 ) ImGui::PushStyleColor( ImGuiCol_Button, selectedcolor );
    if( ImGui::Button( "3" ) ) buttonpressed = 3;
    if( m_CurrentBlockType == 3 ) ImGui::PopStyleColor();

    ImGui::NextColumn();
    if( m_CurrentBlockType == 4 ) ImGui::PushStyleColor( ImGuiCol_Button, selectedcolor );
    if( ImGui::Button( "4" ) ) buttonpressed = 4;
    if( m_CurrentBlockType == 4 ) ImGui::PopStyleColor();

    ImGui::NextColumn();
    if( m_CurrentBlockType == 5 ) ImGui::PushStyleColor( ImGuiCol_Button, selectedcolor );
    if( ImGui::Button( "5" ) ) buttonpressed = 5;
    if( m_CurrentBlockType == 5 ) ImGui::PopStyleColor();

    ImGui::Columns( 1, 0, true );

    ImGui::End();

    if( ImGui::IsKeyDown( '1' ) ) m_CurrentBlockType = 1;
    if( ImGui::IsKeyDown( '2' ) ) m_CurrentBlockType = 2;
    if( ImGui::IsKeyDown( '3' ) ) m_CurrentBlockType = 3;
    if( ImGui::IsKeyDown( '4' ) ) m_CurrentBlockType = 4;
    if( ImGui::IsKeyDown( '5' ) ) m_CurrentBlockType = 5;

    if( buttonpressed != 0 )
    {
        m_CurrentBlockType = buttonpressed;
    }
}

void EditorInterface_VoxelMeshEditor::CancelCurrentOperation()
{
}

void EditorInterface_VoxelMeshEditor::SaveVoxelMesh()
{
    if( m_pVoxelWorld )
    {
        LOGInfo( LOGTag, "Voxel Editor: Voxel World Saved\n" );

        m_pVoxelWorld->GetWorld()->SaveTheWorld();
    }
    else
    {
        LOGInfo( LOGTag, "Voxel Editor: Voxel Mesh Saved\n" );

        VoxelChunk* pChunk = m_pVoxelMesh->GetChunk();
        cJSON* jVoxelMesh = pChunk->ExportAsJSONObject();

        char* string = cJSON_Print( jVoxelMesh );

        FILE* pFile;
#if MYFW_WINDOWS
        fopen_s( &pFile, m_pVoxelMesh->m_pMesh->GetFile()->GetFullPath(), "wb" );
#else
        pFile = fopen( m_pVoxelMesh->m_pMesh->GetFile()->GetFullPath(), "wb" );
#endif
        fprintf( pFile, "%s", string );
        fclose( pFile );

        cJSON_Delete( jVoxelMesh );

        cJSONExt_free( string );
    }
}

bool EditorInterface_VoxelMeshEditor::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    VoxelChunk* pChunk = 0;
    if( m_pVoxelMesh )
        pChunk = m_pVoxelMesh->GetChunk();

    if( keyaction == GCBA_Down )
    {
        if( keycode == '1' ) { m_CurrentBlockType = 1; return true; }
        if( keycode == '2' ) { m_CurrentBlockType = 2; return true; }
        if( keycode == '3' ) { m_CurrentBlockType = 3; return true; }
        if( keycode == '4' ) { m_CurrentBlockType = 4; return true; }
        if( keycode == '5' ) { m_CurrentBlockType = 5; return true; }
    }

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_ESC )
    {
        // TODO: move this save op elsewhere.
        SaveVoxelMesh();

        //CancelCurrentOperation();
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_DELETE )
    {
    }

    EditorInterface::SetModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    // if right button is captured and held, then don't call let camera function get called below.
    if( mouseaction == GCBA_Held )
    {
        if( id == 1 /*& (1 << 1)*/ && m_CapturedRightMouse )
        {
            return true;
        }
    }

    if( id == 0 ) // left mouse button to add a block
    {
        if( mouseaction == GCBA_Down )
        {
            Vector2 mousepos( x, y );
            VoxelRayCastResult result;
            RayCast( mousepos, &result );

            if( result.m_Hit == true )
            {
                if( result.m_BlockFaceNormal.x == -1 ) result.m_BlockWorldPosition.x--;
                if( result.m_BlockFaceNormal.x ==  1 ) result.m_BlockWorldPosition.x++;
                if( result.m_BlockFaceNormal.y == -1 ) result.m_BlockWorldPosition.y--;
                if( result.m_BlockFaceNormal.y ==  1 ) result.m_BlockWorldPosition.y++;
                if( result.m_BlockFaceNormal.z == -1 ) result.m_BlockWorldPosition.z--;
                if( result.m_BlockFaceNormal.z ==  1 ) result.m_BlockWorldPosition.z++;

                // if result.m_BlockWorldPosition is inside the chunk, add a block
                if( m_pVoxelWorld )
                {
                    VoxelWorld* pWorld = m_pVoxelWorld->GetWorld();
                    pWorld->ChangeBlockState( result.m_BlockWorldPosition, m_CurrentBlockType, true );
                }

                if( m_pVoxelMesh )
                {
                    if( pChunk->IsInChunkSpace( result.m_BlockWorldPosition ) )
                    {
                        pChunk->ChangeBlockState( result.m_BlockWorldPosition, m_CurrentBlockType, true );
                        pChunk->RebuildMesh( 1 );
                    }
                }
            }
        }
    }

    if( id == 1 ) // right mouse button to remove a block from the chunk
    {
        if( mouseaction == GCBA_Down )
        {
            Vector2 mousepos( x, y );
            VoxelRayCastResult result;
            RayCast( mousepos, &result );

            if( result.m_Hit == true )
            {
                if( m_pVoxelWorld )
                {
                    VoxelWorld* pWorld = m_pVoxelWorld->GetWorld();
                    pWorld->ChangeBlockState( result.m_BlockWorldPosition, 0, false );
                }

                if( m_pVoxelMesh )
                {
                    pChunk->ChangeBlockState( result.m_BlockWorldPosition, 0, false );
                    pChunk->RebuildMesh( 1 );
                }

                m_CapturedRightMouse = true;
                return true;
            }
        }

        if( mouseaction == GCBA_Up )
        {
            if( m_CapturedRightMouse )
            {
                m_CapturedRightMouse = false;
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_RightMouse;
                return true;
            }
        }
    }

    if( id == 2 ) // middle mouse button to pick the color from the block
    {
        if( mouseaction == GCBA_Down )
        {
            Vector2 mousepos( x, y );
            VoxelRayCastResult result;
            RayCast( mousepos, &result );

            // if result.m_BlockWorldPosition is inside the chunk, add a block
            if( m_pVoxelWorld )
            {
                VoxelWorld* pWorld = m_pVoxelWorld->GetWorld();
                VoxelBlock* pBlock = pWorld->GetBlock( result.m_BlockWorldPosition );
                m_CurrentBlockType = pBlock->GetBlockType();
            }

            if( m_pVoxelMesh )
            {
                if( pChunk->IsInChunkSpace( result.m_BlockWorldPosition ) )
                {
                    VoxelBlock* pBlock = pChunk->GetBlockFromLocalPos( result.m_BlockWorldPosition );
                    m_CurrentBlockType = pBlock->GetBlockType();
                }
            }
        }
    }

    // handle camera movement, with both mouse and keyboard.
    EditorInterface::HandleInputForEditorCamera( keyaction, keycode, mouseaction, id, x, y, pressure );

    return false;
}

void EditorInterface_VoxelMeshEditor::SetWorldToEdit(ComponentVoxelWorld* pVoxelWorld)
{
    m_pVoxelWorld = pVoxelWorld;
}

void EditorInterface_VoxelMeshEditor::SetMeshToEdit(ComponentVoxelMesh* pVoxelMesh)
{
    m_pVoxelMesh = pVoxelMesh;
}

void EditorInterface_VoxelMeshEditor::RenderObjectIDsToFBO()
{
    EditorInterface::RenderObjectIDsToFBO();
}

bool EditorInterface_VoxelMeshEditor::RayCast(Vector2 mousepos, VoxelRayCastResult* pResult)
{
    Vector3 start, end;
    g_pEngineCore->GetMouseRay( mousepos, &start, &end );

    if( m_pVoxelWorld )
    {
        // raycast against the world
        VoxelWorld* pWorld = m_pVoxelWorld->GetWorld();
        Vector3 blocksize = pWorld->GetBlockSize();
        float smallestdimension = blocksize.x < blocksize.y ?
                                  (blocksize.x < blocksize.z ? blocksize.x : blocksize.z) :
                                  (blocksize.y < blocksize.z ? blocksize.y : blocksize.z);

        return pWorld->RayCast( start, end, smallestdimension/10.0f, pResult );
    }

    if( m_pVoxelMesh )
    {
        // transform ray to chunk space.
        MyMatrix transform = *m_pVoxelMesh->GetGameObject()->GetTransform()->GetWorldTransform();
        transform.Inverse();
        Vector3 chunkspacestart = transform * start;
        Vector3 chunkspaceend = transform * end;

        // raycast against the single chunk
        VoxelChunk* pChunk = m_pVoxelMesh->GetChunk();
        Vector3 blocksize = pChunk->GetBlockSize();
        float smallestdimension = blocksize.x < blocksize.y ?
                                  (blocksize.x < blocksize.z ? blocksize.x : blocksize.z) :
                                  (blocksize.y < blocksize.z ? blocksize.y : blocksize.z);

        return pChunk->RayCast( chunkspacestart, chunkspaceend, smallestdimension/10.0f, pResult );
    }

    return false;
}
