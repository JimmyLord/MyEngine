//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "VoxelBlock.h"
#include "VoxelChunk.h"
#include "VoxelJobs.h"
#include "VoxelWorld.h"

//====================================================================================================
// VoxelChunkGenerator
//====================================================================================================

VoxelChunkGenerator::VoxelChunkGenerator()
{
    m_pWorld = 0;
    m_pChunk = 0;
}

VoxelChunkGenerator::~VoxelChunkGenerator()
{
}

void VoxelChunkGenerator::DoWork()
{
    //LOGInfo( "Voxel Chunk Generator", "Started generating chunk\n" );

    m_pChunk->GenerateMap();

    //LOGInfo( "Voxel Chunk Generator", "Finished generating chunk\n" );
}

//====================================================================================================
// VoxelMeshBuilder
//====================================================================================================

VoxelMeshBuilder::VoxelMeshBuilder()
{
    m_pWorld = 0;
    m_pChunk = 0;

    m_pVerts = 0;
    m_VertCount = 0;

    m_TimeToBuild = 0;
}

VoxelMeshBuilder::~VoxelMeshBuilder()
{
}

void VoxelMeshBuilder::DoWork()
{
    //LOGInfo( "Voxel Mesh Builder", "Started creating mesh\n" );

    m_pChunk->RebuildMesh( 1, m_pVerts, &m_VertCount, &m_TimeToBuild );

    //LOGInfo( "Voxel Mesh Builder", "Finished creating mesh\n" );
}
