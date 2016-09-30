//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VoxelChunk_H__
#define __VoxelChunk_H__

#include "VoxelRayCast.h"

class VoxelBlock;
class MyMesh;

class VoxelChunk : public MyMesh
{
protected:
    VoxelWorld* m_pWorld;

    bool m_MapCreated;
    bool m_MeshOptimized;
    bool m_MapWasEdited;

    MyMatrix m_Transform;

    Vector3Int m_ChunkOffset;
    Vector3 m_BlockSize;
    Vector3Int m_ChunkSize;
    Vector2Int m_TextureTileCount;

    uint32* m_pBlockEnabledBits; // pointer to enough bits to store enabled flags for each block
    VoxelBlock* m_pBlocks;
    uint32 m_BlocksAllocated; // set to 0 if blocks were allocated elsewhere and passed in.

    SceneGraphObject* m_pSceneGraphObject;

    // Slightly faster lookup of nearby blocks.
    bool IsNearbyWorldBlockEnabled(unsigned int worldactivechunkarrayindex, int localx, int localy, int localz, bool blockexistsifnotready = false);

    // Internal file loading functions
    void CreateFromVoxelMeshFile();
    static void StaticOnFileFinishedLoadingVoxelMesh(void* pObjectPtr, MyFileObject* pFile) { ((VoxelChunk*)pObjectPtr)->OnFileFinishedLoadingVoxelMesh( pFile ); }
    void OnFileFinishedLoadingVoxelMesh(MyFileObject* pFile);

    virtual void ParseFile(); // MyMesh override

public:
    VoxelChunk();
    virtual ~VoxelChunk();

    void Initialize(VoxelWorld* world, Vector3 pos, Vector3Int chunkoffset, Vector3 blocksize);
    void SetBlockSize(Vector3 blocksize);
    void SetChunkSize(Vector3Int chunksize, uint32* pPreallocatedBlockEnabledBits = 0, VoxelBlock* pPreallocatedBlocks = 0);
    void SetTextureTileCount(Vector2Int tilecount);

    Vector3Int GetChunkOffset() { return m_ChunkOffset; }
    Vector3 GetBlockSize() { return m_BlockSize; }
    Vector3Int GetChunkSize() { return m_ChunkSize; }
    Vector2Int GetTextureTileCount() { return m_TextureTileCount; }

    // Load/Save ".myvoxelmesh" files
    cJSON* ExportAsJSONObject(bool exportforworld = false);
    void ImportFromJSONObject(cJSON* jVoxelMesh);

    // Map/Blocks
    static unsigned int DefaultGenerateMapFunc(Vector3Int worldpos);
    void GenerateMap();
    bool IsMapEdited() { return m_MapWasEdited; }
    bool IsInChunkSpace(Vector3Int worldpos);
    uint32* GetBlockEnabledBits() { return m_pBlockEnabledBits; }
    VoxelBlock* GetBlocks() { return m_pBlocks; }
    bool IsBlockEnabled(Vector3Int localpos, bool blockexistsifnotready = false);
    bool IsBlockEnabled(int localx, int localy, int localz, bool blockexistsifnotready = false);

    // Rendering
    bool RebuildMesh(unsigned int increment);

    void AddToSceneGraph(void* pUserData, MaterialDefinition* pMaterial);
    void OverrideSceneGraphObjectTransform(MyMatrix* pTransform);
    void RemoveFromSceneGraph();

    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex); // MyMesh override

    // Chunk state
    bool IsMapCreated() { return m_MapCreated; }
    bool IsMeshOptimized() { return m_MeshOptimized; }

    // Space conversions
    Vector3Int GetWorldPosition(Vector3 scenepos);
    VoxelBlock* GetBlockFromLocalPos(Vector3Int localpos);
    unsigned int GetBlockIndex(Vector3Int worldpos);

    // Collision/Block queries
    bool RayCastSingleBlockFindFaceHit(Vector3Int worldpos, Vector3 startpos, Vector3 endpos, Vector3* pPoint, Vector3* pNormal);
    bool RayCast(Vector3 startpos, Vector3 endpos, float step, VoxelRayCastResult* pResult);

    // Add/Remove blocks
    void ChangeBlockState(Vector3Int worldpos, unsigned int type, bool enabled);
};

#endif //__VoxelChunk_H__
