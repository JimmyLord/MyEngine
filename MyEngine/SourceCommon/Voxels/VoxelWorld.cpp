//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "VoxelBlock.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"

VoxelWorld::VoxelWorld()
{
}

VoxelWorld::~VoxelWorld()
{
    for( CPPListNode* pNode = m_pChunksFree.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        delete pChunk;
    }

    for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        delete pChunk;
    }
}

void VoxelWorld::Initialize()
{
    VoxelChunk* pChunk = MyNew VoxelChunk;
    m_pChunksFree.AddHead( pChunk );

    PrepareChunk();
}

void VoxelWorld::PrepareChunk()
{
    VoxelChunk* pChunk = (VoxelChunk*)m_pChunksFree.GetHead();
    if( pChunk == 0 )
    {
        LOGInfo( "VoxelWorld", "Attempting to prepare chunk, but none available\n" );
        return;
    }

    pChunk->Initialize( Vector3Int( 5, 5, 5 ) );
    pChunk->RebuildMesh();

    m_pChunksVisible.MoveTail( pChunk );
}

void VoxelWorld::UpdateVisibility(void* pUserData)
{
    // TODO: Update list of visible chunks

    // Add all visible chunks to scene graph
    for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;

        pChunk->AddToSceneGraph( pUserData );
    }
}
