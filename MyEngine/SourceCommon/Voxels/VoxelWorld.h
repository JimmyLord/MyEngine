//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VoxelWorld_H__
#define __VoxelWorld_H__

class VoxelChunk;

class VoxelWorld
{
protected:
    CPPListHead m_pChunksFree;
    CPPListHead m_pChunksVisible;

    Vector3Int m_WorldSize;
    Vector3Int m_ChunkSize;
    Vector3 m_BlockSize;

    VoxelChunk** m_pWorldChunkPtrs;
    unsigned int m_NumChunkPointersAllocated;

public:
    VoxelWorld();
    virtual ~VoxelWorld();

    void Initialize(Vector3Int worldsize);
    void SetWorldSize(Vector3Int worldsize);

    void PrepareChunk(Vector3 pos, Vector3Int size);

    Vector3 GetBlockSize() { return m_BlockSize; }

    void UpdateVisibility(void* pUserData);
};

#endif //__VoxelWorld_H__
