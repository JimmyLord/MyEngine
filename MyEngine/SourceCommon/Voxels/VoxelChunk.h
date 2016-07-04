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
    void RebuildMesh();

    void AddToSceneGraph(void* pUserData);
    void RemoveFromSceneGraph();

    bool IsBlockEnabled(Vector3Int worldpos);
    bool IsBlockEnabled(int worldx, int worldy, int worldz);
};

#endif //__VoxelChunk_H__
