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
        CancelCurrentOperation();
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );        
    }

    if( keyaction == GCBA_Up && keycode == MYKEYCODE_DELETE )
    {
    }

    EditorInterface::SetModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

    if( id == 1 ) // right mouse button to remove a block from the chunk
    {
        if( mouseaction == GCBA_Down )
        {
            Vector2 mousepos( x, y );

            Vector3 start, end;
            g_pEngineCore->GetMouseRay( mousepos, &start, &end );

            // TODO: transform ray to chunk space.

            VoxelRayCastResult result;
            pChunk->RayCast( start, end, 0.1f, &result );

            if( result.m_Hit == true )
            {
                pChunk->ChangeBlockState( result.m_BlockWorldPosition, false );
                pChunk->RebuildMesh( 1 );
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

                    Vector3 start, end;
                    g_pEngineCore->GetMouseRay( mousepos, &start, &end );

                    // TODO: transform ray to chunk space.

                    VoxelRayCastResult result;
                    pChunk->RayCast( start, end, 0.1f, &result );

                    if( result.m_Hit == true )
                    {
                        if( result.m_BlockFaceNormal.x == -1 ) result.m_BlockWorldPosition.x--;
                        if( result.m_BlockFaceNormal.x ==  1 ) result.m_BlockWorldPosition.x++;
                        if( result.m_BlockFaceNormal.y == -1 ) result.m_BlockWorldPosition.y--;
                        if( result.m_BlockFaceNormal.y ==  1 ) result.m_BlockWorldPosition.y++;
                        if( result.m_BlockFaceNormal.z == -1 ) result.m_BlockWorldPosition.z--;
                        if( result.m_BlockFaceNormal.z ==  1 ) result.m_BlockWorldPosition.z++;

                        // TODO: validate that result.m_BlockWorldPosition is inside the chunk

                        pChunk->ChangeBlockState( result.m_BlockWorldPosition, true );
                        pChunk->RebuildMesh( 1 );
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

    // clear mouse button states.
    EditorInterface::ClearModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );

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
