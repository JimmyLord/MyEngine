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
    CPPListHead m_pChunksLoading;
    CPPListHead m_pChunksVisible;

    Vector3Int m_WorldSize;
    Vector3Int m_ChunkSize;
    Vector3 m_BlockSize;

    Vector3Int m_WorldOffset;
    VoxelChunk** m_pActiveWorldChunkPtrs;
    unsigned int m_NumChunkPointersAllocated;

protected:
    unsigned int GetActiveChunkArrayIndex(Vector3Int chunkpos);
    unsigned int GetActiveChunkArrayIndex(int chunkx, int chunky, int chunkz);
    VoxelChunk* GetActiveChunk(unsigned int arrayindex);
    VoxelChunk* GetActiveChunk(Vector3Int chunkpos);
    VoxelChunk* GetActiveChunk(int chunkx, int chunky, int chunkz);

    void PrepareChunk(Vector3Int chunkpos);
    void ShiftChunk(Vector3Int to, Vector3Int from, bool isedgeblock);

public:
    VoxelWorld();
    virtual ~VoxelWorld();

    void Tick(double timepassed);

    void Initialize(Vector3Int visibleworldsize);

    Vector3Int GetWorldSize() { return m_WorldSize; }
    void SetWorldSize(Vector3Int visibleworldsize);
    void SetWorldCenter(Vector3 scenepos);
    void SetWorldCenter(Vector3Int newworldcenter);

    void UpdateVisibility(void* pUserData);

public:
    Vector3 GetBlockSize() { return m_BlockSize; }

    // Space conversions
    Vector3Int GetWorldPosition(Vector3 scenepos);
    Vector3Int GetChunkPosition(Vector3Int worldpos);

    // Collision/Block queries
    bool IsBlockEnabled(Vector3Int pos);
    bool IsBlockEnabled(int x, int y, int z);
    float GetSceneYForNextBlockBelowPosition(Vector3 scenepos, float radius);
};

#endif //__VoxelWorld_H__
