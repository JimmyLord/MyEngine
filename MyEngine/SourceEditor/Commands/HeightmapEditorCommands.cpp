//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "HeightmapEditorCommands.h"

//====================================================================================================
// EditorCommand_Heightmap_Raise
//====================================================================================================

EditorCommand_Heightmap_Raise::EditorCommand_Heightmap_Raise(ComponentHeightmap* pHeightmap, Vector3 position, float amount, float radius, float softness)
{
    m_Name = "EditorCommand_Heightmap_Raise";

    MyAssert( pHeightmap != nullptr );

    m_pHeightmap = pHeightmap;
    m_Position = position;
    m_BrushSoftness = softness;
    m_RaiseAmount = amount;
    m_BrushRadius = radius;
}

EditorCommand_Heightmap_Raise::~EditorCommand_Heightmap_Raise()
{
}

void EditorCommand_Heightmap_Raise::Do()
{
    m_pHeightmap->Tool_Raise( m_Position, m_RaiseAmount, m_BrushRadius, m_BrushSoftness, true );

    // Regenerate the heightmap normals when redoing the last 'raise' command in the clump.
    if( this->m_LinkedToNextCommandOnRedoStack == false )
    {
        m_pHeightmap->GenerateHeightmapMesh( false, false, true );
    }
}

void EditorCommand_Heightmap_Raise::Undo()
{
    m_pHeightmap->Tool_Raise( m_Position, -m_RaiseAmount, m_BrushRadius, m_BrushSoftness, true );

    // Regenerate the heightmap normals when undoing the last 'raise' command in the clump.
    if( this->m_LinkedToPreviousCommandOnUndoStack == false )
    {
        m_pHeightmap->GenerateHeightmapMesh( false, false, true );
    }
}

EditorCommand* EditorCommand_Heightmap_Raise::Repeat()
{
    // Do nothing.

    return nullptr;
}

//====================================================================================================
// EditorCommand_Heightmap_FullBackup
//====================================================================================================

EditorCommand_Heightmap_FullBackup::EditorCommand_Heightmap_FullBackup(ComponentHeightmap* pHeightmap)
{
    m_Name = "EditorCommand_Heightmap_FullBackup";

    MyAssert( pHeightmap != nullptr );

    m_pHeightmap = pHeightmap;
    uint32 numVerts = m_pHeightmap->m_VertCount.x * m_pHeightmap->m_VertCount.y;
    m_PreviousHeights = new float[numVerts];
    for( uint32 i=0; i<numVerts; i++ )
    {
        m_PreviousHeights[i] = m_pHeightmap->m_Heights[i];
    }
    m_FinalHeights = new float[numVerts];
}

EditorCommand_Heightmap_FullBackup::~EditorCommand_Heightmap_FullBackup()
{
    delete[] m_PreviousHeights;
    delete[] m_FinalHeights;
}

void EditorCommand_Heightmap_FullBackup::CopyInFinalHeights()
{
    uint32 numVerts = m_pHeightmap->m_VertCount.x * m_pHeightmap->m_VertCount.y;
    for( uint32 i=0; i<numVerts; i++ )
    {
        m_FinalHeights[i] = m_pHeightmap->m_Heights[i];
    }
}

void EditorCommand_Heightmap_FullBackup::Do()
{
    uint32 numVerts = m_pHeightmap->m_VertCount.x * m_pHeightmap->m_VertCount.y;
    for( uint32 i=0; i<numVerts; i++ )
    {
        m_pHeightmap->m_Heights[i] = m_FinalHeights[i];
    }

    // Regenerate the heightmap when redoing the last 'raise' command in the clump.
    //if( this->m_LinkedToNextCommandOnRedoStack == false )
    {
        m_pHeightmap->GenerateHeightmapMesh( false, false, true );
    }
}

void EditorCommand_Heightmap_FullBackup::Undo()
{
    uint32 numVerts = m_pHeightmap->m_VertCount.x * m_pHeightmap->m_VertCount.y;
    for( uint32 i=0; i<numVerts; i++ )
    {
        m_pHeightmap->m_Heights[i] = m_PreviousHeights[i];
    }

    // Regenerate the heightmap when undoing the last 'raise' command in the clump.
    //if( this->m_LinkedToPreviousCommandOnUndoStack == false )
    {
        m_pHeightmap->GenerateHeightmapMesh( false, false, true );
    }
}

EditorCommand* EditorCommand_Heightmap_FullBackup::Repeat()
{
    // Do nothing.

    return nullptr;
}
