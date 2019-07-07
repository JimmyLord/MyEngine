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
    m_RaiseRadius = radius;
}

EditorCommand_Heightmap_Raise::~EditorCommand_Heightmap_Raise()
{
}

void EditorCommand_Heightmap_Raise::Do()
{
    m_pHeightmap->Tool_Raise( m_Position, m_RaiseAmount, m_RaiseRadius, m_BrushSoftness, true );

    // Regenerate the heightmap normals when redoing the last 'raise' command in the clump.
    if( this->m_LinkedToNextCommandOnRedoStack == false )
    {
        m_pHeightmap->GenerateHeightmapMesh( false, false, true );
    }
}

void EditorCommand_Heightmap_Raise::Undo()
{
    m_pHeightmap->Tool_Raise( m_Position, -m_RaiseAmount, m_RaiseRadius, m_BrushSoftness, true );

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
