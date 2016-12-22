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

#include "VoxelRayCast.h"

class VoxelChunk;

typedef unsigned int (*VoxelWorld_GenerateMap_CallbackFunction)(Vector3Int worldpos);

class VoxelWorld
{
    friend class VoxelChunk;

protected:
    CPPListHead m_pChunksFree;
    CPPListHead m_pChunksLoading;
    CPPListHead m_pChunksWaitingForMesh;
    CPPListHead m_pChunksVisible;

    Vector3Int m_WorldSize;
    Vector3Int m_ChunkSize;
    Vector3 m_BlockSize;

    Vector3Int m_WorldOffset;
    uint32* m_VoxelBlockEnabledBitsSingleAllocation;
    VoxelBlock* m_VoxelBlockSingleAllocation;
    VoxelChunk* m_VoxelChunkSingleAllocation;
    VoxelChunk** m_pActiveWorldChunkPtrs;
    unsigned int m_NumChunkPointersAllocated;

    MaterialDefinition* m_pMaterial;
    BufferDefinition* m_pSharedIndexBuffer;

    VoxelWorld_GenerateMap_CallbackFunction m_pMapGenCallbackFunc;

    Vector3Int m_MaxWorldSize;
    MyFileObject* m_pSaveFile;
    cJSON* m_jJSONSavedMapData; // TODO: replace this with custom solution for large worlds.

protected:
    void BuildSharedIndexBuffer();

    bool IsChunkActive(Vector3Int chunkpos);
    unsigned int GetActiveChunkArrayIndex(Vector3Int chunkpos);
    unsigned int GetActiveChunkArrayIndex(int chunkx, int chunky, int chunkz);
    VoxelChunk* GetActiveChunk(unsigned int arrayindex);
    VoxelChunk* GetActiveChunk(Vector3Int chunkpos);
    VoxelChunk* GetActiveChunk(int chunkx, int chunky, int chunkz);

    void PrepareChunk(Vector3Int chunkpos, uint32* pPreallocatedBlockEnabledBits, VoxelBlock* pBlocks);
    void ShiftChunk(Vector3Int to, Vector3Int from, bool isedgeblock);

    cJSON* GetJSONObjectForChunk(Vector3Int chunkpos);

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
    void SetMaterial(MaterialDefinition* pMaterial);
    MaterialDefinition* GetMaterial() { return m_pMaterial; }
    BufferDefinition* GetSharedIndexBuffer() { return m_pSharedIndexBuffer; }

    void SetSaveFile(MyFileObject* pFile);
    void SaveTheWorld();
    void SaveChunk(VoxelChunk* pChunk);

public:
    Vector3 GetBlockSize() { return m_BlockSize; }

    // Map generation
    void SetMapGenerationCallbackFunction(VoxelWorld_GenerateMap_CallbackFunction pFunc);
    VoxelWorld_GenerateMap_CallbackFunction GetMapGenerationCallbackFunction();

    // Space conversions
    Vector3Int GetWorldPosition(Vector3 scenepos);
    Vector3Int GetChunkPosition(Vector3Int worldpos);

    // Collision/Block queries
    VoxelBlock* GetBlock(Vector3Int worldpos);
    VoxelBlock* GetBlock(int worldx, int worldy, int worldz);
    VoxelChunk* GetChunkContainingWorldPosition(Vector3Int worldpos);
    bool IsBlockEnabled(Vector3Int worldpos, bool blockexistsifnotready = false);
    bool IsBlockEnabled(int worldx, int worldy, int worldz, bool blockexistsifnotready = false);
    bool IsBlockEnabledAroundLocation(Vector3 scenepos, float radius, bool blockexistsifnotready = false);
    float GetSceneYForNextBlockBelowPosition(Vector3 scenepos, float radius);
    bool RayCastSingleBlockFindFaceHit(Vector3Int worldpos, Vector3 startpos, Vector3 endpos, Vector3* pPoint, Vector3* pNormal);
    bool RayCast(Vector3 startpos, Vector3 endpos, float step, VoxelRayCastResult* pResult);

    void GetMouseRayBadly(Vector2 mousepos, Vector3* start, Vector3* end);

    // Add/Remove blocks
    void ChangeBlockState(Vector3Int worldpos, unsigned int type, bool enabled);
};

#endif //__VoxelWorld_H__
