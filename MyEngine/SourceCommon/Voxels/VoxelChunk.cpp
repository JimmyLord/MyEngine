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

VoxelChunk::VoxelChunk()
{
    m_pWorld = 0;

    m_MapCreated = false;
    m_MeshOptimized = false;

    m_Transform.SetIdentity();
    m_BlockSize.Set( 0, 0, 0 );
    m_ChunkSize.Set( 0, 0, 0 );
    m_ChunkOffset.Set( 0, 0, 0 );
    m_pSceneGraphObject = 0;

    m_pBlocks = 0;
}

VoxelChunk::~VoxelChunk()
{
    RemoveFromSceneGraph();

    delete[] m_pBlocks;
}

void VoxelChunk::Initialize(VoxelWorld* world, Vector3 pos, Vector3Int chunksize, Vector3Int chunkoffset, Vector3 blocksize)
{
    m_pWorld = world;

    m_Transform.SetIdentity();
    m_Transform.SetTranslation( pos );

    m_BlockSize = blocksize;
    m_ChunkSize = chunksize;
    m_ChunkOffset = chunkoffset;

    if( m_pBlocks == 0 )
        m_pBlocks = MyNew VoxelBlock[chunksize.x * chunksize.y * chunksize.z];

    m_MapCreated = false;

    // if this chunk isn't part of a world, then it's a manually created mesh, so set map is created for now.
    if( m_pWorld == 0 )
        m_MapCreated = true;

    // Set up the vertex format for this mesh.
    if( m_SubmeshList.Length() == 0 )
    {
        int indexbytes = 2; // TODO: take out hard-coded unsigned short as index type

        VertexFormat_Dynamic_Desc* pVertFormat = g_pVertexFormatManager->GetDynamicVertexFormat( 1, true, false, false, false, 0 );
        CreateBuffers( pVertFormat, 0, indexbytes, 0, true );
    }
}

// ============================================================================================================================
// Map/Blocks
// ============================================================================================================================
unsigned int VoxelChunk::DefaultGenerateMapFunc(Vector3Int worldpos)
{
    bool enabled = false;

    if( 0 )
    {
        float freq = 1/50.0f;

        double value = SimplexNoise( worldpos.x * freq, worldpos.y * freq, worldpos.z * freq );
                
        enabled = value > 0.0f;
    }
    else
    {
        float freq = 1/50.0f;

        double value = SimplexNoise( worldpos.x * freq, worldpos.z * freq );

        //// shift -1 to 1 into range of 0.5 to 1.
        //double shiftedvalue = value * 0.25 + 0.75;

        //// bottom half solid, top half hilly.
        //enabled = ((float)worldpos.y / worldblocksize.y) < shiftedvalue;

        //below 0 is solid
        //above 0 is hilly -> hills are -20 to 20 blocks tall
        enabled = value * 20 > worldpos.y ? true : false;
    }

    int blocktype = 1;
    if( worldpos.y > 8 )
        blocktype = 2;
    if( worldpos.y > 12 )
        blocktype = 4;
    if( worldpos.y < -15 )
        blocktype = 3;

    if( enabled == false )
        blocktype = 0;

    return blocktype;
}

void VoxelChunk::GenerateMap()
{
    MyAssert( m_pWorld != 0 );
    if( m_pWorld == 0 )
        return;

    //Vector3Int worldsize = m_pWorld->GetWorldSize();
    //Vector3Int worldblocksize = worldsize.MultiplyComponents( m_ChunkSize );
    Vector3Int chunksize = GetChunkSize();
    Vector3Int chunkoffset = GetChunkOffset();

    for( int z=0; z<chunksize.z; z++ )
    {
        for( int y=0; y<chunksize.y; y++ )
        {
            for( int x=0; x<chunksize.x; x++ )
            {
                unsigned int blocktype;
                Vector3Int worldpos = Vector3Int( chunkoffset.x + x, chunkoffset.y + y, chunkoffset.z + z );

                VoxelWorld_GenerateMap_CallbackFunction pFunc = m_pWorld->GetMapGenerationCallbackFunction();

                if( pFunc != 0 )
                    blocktype = pFunc( worldpos );
                else
                    blocktype = VoxelChunk::DefaultGenerateMapFunc( worldpos );

                VoxelBlock* pBlock = GetBlockFromLocalPos( Vector3Int( x, y, z ) );
                pBlock->SetEnabled( blocktype > 0 ? true : false );
                pBlock->SetBlockType( blocktype );
            }
        }
    }

    m_MapCreated = true;
}

bool VoxelChunk::IsInChunkSpace(Vector3Int worldpos)
{
    Vector3Int localpos = worldpos - m_ChunkOffset;

    if( localpos.x >= 0 && localpos.x < m_ChunkSize.x &&
        localpos.y >= 0 && localpos.y < m_ChunkSize.y &&
        localpos.z >= 0 && localpos.z < m_ChunkSize.z )
    {
        return true;
    }

    return false;
}

bool VoxelChunk::IsBlockEnabled(Vector3Int localpos, bool blockexistsifnotready)
{
    return IsBlockEnabled( localpos.x, localpos.y, localpos.z, blockexistsifnotready );
}

bool VoxelChunk::IsBlockEnabled(int localx, int localy, int localz, bool blockexistsifnotready)
{
    // If the block haven't been setup yet, then return false, as if blocks aren't there.
    if( m_MapCreated == false )
        return blockexistsifnotready;

    MyAssert( localx >= 0 && localx < m_ChunkSize.x );
    MyAssert( localy >= 0 && localy < m_ChunkSize.y );
    MyAssert( localz >= 0 && localz < m_ChunkSize.z );

    VoxelBlock* pBlock = &m_pBlocks[localz * m_ChunkSize.y * m_ChunkSize.x + localy * m_ChunkSize.x + localx];

    return pBlock->IsEnabled();
}

// ============================================================================================================================
// Rendering
// ============================================================================================================================
void VoxelChunk::RebuildMesh(unsigned int increment)
{
    MyAssert( m_pBlocks );

    // Loop through blocks and add a cube for each one that's enabled
    // TODO: merge outer faces, eliminate inner faces.
    {
        MyAssert( GetStride( 0 ) == (12 + 8 + 12) ); // XYZ + UV + NORM

        int numblocks = m_ChunkSize.x * m_ChunkSize.y * m_ChunkSize.z;
        int maxverts = 6*4*numblocks;
        int maxindices = 6*2*3*numblocks;

        // TODO: take out hard-coded unsigned short as index type
        int vertbuffersize = maxverts * GetStride( 0 );
        int indexbuffersize = maxindices * 2;

        // TODO: fill buffer without storing a local copy in main ram.
        //Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_pMesh->GetVerts( true );
        //unsigned short* pIndices = (unsigned short*)m_pMesh->GetIndices( true );
        MyStackAllocator::MyStackPointer memstart = g_pEngineCore->m_SingleFrameMemoryStack.GetCurrentLocation();
        Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)g_pEngineCore->m_SingleFrameMemoryStack.AllocateBlock( vertbuffersize );
        unsigned short* pIndices = (unsigned short*)g_pEngineCore->m_SingleFrameMemoryStack.AllocateBlock( indexbuffersize );

        Vector3 minextents( FLT_MAX, FLT_MAX, FLT_MAX );
        Vector3 maxextents( -FLT_MAX, -FLT_MAX, -FLT_MAX );

        // pVerts gets advanced by code below, so store a copy.
        Vertex_XYZUVNorm* pActualVerts = pVerts;
        unsigned short* pActualIndices = pIndices;

        //  block type          1, 2, 3, 4, 5, 6
        int TileTops_Col[] =  { 0, 1, 2, 3, 0, 0 };
        int TileTops_Row[] =  { 0, 0, 0, 0, 2, 3 };
        int TileSides_Col[] = { 0, 1, 2, 3, 0, 0 };
        int TileSides_Row[] = { 1, 1, 1, 1, 2, 3 };

        int vertcount = 0;
        int indexcount = 0;
        int count = 0;
        for( int z=0; z<m_ChunkSize.z; z++ )
        {
            for( int y=0; y<m_ChunkSize.y; y++ )
            {
                for( int x=0; x<m_ChunkSize.x; x++ )
                {
                    VoxelBlock* pBlock = &m_pBlocks[z * m_ChunkSize.y * m_ChunkSize.x + y * m_ChunkSize.x + x];
                    if( pBlock->IsEnabled() == false )
                        continue;

                    int tileindex = pBlock->GetBlockType() - 1;

                    float xleft   = x*m_BlockSize.x - m_BlockSize.x/2;
                    float xright  = x*m_BlockSize.x + m_BlockSize.x/2;
                    float ybottom = y*m_BlockSize.y - m_BlockSize.y/2;
                    float ytop    = y*m_BlockSize.y + m_BlockSize.y/2;
                    float zfront  = z*m_BlockSize.z - m_BlockSize.z/2;
                    float zback   = z*m_BlockSize.z + m_BlockSize.z/2;

                    if( xleft   < minextents.x ) minextents.x = xleft;
                    if( xright  > maxextents.x ) maxextents.x = xright;
                    if( ybottom < minextents.y ) minextents.y = ybottom;
                    if( ytop    > maxextents.y ) maxextents.y = ytop;
                    if( zfront  < minextents.z ) minextents.z = zfront;
                    if( zback   > maxextents.z ) maxextents.z = zback;

                    int c = 0;//rand()%3;
                    int r = 1;//rand()%3;

                    float uleft   = (32.0f*(TileSides_Col[tileindex]+0)) / 128.0f;
                    float uright  = (32.0f*(TileSides_Col[tileindex]+1)) / 128.0f;
                    float vtop    = (32.0f*(TileSides_Row[tileindex]+0)) / 128.0f;
                    float vbottom = (32.0f*(TileSides_Row[tileindex]+1)) / 128.0f;

                    Vector3Int worldpos( m_ChunkOffset.x+x, m_ChunkOffset.y+y, m_ChunkOffset.z+z );

                    // front
                    if( z == 0 && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y, worldpos.z-1 ) )
                    {
                    }
                    else if( z == 0 || m_pBlocks[(z-1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;    // upper left
                        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;    // upper right
                        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom; // lower left
                        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom; // lower right
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, 0, -1 );
                        for( int i=0; i<6; i++ )
                            pIndices[i] = (unsigned short)(vertcount + g_SpriteVertexIndices[i]);
                        pVerts += 4;
                        vertcount += 4;
                        pIndices += 6;
                        indexcount += 6;
                    }

                    // back
                    if( z == (m_ChunkSize.z-1) && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y, worldpos.z+1 ) )
                    {
                    }
                    else if( z == (m_ChunkSize.z-1) || m_pBlocks[(z+1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos.x = xright; pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zback;  pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos.x = xleft;  pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zback;  pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos.x = xright; pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zback;  pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos.x = xleft;  pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zback;  pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, 0, 1 );
                        for( int i=0; i<6; i++ )
                            pIndices[i] = (unsigned short)(vertcount + g_SpriteVertexIndices[i]);
                        pVerts += 4;
                        vertcount += 4;
                        pIndices += 6;
                        indexcount += 6;
                    }

                    // left
                    if( x == 0 && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x-1, worldpos.y, worldpos.z ) )
                    {
                    }
                    else if( x == 0 || m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x-1)].IsEnabled() == false )
                    {
                        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zback;  pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos.x = xleft;  pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zback;  pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos.x = xleft;  pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( -1, 0, 0 );
                        for( int i=0; i<6; i++ )
                            pIndices[i] = (unsigned short)(vertcount + g_SpriteVertexIndices[i]);
                        pVerts += 4;
                        vertcount += 4;
                        pIndices += 6;
                        indexcount += 6;
                    }

                    // right
                    if( x == (m_ChunkSize.x-1) && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x+1, worldpos.y, worldpos.z ) )
                    {
                    }
                    else if( x == (m_ChunkSize.x-1) || m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x+1)].IsEnabled() == false )
                    {
                        pVerts[0].pos.x = xright; pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zback;  pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos.x = xright; pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zback;  pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 1, 0, 0 );
                        for( int i=0; i<6; i++ )
                            pIndices[i] = (unsigned short)(vertcount + g_SpriteVertexIndices[i]);
                        pVerts += 4;
                        vertcount += 4;
                        pIndices += 6;
                        indexcount += 6;
                    }

                    // bottom
                    if( y == 0 && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y-1, worldpos.z ) )
                    {
                    }
                    else if( y == 0 || m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y-1) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ybottom; pVerts[0].pos.z = zfront; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos.x = xright; pVerts[1].pos.y = ybottom; pVerts[1].pos.z = zfront; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zback;  pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zback;  pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, -1, 0 );
                        for( int i=0; i<6; i++ )
                            pIndices[i] = (unsigned short)(vertcount + g_SpriteVertexIndices[i]);
                        pVerts += 4;
                        vertcount += 4;
                        pIndices += 6;
                        indexcount += 6;
                    }

                    c = 3;//rand()%3;
                    r = 0;//rand()%3;

                    uleft   = (32.0f*(TileTops_Col[tileindex]+0)) / 128.0f;
                    uright  = (32.0f*(TileTops_Col[tileindex]+1)) / 128.0f;
                    vtop    = (32.0f*(TileTops_Row[tileindex]+0)) / 128.0f;
                    vbottom = (32.0f*(TileTops_Row[tileindex]+1)) / 128.0f;

                    // top
                    if( y == (m_ChunkSize.y-1) && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y+1, worldpos.z ) )
                    {
                    }
                    else if( y == (m_ChunkSize.y-1) || m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y+1) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zback;  pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zback;  pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ytop;    pVerts[2].pos.z = zfront; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos.x = xright; pVerts[3].pos.y = ytop;    pVerts[3].pos.z = zfront; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, 1, 0 );
                        for( int i=0; i<6; i++ )
                            pIndices[i] = (unsigned short)(vertcount + g_SpriteVertexIndices[i]);
                        pVerts += 4;
                        vertcount += 4;
                        pIndices += 6;
                        indexcount += 6;
                    }

                    if( vertcount > maxverts - 50 )
                        break;

                    count++;
                }
                
                if( vertcount > maxverts - 50 )
                    break;
            }
            
            if( vertcount > maxverts - 50 )
                break;
        }

        if( count > 0 )
        {
            // Copy the data into the OpenGL buffers.
            if( vertcount > 0 )
            {
                if( vertcount > 256*256 )
                {
                    LOGInfo( "VoxelWorld", "Too many verts needed in chunk - %d\n", maxverts );
                }

                m_SubmeshList[0]->m_pVertexBuffer->TempBufferData( vertcount * GetStride( 0 ), pActualVerts );
                m_SubmeshList[0]->m_pIndexBuffer->TempBufferData( indexcount * 2, pActualIndices );
            }

            m_SubmeshList[0]->m_NumIndicesToDraw = indexcount;
            //LOGInfo( "VoxelChunk", "Num indices: %d\n", indexcount );

            Vector3 center = (minextents + maxextents) / 2;
            Vector3 extents = (maxextents - minextents) / 2;
            GetBounds()->Set( center, extents );
        }
        else
        {
            m_SubmeshList[0]->m_NumIndicesToDraw = 0;

            Vector3 center( 0, 0, 0 );
            Vector3 extents( 0, 0, 0 );
            GetBounds()->Set( center, extents );
        }

        g_pEngineCore->m_SingleFrameMemoryStack.RewindStack( memstart );
    }

    // if any of the neighbouring chunks wasn't ready, then we likely created extra faces to close outside walls.
    // rebuild the mesh to get rid of those sides.
    if( m_pWorld )
    {
        m_MeshOptimized = true;

        Vector3Int chunkpos = m_pWorld->GetChunkPosition( m_ChunkOffset );

        for( int i=0; i<6; i++ )
        {
            Vector3Int neighbourchunkpos = chunkpos;

            if( i == 0 ) chunkpos.x += 1;
            if( i == 1 ) chunkpos.x -= 1;
            if( i == 2 ) chunkpos.y += 1;
            if( i == 3 ) chunkpos.y -= 1;
            if( i == 4 ) chunkpos.z += 1;
            if( i == 5 ) chunkpos.z -= 1;

            if( m_pWorld->IsChunkActive( neighbourchunkpos ) )
            {
                VoxelChunk* pNeighbourChunk = m_pWorld->GetActiveChunk( neighbourchunkpos );
                if( pNeighbourChunk && pNeighbourChunk->m_MapCreated == false )
                {
                    m_MeshOptimized = false;
                    break;
                }
            }
        }        
    }

    m_MeshReady = true;
}

void VoxelChunk::AddToSceneGraph(void* pUserData, MaterialDefinition* pMaterial)
{
    if( m_pSceneGraphObject != 0 )
        return;

    m_pSceneGraphObject = g_pComponentSystemManager->m_pSceneGraph->AddObject(
        &m_Transform, this, m_SubmeshList[0],
        pMaterial, GL_TRIANGLES, 0, SceneGraphFlag_Opaque, 1, pUserData );
}

void VoxelChunk::OverrideSceneGraphObjectTransform(MyMatrix* pTransform)
{
    MyAssert( m_pSceneGraphObject != 0 );
    if( m_pSceneGraphObject == 0 )
        return;

    m_pSceneGraphObject->m_pTransform = pTransform;
}

void VoxelChunk::RemoveFromSceneGraph()
{
    if( m_pSceneGraphObject == 0 )
        return;

    g_pComponentSystemManager->m_pSceneGraph->RemoveObject( m_pSceneGraphObject );
    m_pSceneGraphObject = 0;
}

void VoxelChunk::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    if( m_pSceneGraphObject == 0 )
        return;

    MyMesh::SetMaterial( pMaterial, 0 );

    m_pSceneGraphObject->m_pMaterial = pMaterial;
}

// ============================================================================================================================
// Space conversions
// ============================================================================================================================
Vector3Int VoxelChunk::GetWorldPosition(Vector3 scenepos)
{
    Vector3Int worldpos;

    worldpos.x = (int)floor( (scenepos.x+m_BlockSize.x/2) / m_BlockSize.x );
    worldpos.y = (int)floor( (scenepos.y+m_BlockSize.y/2) / m_BlockSize.y );
    worldpos.z = (int)floor( (scenepos.z+m_BlockSize.z/2) / m_BlockSize.z );

    return worldpos;
}

VoxelBlock* VoxelChunk::GetBlockFromLocalPos(Vector3Int localpos)
{
    VoxelBlock* pBlock = &m_pBlocks[localpos.z * m_ChunkSize.y * m_ChunkSize.x + localpos.y * m_ChunkSize.x + localpos.x];

    return pBlock;
}

unsigned int VoxelChunk::GetBlockIndex(Vector3Int worldpos)
{
    Vector3Int localpos = worldpos - m_ChunkOffset;

    unsigned int index = localpos.z * m_ChunkSize.y * m_ChunkSize.x + localpos.y * m_ChunkSize.x + localpos.x;

    MyAssert( index < (unsigned int)m_ChunkSize.x * (unsigned int)m_ChunkSize.y * (unsigned int)m_ChunkSize.z );

    return index;
}

// ============================================================================================================================
// Collision/Block queries
// ============================================================================================================================
bool VoxelChunk::RayCastSingleBlockFindFaceHit(Vector3Int worldpos, Vector3 startpos, Vector3 endpos, Vector3* pPoint, Vector3* pNormal)
{
    MyAssert( pPoint != 0 && pNormal != 0 );
    
    // TODO: Find the normal for the side of the block that was hit.
    startpos.x -= worldpos.x * m_BlockSize.x;
    startpos.y -= worldpos.y * m_BlockSize.y;
    startpos.z -= worldpos.z * m_BlockSize.z;

    endpos.x -= worldpos.x * m_BlockSize.x;
    endpos.y -= worldpos.y * m_BlockSize.y;
    endpos.z -= worldpos.z * m_BlockSize.z;

    float shortestlength = FLT_MAX;
    Plane plane;
    Vector3 result;

    Vector3 bs = m_BlockSize;
    Vector3 halfbs = m_BlockSize/2;

    for( int i=0; i<6; i++ )
    {
        switch( i )
        {
        case 0: plane.Set( Vector3(-bs.x,0,0), Vector3(-halfbs.x, 0, 0) ); break;
        case 1: plane.Set( Vector3( bs.x,0,0), Vector3( halfbs.x, 0, 0) ); break;
        case 2: plane.Set( Vector3(0,-bs.y,0), Vector3(0, -halfbs.y, 0) ); break;
        case 3: plane.Set( Vector3(0, bs.y,0), Vector3(0,  halfbs.y, 0) ); break;
        case 4: plane.Set( Vector3(0,0,-bs.z), Vector3(0, 0, -halfbs.z) ); break;
        case 5: plane.Set( Vector3(0,0, bs.z), Vector3(0, 0,  halfbs.z) ); break;
        }

        plane.IntersectRay( startpos, endpos, &result );

        if( ( i >=0 && i <= 1 && result.y > -halfbs.y && result.y < halfbs.y && result.z > -halfbs.z && result.z < halfbs.z ) ||
            ( i >=2 && i <= 3 && result.x > -halfbs.x && result.x < halfbs.x && result.z > -halfbs.z && result.z < halfbs.z ) ||
            ( i >=4 && i <= 5 && result.x > -halfbs.x && result.x < halfbs.x && result.y > -halfbs.y && result.y < halfbs.y ) )
        {
            float len = (result - startpos).LengthSquared();
            if( len < shortestlength )
            {
                shortestlength = len;

                // Return face normal hit.
                *pPoint = result;
                *pNormal = plane.m_Normal;
            }
        }
    }

    //LOGInfo( "VoxelWorld", "Normal: %0.0f,%0.0f,%0.0f\n", pNormal->x, pNormal->y, pNormal->z );
    return true;
}

bool VoxelChunk::RayCast(Vector3 startpos, Vector3 endpos, float step, VoxelRayCastResult* pResult)
{
    // startpos and endpos are expected to be in chunk space.

    // Lazy raycast, will pass through blocks if step too big, will always pass through corners.

    // Init some vars and figure out the length and direction of the ray we're casting.
    Vector3 currentchunkspacepos = startpos;
    Vector3 dir = endpos - startpos;
    float len = dir.Length();
    dir.Normalize();

    // Init last worldpos to something that isn't the current world pos.
    Vector3Int lastlocalpos = GetWorldPosition( currentchunkspacepos ) + Vector3Int( 1, 1, 1 );
    
    while( true )
    {
        Vector3Int localpos = GetWorldPosition( currentchunkspacepos );

        if( IsInChunkSpace( localpos ) )
        {
            // If the worldpos is different than the previous loop, check for a block.
            if( localpos != lastlocalpos && IsBlockEnabled( localpos ) == true )
            {
                if( pResult )
                {
                    pResult->m_Hit = true;
                    pResult->m_BlockWorldPosition = localpos;

                    // Find the normal for the side of the block that was hit.
                    RayCastSingleBlockFindFaceHit( localpos, startpos, endpos, &pResult->m_BlockFacePoint, &pResult->m_BlockFaceNormal );
                }
                return true;
            }
        }

        // Store the world position we just checked.
        lastlocalpos = localpos;

        // Move forward along our line.
        len -= step;
        currentchunkspacepos += dir * step;

        // Break if we passed the end of our line.
        if( len < 0 )
            break;
    }

    if( pResult )
    {
        pResult->m_Hit = false;
    }

    return false;
}

// ============================================================================================================================
// Add/Remove blocks
// ============================================================================================================================
void VoxelChunk::ChangeBlockState(Vector3Int worldpos, bool enabled)
{
    unsigned int index = GetBlockIndex( worldpos );

    m_pBlocks[index].SetEnabled( enabled );
}
