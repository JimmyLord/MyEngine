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

class VoxelBlock;
class MyMesh;

class VoxelChunk : public CPPListNode
{
protected:
    VoxelWorld* m_pWorld;

    bool m_MapCreated;
    bool m_MeshOptimized;

    MyMatrix m_Transform;
    Vector3Int m_ChunkSize;
    Vector3Int m_ChunkOffset;

    VoxelBlock* m_pBlocks;
    MyMesh* m_pMesh;

    SceneGraphObject* m_pSceneGraphObject;

public:
    VoxelChunk();
    virtual ~VoxelChunk();

    void Initialize(VoxelWorld* world, Vector3 pos, Vector3Int chunksize, Vector3Int chunkoffset);

    // Map/Blocks
    static unsigned int DefaultGenerateMapFunc(Vector3Int worldpos);
    void GenerateMap();
    bool IsBlockEnabled(Vector3Int worldpos, bool blockexistsifnotready = false);
    bool IsBlockEnabled(int worldx, int worldy, int worldz, bool blockexistsifnotready = false);

    // Rendering
    void RebuildMesh();

    void AddToSceneGraph(void* pUserData, MaterialDefinition* pMaterial);
    void RemoveFromSceneGraph();

    void SetMaterial(MaterialDefinition* pMaterial);

    // Chunk state
    bool IsMapCreated() { return m_MapCreated; }
    bool IsMeshOptimized() { return m_MeshOptimized; }

    // Space conversions
    Vector3Int GetChunkSize() { return m_ChunkSize; }
    Vector3Int GetChunkOffset() { return m_ChunkOffset; }
    VoxelBlock* GetBlockFromLocalPos(Vector3Int localpos);
    unsigned int GetBlockIndex(Vector3Int worldpos);

    // Add/Remove blocks
    void ChangeBlockState(Vector3Int, bool enabled);
};

#endif //__VoxelChunk_H__
