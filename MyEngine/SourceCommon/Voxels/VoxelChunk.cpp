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

VoxelChunk::VoxelChunk()
{
    m_Transform.SetIdentity();
    m_ChunkSize.Set( 0, 0, 0 );
    m_pSceneGraphObject = 0;

    m_pBlocks = 0;
    m_pMesh = 0;
}

VoxelChunk::~VoxelChunk()
{
    // remove from cpplist.
    this->Remove();

    RemoveFromSceneGraph();

    delete[] m_pBlocks;
    m_pMesh->Release();
}

void VoxelChunk::Initialize(Vector3Int chunksize)
{
    MyAssert( m_pMesh == 0 );

    m_ChunkSize = chunksize;

    m_pBlocks = MyNew VoxelBlock[chunksize.x * chunksize.y * chunksize.z];

    // Create a mesh.
    {
        m_pMesh = MyNew MyMesh();

        int numblocks = chunksize.x * chunksize.y * chunksize.z;
        int maxverts = 6*4*numblocks;
        int maxindices = 6*2*3*numblocks;
        int indexbytes = 2; // TODO: take out hard-coded unsigned short as index type
        //int indexbytes = 1;
        //if( maxverts > 256 )
        //    indexbytes = 2;
        //if( maxverts > 256*256 )
        //    indexbytes = 4;

        VertexFormat_Dynamic_Desc* pVertFormat = g_pVertexFormatManager->GetDynamicVertexFormat( 1, true, false, false, false, 0 );
        m_pMesh->CreateBuffers( pVertFormat, maxverts, indexbytes, maxindices, true );

        RebuildMesh();
    }
}

void VoxelChunk::RebuildMesh()
{
    MyAssert( m_pBlocks );
    MyAssert( m_pMesh );

    // Loop through blocks and add a cube for each one that's enabled
    // TODO: merge outer faces, eliminate inner faces.
    {
        MyAssert( m_pMesh->GetStride( 0 ) == (12 + 8 + 12) ); // XYZ + UV + NORM

        Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_pMesh->GetVerts( true );
        unsigned short* pIndices = (unsigned short*)m_pMesh->GetIndices( true );

        int count = 0;
        for( int z=0; z<m_ChunkSize.z; z++ )
        {
            for( int y=0; y<m_ChunkSize.y; y++ )
            {
                for( int x=0; x<m_ChunkSize.x; x++ )
                {
                    Vector3 boxsize( 1, 1, 1 );

                    float xleft   = x*boxsize.x - boxsize.x/2;
                    float xright  = x*boxsize.x + boxsize.x/2;
                    float ytop    = y*boxsize.y + boxsize.y/2;
                    float ybottom = y*boxsize.y - boxsize.y/2;
                    float zfront  = z*boxsize.z - boxsize.z/2;
                    float zback   = z*boxsize.z + boxsize.z/2;

                    int side;

                    float uleft   =  0.0f / 128.0f;
                    float uright  = 32.0f / 128.0f;
                    float vtop    =  0.0f / 128.0f;
                    float vbottom = 32.0f / 128.0f;

                    // front
                    side = 0;
                    pVerts[4*side + 0].pos.x = xleft;  pVerts[4*side + 0].pos.y = ytop;    pVerts[4*side + 0].pos.z = zfront;  pVerts[4*side + 0].uv.x = uleft;  pVerts[4*side + 0].uv.y = vtop;    // upper left
                    pVerts[4*side + 1].pos.x = xright; pVerts[4*side + 1].pos.y = ytop;    pVerts[4*side + 1].pos.z = zfront;  pVerts[4*side + 1].uv.x = uright; pVerts[4*side + 1].uv.y = vtop;    // upper right
                    pVerts[4*side + 2].pos.x = xleft;  pVerts[4*side + 2].pos.y = ybottom; pVerts[4*side + 2].pos.z = zfront;  pVerts[4*side + 2].uv.x = uleft;  pVerts[4*side + 2].uv.y = vbottom; // lower left
                    pVerts[4*side + 3].pos.x = xright; pVerts[4*side + 3].pos.y = ybottom; pVerts[4*side + 3].pos.z = zfront;  pVerts[4*side + 3].uv.x = uright; pVerts[4*side + 3].uv.y = vbottom; // lower right
                    for( int i=0; i<4; i++ )
                        pVerts[4*side + i].normal.Set( 0, 0, 1 );

                    // back
                    side = 1;
                    pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
                    pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
                    pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
                    pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
                    for( int i=0; i<4; i++ )
                        pVerts[4*side + i].normal.Set( 0, 0, -1 );

                    // right
                    side = 2;
                    pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
                    pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
                    pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
                    pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
                    for( int i=0; i<4; i++ )
                        pVerts[4*side + i].normal.Set( 1, 0, 0 );

                    // left
                    side = 3;
                    pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
                    pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
                    pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
                    pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
                    for( int i=0; i<4; i++ )
                        pVerts[4*side + i].normal.Set( -1, 0, 0 );

                    // bottom
                    side = 4;
                    pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv = pVerts[0].uv;
                    pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv = pVerts[1].uv;
                    pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
                    pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
                    for( int i=0; i<4; i++ )
                        pVerts[4*side + i].normal.Set( 0, -1, 0 );

                    // top
                    side = 5;
                    pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
                    pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
                    pVerts[4*side + 2] = pVerts[0];  pVerts[4*side + 2].uv = pVerts[2].uv;
                    pVerts[4*side + 3] = pVerts[1];  pVerts[4*side + 3].uv = pVerts[3].uv;
                    for( int i=0; i<4; i++ )
                        pVerts[4*side + i].normal.Set( 0, 1, 0 );

                    // Set up indices
                    // TODO: take out hard-coded unsigned short as index type
                    for( side=0; side<6; side++ )
                    {
                        pIndices[count*36 + 6*side + 0] = (unsigned short)(count*24 + 4*side + 0);
                        pIndices[count*36 + 6*side + 1] = (unsigned short)(count*24 + 4*side + 1);
                        pIndices[count*36 + 6*side + 2] = (unsigned short)(count*24 + 4*side + 2);
                        pIndices[count*36 + 6*side + 3] = (unsigned short)(count*24 + 4*side + 1);
                        pIndices[count*36 + 6*side + 4] = (unsigned short)(count*24 + 4*side + 3);
                        pIndices[count*36 + 6*side + 5] = (unsigned short)(count*24 + 4*side + 2);
                    }

                    pVerts += 24;

                    count++;
                }
            }
        }

        //Vector3 center( (xleft + xright) / 2, (ytop + ybottom) / 2, (zfront + zback) / 2 );
        //m_AABounds.Set( center, Vector3(boxw/2, boxh/2, boxh/2) );
    }
}

void VoxelChunk::AddToSceneGraph(void* pUserData)
{
    if( m_pSceneGraphObject != 0 )
        return;

    MaterialDefinition* pMaterial = g_pMaterialManager->FindMaterialByFilename( "Data/Voxels/Tiles.mymaterial" );

    m_pSceneGraphObject = g_pComponentSystemManager->m_pSceneGraph->AddObject(
        &m_Transform, m_pMesh, m_pMesh->m_SubmeshList[0],
        pMaterial, GL_TRIANGLES, 0, SceneGraphFlag_Opaque, 0xFFFFFFFF, pUserData );
}

void VoxelChunk::RemoveFromSceneGraph()
{
    if( m_pSceneGraphObject == 0 )
        return;

    g_pComponentSystemManager->m_pSceneGraph->RemoveObject( m_pSceneGraphObject );
    m_pSceneGraphObject = 0;
}
