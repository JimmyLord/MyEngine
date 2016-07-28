//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "../../Voxels/VoxelBlock.h"
#include "../../Voxels/VoxelChunk.h"

EditorInterface_VoxelMeshEditor::EditorInterface_VoxelMeshEditor()
{
    m_pVoxelMesh = 0;

    m_CapturedRightMouse = false;
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
}

void EditorInterface_VoxelMeshEditor::CancelCurrentOperation()
{
}

bool EditorInterface_VoxelMeshEditor::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    VoxelChunk* pChunk = m_pVoxelMesh->GetChunk();

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_ESC )
    {
        // TODO: move this save op elsewhere.
        pChunk->SaveMyVoxelMesh( m_pVoxelMesh->m_pMesh->m_pSourceFile->m_FullPath );

        CancelCurrentOperation();
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_DELETE )
    {
    }

    EditorInterface::SetModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    if( mouseaction == GCBA_Held )
    {
        if( id & (1 << 1) && m_CapturedRightMouse )
        {
            return true;
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
                pChunk->ChangeBlockState( result.m_BlockWorldPosition, 0, false );
                pChunk->RebuildMesh( 1 );

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

    if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( id == 0 ) // left mouse button
        {
            if( mouseaction == GCBA_Down )
            {
                if( pChunk )
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
                        if( pChunk->IsInChunkSpace( result.m_BlockWorldPosition ) )
                        {
                            pChunk->ChangeBlockState( result.m_BlockWorldPosition, 1, true );
                            pChunk->RebuildMesh( 1 );
                        }
                    }
                }
            }

            if( mouseaction == GCBA_Held && id == 0 )
            {
            }

            if( mouseaction == GCBA_Up )
            {
            }
        }
    }

    // handle camera movement, with both mouse and keyboard.
    EditorInterface::HandleInputForEditorCamera( keyaction, keycode, mouseaction, id, x, y, pressure );

    return false;
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

    // transform ray to chunk space.
    MyMatrix transform = *m_pVoxelMesh->m_pGameObject->m_pComponentTransform->GetWorldTransform();
    transform.Inverse();
    Vector3 chunkspacestart = transform * start;
    Vector3 chunkspaceend = transform * end;

    VoxelChunk* pChunk = m_pVoxelMesh->GetChunk();
    Vector3 blocksize = pChunk->GetBlockSize();
    float smallestdimension = blocksize.x < blocksize.y ?
                              (blocksize.x < blocksize.z ? blocksize.x : blocksize.z) :
                              (blocksize.y < blocksize.z ? blocksize.y : blocksize.z);

    return pChunk->RayCast( chunkspacestart, chunkspaceend, smallestdimension/2.0f, pResult );
}
