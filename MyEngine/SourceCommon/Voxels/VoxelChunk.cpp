//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "VoxelBlock.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "Core/EngineCore.h"

VoxelChunk::VoxelChunk()
: MyMesh( nullptr )
{
    m_pWorld = 0;

    m_LockedInThreadedOp = false;

    m_MapCreated = false;
    m_MeshOptimized = false;
    m_MapWasEdited = false;
    m_WasVisibleLastFrame = false;

    m_Transform.SetIdentity();
    m_BlockSize.Set( 0, 0, 0 );
    m_ChunkSize.Set( 0, 0, 0 );
    m_ChunkOffset.Set( 0, 0, 0 );
    m_ChunkPosition.Set( 0, 0, 0 );
    m_pRenderGraphObject = 0;

    m_TextureTileCount.Set( 8, 8 );

    m_pBlockEnabledBits = 0;
    m_pBlocks = 0;
    m_BlocksAllocated = 0;
}

VoxelChunk::~VoxelChunk()
{
    RemoveFromRenderGraph();

    if( m_BlocksAllocated > 0 )
    {
        delete[] m_pBlockEnabledBits;
        delete[] m_pBlocks;
    }
}

void VoxelChunk::Initialize(VoxelWorld* world, Vector3 pos, Vector3Int chunkoffset, Vector3 blocksize)
{
    m_pWorld = world;

    m_Transform.SetIdentity();
    m_Transform.SetTranslation( pos );

    m_BlockSize = blocksize;
    m_ChunkOffset = chunkoffset;
    if( m_pWorld )
        m_ChunkPosition = m_pWorld->GetChunkPosition( m_ChunkOffset );

    m_MapCreated = false;

    // if this chunk isn't part of a world, then it's a manually created mesh, so set map is created for now.
    if( m_pWorld == 0 )
        m_MapCreated = true;

    // Set up the vertex format for this mesh.
    if( m_SubmeshList.Length() == 0 )
    {
        int indexbytes = 2; // TODO: take out hard-coded unsigned short as index type

        // Vertex_XYZUVNorm_RGBA
        VertexFormat_Dynamic_Desc* pVertFormat = m_pWorld->GetVertexFormatManager()->GetDynamicVertexFormat( 1, true, false, false, true, 0 );
        if( m_pWorld == 0 )
        {
            CreateOneSubmeshWithBuffers( pVertFormat, 0, indexbytes, 0, true );
        }
        else
        {
            CreateSubmeshes( 1 );
            CreateVertexBuffer( 0, pVertFormat, 0, true );
            SetIndexBuffer( m_pWorld->GetSharedIndexBuffer() );
        }
    }
}

void VoxelChunk::SetChunkSize(Vector3Int chunksize, uint32* pPreallocatedBlockEnabledBits, VoxelBlock* pPreallocatedBlocks)
{
    unsigned int numblocks = (unsigned int)(chunksize.x * chunksize.y * chunksize.z);
    if( numblocks == m_BlocksAllocated )
        return;

    if( chunksize == m_ChunkSize )
        return;

    if( m_pBlocks == 0 )
    {
        if( pPreallocatedBlocks ) // passed in by world objects, m_BlocksAllocated will equal 0 if it doesn't need freeing.
        {
            m_BlocksAllocated = 0;
            m_pBlockEnabledBits = pPreallocatedBlockEnabledBits;
            m_pBlocks = pPreallocatedBlocks;
        }
        else
        {
            m_BlocksAllocated = chunksize.x * chunksize.y * chunksize.z;
            int num4bytecontainersneeded = m_BlocksAllocated / 32;
            if( m_BlocksAllocated % 32 != 0 )
                num4bytecontainersneeded += 1;
            m_pBlockEnabledBits = MyNew uint32[num4bytecontainersneeded];
            m_pBlocks = MyNew VoxelBlock[m_BlocksAllocated];
        }

        for( unsigned int i=0; i<numblocks; i++ )
        {
            m_pBlocks[i].SetBlockType( 0 );
            m_pBlockEnabledBits[i/32] &= ~(1 << (i%32)); // TODO: don't set this bit by bit to disable all blocks
            //m_pBlocks[i].SetEnabled( false );
        }
    }
    else
    {
        // world chunks should never be resized.
        MyAssert( m_BlocksAllocated != 0 );

        if( (unsigned int)(chunksize.x * chunksize.y * chunksize.z) > m_BlocksAllocated )
        {
            VoxelBlock* oldblocks = m_pBlocks;

            m_BlocksAllocated = chunksize.x * chunksize.y * chunksize.z;
            m_pBlocks = MyNew VoxelBlock[m_BlocksAllocated];
            for( unsigned int i=0; i<m_BlocksAllocated; i++ )
            {
                m_pBlocks[i].SetBlockType( 0 );
                m_pBlockEnabledBits[i/32] &= ~(1 << (i%32)); // TODO: don't set this bit by bit to disable all blocks
                //m_pBlocks[i].SetEnabled( false );
            }

            // copy the contents of the old size into the new one.
            // m_ChunkSize is old size, chunksize is new.
            for( int z=0; z<m_ChunkSize.z; z++ )
            {
                for( int y=0; y<m_ChunkSize.y; y++ )
                {
                    for( int x=0; x<m_ChunkSize.x; x++ )
                    {
                        int oldoffset = z * m_ChunkSize.y * m_ChunkSize.x + y * m_ChunkSize.x + x;
                        int newoffset = z * chunksize.y * chunksize.x + y * chunksize.x + x;

                        bool wasenabled = m_pBlockEnabledBits[oldoffset/32] & 1 << (oldoffset%32) ? true : false;
                        if( wasenabled )
                            m_pBlockEnabledBits[newoffset/32] |= (1 << (newoffset%32));
                        else
                            m_pBlockEnabledBits[newoffset/32] &= ~(1 << (newoffset%32));
                        //m_pBlocks[newoffset].SetEnabled( oldblocks[oldoffset].IsEnabled() );
                        m_pBlocks[newoffset].SetBlockType( oldblocks[oldoffset].GetBlockType() );
                    }
                }
            }

            delete[] oldblocks;
        }
        else
        {
            // move the blocks around to fit into the smaller size.
            // m_ChunkSize is old size, chunksize is new.
            for( int z=0; z<chunksize.z; z++ )
            {
                for( int y=0; y<chunksize.y; y++ )
                {
                    for( int x=0; x<chunksize.x; x++ )
                    {
                        int oldoffset = z * m_ChunkSize.y * m_ChunkSize.x + y * m_ChunkSize.x + x;
                        int newoffset = z * chunksize.y * chunksize.x + y * chunksize.x + x;

                        if( oldoffset != newoffset )
                        {
                            bool wasenabled = m_pBlockEnabledBits[oldoffset/32] & 1 << (oldoffset%32) ? true : false;
                            if( wasenabled )
                                m_pBlockEnabledBits[newoffset/32] |= (1 << (newoffset%32));
                            else
                                m_pBlockEnabledBits[newoffset/32] &= ~(1 << (newoffset%32));
                            //m_pBlocks[newoffset].SetEnabled( m_pBlocks[oldoffset].IsEnabled() );
                            m_pBlocks[newoffset].SetBlockType( m_pBlocks[oldoffset].GetBlockType() );
                        }
                    }
                }
            }
        }
    }

    m_ChunkSize = chunksize;

    CalculateBounds();
}

void VoxelChunk::SetBlockSize(Vector3 blocksize)
{
    m_BlockSize = blocksize;
}

void VoxelChunk::SetTextureTileCount(Vector2Int tilecount)
{
    m_TextureTileCount = tilecount;
}

// ============================================================================================================================
// Internal functions
// ============================================================================================================================
void VoxelChunk::CalculateBounds()
{
    // figure out the min/max extents of this chunk
    Vector3 minextents( 0, 0, 0 ); //-m_BlockSize.x/2, -m_BlockSize.y/2, -m_BlockSize.z/2 );
    Vector3 maxextents( (m_ChunkSize.x-1) * m_BlockSize.x + m_BlockSize.x, ///2
                        (m_ChunkSize.x-1) * m_BlockSize.y + m_BlockSize.y, ///2
                        (m_ChunkSize.x-1) * m_BlockSize.z + m_BlockSize.z ); ///2

    Vector3 center = (minextents + maxextents) / 2;
    Vector3 extents = (maxextents - minextents) / 2;

    MyAABounds* pBounds = GetBounds();
    pBounds->Set( center, extents );
}

// ============================================================================================================================
// Internal file loading functions
// ============================================================================================================================
void VoxelChunk::CreateFromVoxelMeshFile()
{
    m_MeshReady = false;

    if( m_pSourceFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        OnFileFinishedLoadingVoxelMesh( m_pSourceFile );
    }
    else
    {
        m_pSourceFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingVoxelMesh );
    }
}

void VoxelChunk::OnFileFinishedLoadingVoxelMesh(MyFileObject* pFile)
{
    if( pFile == m_pSourceFile )
    {
        pFile->UnregisterFileFinishedLoadingCallback( this );

        cJSON* jVoxelMesh = cJSON_Parse( pFile->GetBuffer() );
        if( jVoxelMesh )
        {
            Initialize( 0, Vector3(0,0,0), Vector3Int(0,0,0), m_BlockSize );
            ImportFromJSONObject( jVoxelMesh );
            cJSON_Delete( jVoxelMesh );

            RebuildMesh( 1 );
        }
        else
        {
            // shouldn't happen if the file isn't corrupt
            // TODO: this number should match the size set in ComponentVoxelMesh.
            SetChunkSize( Vector3Int( 4, 4, 4 ) );
        }
    }
}

void VoxelChunk::ParseFile()
{
    MyAssert( m_pSourceFile == 0 || strcmp( m_pSourceFile->GetExtensionWithDot(), ".myvoxelmesh" ) == 0 );

    // not needed, only ".myvoxelmesh" files should be assigned to voxelchunks.
    //MyMesh::ParseFile();

    if( m_MeshReady == false )
    {
        if( m_pSourceFile != 0 )
        {
            if( strcmp( m_pSourceFile->GetExtensionWithDot(), ".myvoxelmesh" ) == 0 )
            {
                if( m_MapCreated == false )
                {
                    CreateFromVoxelMeshFile();
                }

                if( m_MapCreated )
                {
                    RebuildMesh( 1 );
                }
            }

            if( m_SubmeshList.Count() > 0 )
            {
                MaterialDefinition* pMaterial = m_SubmeshList[0]->GetMaterial();

                if( pMaterial && pMaterial->GetShader() == 0 )
                {
                    // Guess at an appropriate shader for this mesh/material.
                    MeshManager* pMeshManager = m_pGameCore->GetManagers()->GetMeshManager();
                    pMeshManager->GuessAndAssignAppropriateShaderToMesh( this );
                }
            }
        }
    }
}

bool VoxelChunk::IsReady()
{
    if( m_MeshReady == true )
    {
        MyAssert( m_MapCreated == true );
        return true;
    }

    return false;
}

bool VoxelChunk::MeshHasVerts()
{
    if( m_SubmeshList[0]->m_NumIndicesToDraw > 0 )
        return true;

    return false;
}

// ============================================================================================================================
// Load/Save ".myvoxelmesh" files
// ============================================================================================================================
cJSON* VoxelChunk::ExportAsJSONObject(bool exportforworld)
{
    cJSON* jVoxelMesh = cJSON_CreateObject();

    if( exportforworld == false )
    {
        cJSONExt_AddFloatArrayToObject( jVoxelMesh, "BlockSize", &m_BlockSize.x, 3 );
        cJSONExt_AddIntArrayToObject( jVoxelMesh, "ChunkSize", &m_ChunkSize.x, 3 );
        cJSONExt_AddIntArrayToObject( jVoxelMesh, "TextureTileCount", &m_TextureTileCount.x, 2 );
    }

    // save the blocks.
    MyStackAllocator::MyStackPointer stackpointer;
    char* blockstring = (char*)g_pEngineCore->GetSingleFrameMemoryStack()->AllocateBlock( m_ChunkSize.x * m_ChunkSize.y * m_ChunkSize.z + 1, &stackpointer );

    for( int z=0; z<m_ChunkSize.z; z++ )
    {
        for( int y=0; y<m_ChunkSize.y; y++ )
        {
            for( int x=0; x<m_ChunkSize.x; x++ )
            {
                int index = z * m_ChunkSize.y * m_ChunkSize.x + y * m_ChunkSize.x + x;
                unsigned int type = m_pBlocks[index].GetBlockType();
                if( (m_pBlockEnabledBits[index/32] & (1 << (index%32))) == 0 )
                //if( m_pBlocks[index].IsEnabled() == false )
                    type = 0;
                blockstring[index] = (char)type + '#';
            }
        }
    }

    blockstring[m_ChunkSize.z * m_ChunkSize.y * m_ChunkSize.x] = 0;
    MyAssert( strlen( blockstring ) == (size_t)(m_ChunkSize.z * m_ChunkSize.y * m_ChunkSize.x) );

    cJSON_AddStringToObject( jVoxelMesh, "Blocks", blockstring );

    g_pEngineCore->GetSingleFrameMemoryStack()->RewindStack( stackpointer );

    return jVoxelMesh;
}

void VoxelChunk::ImportFromJSONObject(cJSON* jVoxelMesh)
{
    Vector3 blocksize( 0, 0, 0 );
    cJSONExt_GetFloatArray( jVoxelMesh, "BlockSize", &blocksize.x, 3 );

    Vector3Int chunksize( 0, 0, 0 );
    cJSONExt_GetIntArray( jVoxelMesh, "ChunkSize", &chunksize.x, 3 );

    Vector2Int texturetilecount( 0, 0 );
    cJSONExt_GetIntArray( jVoxelMesh, "TextureTileCount", &texturetilecount.x, 2 );

    if( blocksize.x != 0 )
        m_BlockSize = blocksize;
    if( chunksize.x != 0 )
        SetChunkSize( chunksize );
    if( texturetilecount.x != 0 )
        SetTextureTileCount( texturetilecount );

    char* blockstring = cJSON_GetObjectItem( jVoxelMesh, "Blocks" )->valuestring;

    // load the blocks
    for( int z=0; z<m_ChunkSize.z; z++ )
    {
        for( int y=0; y<m_ChunkSize.y; y++ )
        {
            for( int x=0; x<m_ChunkSize.x; x++ )
            {
                int index = z * m_ChunkSize.y * m_ChunkSize.x + y * m_ChunkSize.x + x;
                int blocktype = blockstring[index] - '#';

                m_pBlocks[index].SetBlockType( blocktype );
                if( blocktype > 0 )
                    m_pBlockEnabledBits[index/32] |= (1 << (index%32));
                else
                    m_pBlockEnabledBits[index/32] &= ~(1 << (index%32));
                //m_pBlocks[index].SetEnabled( blocktype != 0 );
            }
        }
    }

    m_MapCreated = true;
}

// ============================================================================================================================
// Map/Blocks
// ============================================================================================================================
unsigned int VoxelChunk::DefaultGenerateMapFunc(VoxelWorld* pWorld, Vector3Int worldpos)
{
    bool enabled = false;

    if( 0 )
    {
        //float freq = 1/50.0f;

        //double value = open_simplex_noise3( pWorld->m_pOpenSimpleNoiseContext, worldpos.x * freq, worldpos.y * freq, worldpos.z * freq );

        //enabled = value > 0.0f;
    }
    else
    {
        float freq = 1/20.0f;

        double value = open_simplex_noise2( pWorld->m_pOpenSimpleNoiseContext, worldpos.x * freq, worldpos.z * freq );

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
    // Runs on a thread, so need to be thread-safe

    //LOGInfo( "VoxelWorld", "GenerateMap() Start - %d, %d, %d\n", m_ChunkPosition.x, m_ChunkPosition.y, m_ChunkPosition.z );

    MyAssert( m_pWorld != 0 );
    if( m_pWorld == 0 )
        return;

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

                VoxelWorld_GenerateMap_CallbackFunction* pFunc = m_pWorld->GetMapGenerationCallbackFunction();

                if( pFunc != 0 )
                    blocktype = pFunc( worldpos );
                else
                    blocktype = VoxelChunk::DefaultGenerateMapFunc( m_pWorld, worldpos );

                Vector3Int localpos = Vector3Int( x, y, z );
                VoxelBlock* pBlock = GetBlockFromLocalPos( localpos );
                pBlock->SetBlockType( blocktype );

                uint32 index = GetBlockIndexFromLocalPos( localpos );
                if( blocktype > 0 )
                    m_pBlockEnabledBits[index/32] |= (1 << (index%32));
                else
                    m_pBlockEnabledBits[index/32] &= ~(1 << (index%32));
            }
        }
    }

    //LOGInfo( "VoxelWorld", "GenerateMap() End - %d, %d, %d\n", m_ChunkPosition.x, m_ChunkPosition.y, m_ChunkPosition.z );

    m_MapCreated = true;
    m_LockedInThreadedOp = false;
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

    uint32 index = localz * m_ChunkSize.y * m_ChunkSize.x + localy * m_ChunkSize.x + localx;
    return m_pBlockEnabledBits[index/32] & 1 << (index%32) ? true : false;

    //VoxelBlock* pBlock = &m_pBlocks[localz * m_ChunkSize.y * m_ChunkSize.x + localy * m_ChunkSize.x + localx];

    //return pBlock->IsEnabled();
}

bool VoxelChunk::IsNearbyWorldBlockEnabled(unsigned int worldactivechunkarrayindex, int localx, int localy, int localz, bool blockexistsifnotready)
{
    // blocks must be within a chunk in each direction.
    MyAssert( localx >= -m_ChunkSize.x && localx < m_ChunkSize.x*2 );
    MyAssert( localy >= -m_ChunkSize.y && localy < m_ChunkSize.y*2 );
    MyAssert( localz >= -m_ChunkSize.z && localz < m_ChunkSize.z*2 );

    VoxelChunk* pChunk = this;

    if( m_pWorld == 0 )
    {
        if( localx < 0 )              { return blockexistsifnotready; }
        if( localx >= m_ChunkSize.x ) { return blockexistsifnotready; }
        if( localy < 0 )              { return blockexistsifnotready; }
        if( localy >= m_ChunkSize.y ) { return blockexistsifnotready; }
        if( localz < 0 )              { return blockexistsifnotready; }
        if( localz >= m_ChunkSize.z ) { return blockexistsifnotready; }
    }
    else //if( m_pWorld )
    {
        if( localx < 0 )
        {
            if( m_ChunkPosition.x - 1 < m_pWorld->m_WorldOffset.x )
                return blockexistsifnotready;
            pChunk = m_pWorld->m_pActiveWorldChunkPtrs[worldactivechunkarrayindex - 1];
            localx += m_ChunkSize.x;
        }
        else if( localx >= m_ChunkSize.x )
        {
            if( m_ChunkPosition.x + 1 >= m_pWorld->m_WorldOffset.x + m_pWorld->m_WorldSize.x )
                return blockexistsifnotready;
            pChunk = m_pWorld->m_pActiveWorldChunkPtrs[worldactivechunkarrayindex + 1];
            localx -= m_ChunkSize.x;
        }

        if( localy < 0 )
        {
            if( m_ChunkPosition.y - 1 < m_pWorld->m_WorldOffset.y )
                return blockexistsifnotready;
            pChunk = m_pWorld->m_pActiveWorldChunkPtrs[worldactivechunkarrayindex - m_pWorld->m_WorldSize.x];
            localy += m_ChunkSize.y;
        }
        else if( localy >= m_ChunkSize.y )
        {
            if( m_ChunkPosition.y + 1 >= m_pWorld->m_WorldOffset.y + m_pWorld->m_WorldSize.y )
                return blockexistsifnotready;
            pChunk = m_pWorld->m_pActiveWorldChunkPtrs[worldactivechunkarrayindex + m_pWorld->m_WorldSize.x];
            localy -= m_ChunkSize.y;
        }

        if( localz < 0 )
        {
            if( m_ChunkPosition.z - 1 < m_pWorld->m_WorldOffset.z )
                return blockexistsifnotready;
            pChunk = m_pWorld->m_pActiveWorldChunkPtrs[worldactivechunkarrayindex - m_pWorld->m_WorldSize.y * m_pWorld->m_WorldSize.x];
            localz += m_ChunkSize.z;
        }
        else if( localz >= m_ChunkSize.z )
        {
            if( m_ChunkPosition.z + 1 >= m_pWorld->m_WorldOffset.z + m_pWorld->m_WorldSize.z )
                return blockexistsifnotready;
            pChunk = m_pWorld->m_pActiveWorldChunkPtrs[worldactivechunkarrayindex + m_pWorld->m_WorldSize.y * m_pWorld->m_WorldSize.x];
            localz -= m_ChunkSize.z;
        }
    }

    MyAssert( pChunk != 0 );

    unsigned int localindex = localz * m_ChunkSize.y * m_ChunkSize.x + localy * m_ChunkSize.x + localx;
    return pChunk->m_pBlockEnabledBits[localindex/32] & (1 << (localindex%32)) ? true : false;
}

int VoxelChunk::CountNeighbouringBlocks(unsigned int worldactivechunkarrayindex, int localx, int localy, int localz, bool blockexistsifnotready)
{
    int count = 0;

    for( int x = -1; x <= 1; x++ )
    {
        for( int y = -1; y <= 1; y++ )
        {
            for( int z = -1; z <= 1; z++ )
            {
                if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, localx + x, localy + y, localz + z, blockexistsifnotready ) )
                    count++;
            }
        }
    }

    return count;
}

// ============================================================================================================================
// Mesh building
// ============================================================================================================================
bool VoxelChunk::RebuildMesh(unsigned int increment, Vertex_XYZUVNorm_RGBA* pPreallocatedVerts, int* pVertCount, float* pTimeToBuild)
{
    // Runs on a thread, so need to be thread-safe
    // samples from neighbouring world chunks, so world is not allowed to change while rebuild is running

    //LOGInfo( "VoxelWorld", "RebuildMesh() Start - %d, %d, %d\n", m_ChunkPosition.x, m_ChunkPosition.y, m_ChunkPosition.z );

    MyAssert( m_pBlocks );
    MyAssert( GetStride( 0 ) == (12 + 8 + 12 + 4) ); // Vertex_XYZUVNorm_RGBA => XYZ + UV + NORM + RGBA

#if MYFW_PROFILING_ENABLED
    double Timing_Start = MyTime_GetSystemTime();
#endif

    //Sleep( 1000 );

    // Grab the pointer to the current position of our stack allocator, we'll rewind at the end.
    MyStackAllocator::MyStackPointer memstart = 0;
    if( pPreallocatedVerts == 0 )
    {
        memstart = g_pEngineCore->GetSingleFrameMemoryStack()->GetCurrentLocation();
    }

    // Loop through blocks and add a cube for each one that's enabled
    {
        unsigned int worldactivechunkarrayindex = -1;

        if( m_pWorld )
            worldactivechunkarrayindex = m_pWorld->GetActiveChunkArrayIndex( m_ChunkPosition );

        int numblocks = m_ChunkSize.x * m_ChunkSize.y * m_ChunkSize.z;

        int maxverts = 6*4*numblocks;
        int vertbuffersize = maxverts * GetStride( 0 );

        // Allocate a block of ram big enough to store our verts
        Vertex_XYZUVNorm_RGBA* pVerts = pPreallocatedVerts;
        if( pPreallocatedVerts == 0 )
        {
            pVerts = (Vertex_XYZUVNorm_RGBA*)g_pEngineCore->GetSingleFrameMemoryStack()->AllocateBlock( vertbuffersize );
        }

        // pVerts gets advanced by code below, so store a copy.
        Vertex_XYZUVNorm_RGBA* pActualVerts = pVerts;

        //  block type          1, 2, 3, 4, 5, 6
        int TileTops_Col[] =  { 0, 1, 2, 3, 4, 5 };
        int TileTops_Row[] =  { 0, 0, 0, 0, 0, 0 };
        int TileSides_Col[] = { 0, 1, 2, 3, 4, 5 };
        int TileSides_Row[] = { 1, 1, 1, 1, 1, 1 };

        int vertcount = 0;
        //int indexcount = 0;
        int count = 0;
        for( int z=0; z<m_ChunkSize.z; z++ )
        {
            for( int y=0; y<m_ChunkSize.y; y++ )
            {
                for( int x=0; x<m_ChunkSize.x; x++ )
                {
                    unsigned int index = z * m_ChunkSize.y * m_ChunkSize.x + y * m_ChunkSize.x + x;
                    if( (m_pBlockEnabledBits[index/32] & (1 << (index%32))) == 0 )
                        continue;

                    VoxelBlock* pBlock = &m_pBlocks[index];
                    //if( pBlock->IsEnabled() == false )
                    //    continue;

                    Vector3Int worldpos( m_ChunkOffset.x+x, m_ChunkOffset.y+y, m_ChunkOffset.z+z );

                    int blocktypetextureindex = pBlock->GetBlockType() - 1;
                    MyAssert( blocktypetextureindex != -1 );

                    struct XYZRGBA
                    {
                        Vector3 pos;
                        ColorByte color;
                    };

                    XYZRGBA ltf;
                    XYZRGBA ltb;
                    XYZRGBA lbf;
                    XYZRGBA lbb;
                    XYZRGBA rtf;
                    XYZRGBA rtb;
                    XYZRGBA rbf;
                    XYZRGBA rbb;

                    float xleft   = x*m_BlockSize.x;// - m_BlockSize.x/2;
                    float xright  = x*m_BlockSize.x + m_BlockSize.x;///2;
                    float ybottom = y*m_BlockSize.y;// - m_BlockSize.y/2;
                    float ytop    = y*m_BlockSize.y + m_BlockSize.y;///2;
                    float zfront  = z*m_BlockSize.z;// - m_BlockSize.z/2;
                    float zback   = z*m_BlockSize.z + m_BlockSize.z;///2;

                    ltf.pos.Set( xleft,  ytop,    zfront );
                    ltb.pos.Set( xleft,  ytop,    zback  );
                    lbf.pos.Set( xleft,  ybottom, zfront );
                    lbb.pos.Set( xleft,  ybottom, zback  );
                    rtf.pos.Set( xright, ytop,    zfront );
                    rtb.pos.Set( xright, ytop,    zback  );
                    rbf.pos.Set( xright, ybottom, zfront );
                    rbb.pos.Set( xright, ybottom, zback  );

                    bool ambientocclusion = true;

                    ColorByte darker( 48, 48, 48, 0 );
                    ColorByte light( 196, 196, 196, 255 );

                    // debug, turn edge blocks red
                    if( false )
                    {
                        if( x == 0 || x == m_ChunkSize.x - 1 ||
                            y == 0 || y == m_ChunkSize.y - 1 ||
                            z == 0 || z == m_ChunkSize.z - 1 )
                        {
                            light = ColorByte( 255, 32, 128, 255 );
                        }
                    }

                    //ltf.color = light;
                    //ltb.color = light;
                    //lbf.color = light;
                    //lbb.color = light;
                    //rtf.color = light;
                    //rtb.color = light;
                    //rbf.color = light;
                    //rbb.color = light;

                    int ltfao = 0;
                    int ltbao = 0;
                    int lbfao = 0;
                    int lbbao = 0;
                    int rtfao = 0;
                    int rtbao = 0;
                    int rbfao = 0;
                    int rbbao = 0;

                    if( ambientocclusion )
                    {
                        // -x    x    +x
                        // LTB ----- RTB // +z
                        //  |         |
                        //  |         |  // z
                        //  |         |
                        // LTF ----- RTF // -z
                        // top of blocks
                        //if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x, y+1, z, true ) == false )
                        {
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x, y+1, z, true ) == true ) // above
                            {
                                ltbao++; // Left(-x) - Top - Back(+z)
                                rtbao++; // Right(+x) - Top - Back(+z)
                                ltfao++; // Left(-x) - Top - Front(-z)
                                rtfao++; // Right(+x) - Top - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z+1, true ) == true ) // UpperMiddle
                            {
                                ltbao++; // Left(-x) - Top - Back(+z)
                                rtbao++; // Right(+x) - Top - Back(+z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z+1, true ) == true ) // UpperRight
                            {
                                rtbao++; // Right(+x) - Top - Back(+z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z  , true ) == true ) // Right
                            {
                                rtbao++; // Right(+x) - Top - Back(+z)
                                rtfao++; // // Right(+x) - Top - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z-1, true ) == true ) // BotRight
                            {
                                rtfao++; // // Right(+x) - Top - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z-1, true ) == true ) // BottomMiddle
                            {
                                rtfao++; // // Right(+x) - Top - Front(-z)
                                ltfao++; // Left(-x) - Top - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z-1, true ) == true ) // BotLeft
                            {
                                ltfao++; // Left(-x) - Top - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z  , true ) == true ) // Left
                            {
                                ltbao++; // Left(-x) - Top - Back(+z)
                                ltfao++; // Left(-x) - Top - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z+1, true ) == true ) // UpperLeft
                            {
                                ltbao++; // Left(-x) - Top - Back(+z)
                            }
                        }

                        // bottom of blocks
                        {
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x, y-1, z, true ) == true ) // below
                            {
                                lbbao++; // Left(-x) - Bottom - Back(+z)
                                rbbao++; // Right(+x) - Bottom - Back(+z)
                                lbfao++; // Left(-x) - Bottom - Front(-z)
                                rbfao++; // Right(+x) - Bottom - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z+1, true ) == true ) // UpperMiddle
                            {
                                lbbao++; // Left(-x) - Bottom - Back(+z)
                                rbbao++; // Right(+x) - Bottom - Back(+z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z+1, true ) == true ) // UpperRight
                            {
                                rbbao++; // Right(+x) - Bottom - Back(+z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z  , true ) == true ) // Right
                            {
                                rbbao++; // Right(+x) - Bottom - Back(+z)
                                rbfao++; // // Right(+x) - Bottom - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z-1, true ) == true ) // BotRight
                            {
                                rbfao++; // // Right(+x) - Bottom - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z-1, true ) == true ) // BottomMiddle
                            {
                                rbfao++; // // Right(+x) - Bottom - Front(-z)
                                lbfao++; // Left(-x) - Bottom - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z-1, true ) == true ) // BotLeft
                            {
                                lbfao++; // Left(-x) - Bottom - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z  , true ) == true ) // Left
                            {
                                lbbao++; // Left(-x) - Bottom - Back(+z)
                                lbfao++; // Left(-x) - Bottom - Front(-z)
                            }
                            if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z+1, true ) == true ) // UpperLeft
                            {
                                lbbao++; // Left(-x) - Bottom - Back(+z)
                            }
                        }

                        ltf.color.Set( light.r - (unsigned char)(darker.r * ltfao), light.g - (unsigned char)(darker.g * ltfao), light.b - (unsigned char)(darker.b * ltfao), 1 );
                        ltb.color.Set( light.r - (unsigned char)(darker.r * ltbao), light.g - (unsigned char)(darker.g * ltbao), light.b - (unsigned char)(darker.b * ltbao), 1 );
                        lbf.color.Set( light.r - (unsigned char)(darker.r * lbfao), light.g - (unsigned char)(darker.g * lbfao), light.b - (unsigned char)(darker.b * lbfao), 1 );
                        lbb.color.Set( light.r - (unsigned char)(darker.r * lbbao), light.g - (unsigned char)(darker.g * lbbao), light.b - (unsigned char)(darker.b * lbbao), 1 );
                        rtf.color.Set( light.r - (unsigned char)(darker.r * rtfao), light.g - (unsigned char)(darker.g * rtfao), light.b - (unsigned char)(darker.b * rtfao), 1 );
                        rtb.color.Set( light.r - (unsigned char)(darker.r * rtbao), light.g - (unsigned char)(darker.g * rtbao), light.b - (unsigned char)(darker.b * rtbao), 1 );
                        rbf.color.Set( light.r - (unsigned char)(darker.r * rbfao), light.g - (unsigned char)(darker.g * rbfao), light.b - (unsigned char)(darker.b * rbfao), 1 );
                        rbb.color.Set( light.r - (unsigned char)(darker.r * rbbao), light.g - (unsigned char)(darker.g * rbbao), light.b - (unsigned char)(darker.b * rbbao), 1 );

                        //{
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x, y+1, z, true ) == true ) // above
                        //    {
                        //        ltb.color -= darker; // Left(-x) - Top - Back(+z)
                        //        rtb.color -= darker; // Right(+x) - Top - Back(+z)
                        //        ltf.color -= darker; // Left(-x) - Top - Front(-z)
                        //        rtf.color -= darker; // Right(+x) - Top - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z+1, true ) == true ) // UpperMiddle
                        //    {
                        //        ltb.color -= darker; // Left(-x) - Top - Back(+z)
                        //        rtb.color -= darker; // Right(+x) - Top - Back(+z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z+1, true ) == true ) // UpperRight
                        //    {
                        //        rtb.color -= darker; // Right(+x) - Top - Back(+z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z  , true ) == true ) // Right
                        //    {
                        //        rtb.color -= darker; // Right(+x) - Top - Back(+z)
                        //        rtf.color -= darker; // // Right(+x) - Top - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z-1, true ) == true ) // BotRight
                        //    {
                        //        rtf.color -= darker; // // Right(+x) - Top - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z-1, true ) == true ) // BottomMiddle
                        //    {
                        //        rtf.color -= darker; // // Right(+x) - Top - Front(-z)
                        //        ltf.color -= darker; // Left(-x) - Top - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z-1, true ) == true ) // BotLeft
                        //    {
                        //        ltf.color -= darker; // Left(-x) - Top - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z  , true ) == true ) // Left
                        //    {
                        //        ltb.color -= darker; // Left(-x) - Top - Back(+z)
                        //        ltf.color -= darker; // Left(-x) - Top - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z+1, true ) == true ) // UpperLeft
                        //    {
                        //        ltb.color -= darker; // Left(-x) - Top - Back(+z)
                        //    }
                        //}

                        //// bottom of blocks
                        //{
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x, y-1, z, true ) == true ) // below
                        //    {
                        //        lbb.color -= darker; // Left(-x) - Bottom - Back(+z)
                        //        rbb.color -= darker; // Right(+x) - Bottom - Back(+z)
                        //        lbf.color -= darker; // Left(-x) - Bottom - Front(-z)
                        //        rbf.color -= darker; // Right(+x) - Bottom - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z+1, true ) == true ) // UpperMiddle
                        //    {
                        //        lbb.color -= darker; // Left(-x) - Bottom - Back(+z)
                        //        rbb.color -= darker; // Right(+x) - Bottom - Back(+z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z+1, true ) == true ) // UpperRight
                        //    {
                        //        rbb.color -= darker; // Right(+x) - Bottom - Back(+z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z  , true ) == true ) // Right
                        //    {
                        //        rbb.color -= darker; // Right(+x) - Bottom - Back(+z)
                        //        rbf.color -= darker; // // Right(+x) - Bottom - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z-1, true ) == true ) // BotRight
                        //    {
                        //        rbf.color -= darker; // // Right(+x) - Bottom - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z-1, true ) == true ) // BottomMiddle
                        //    {
                        //        rbf.color -= darker; // // Right(+x) - Bottom - Front(-z)
                        //        lbf.color -= darker; // Left(-x) - Bottom - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z-1, true ) == true ) // BotLeft
                        //    {
                        //        lbf.color -= darker; // Left(-x) - Bottom - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z  , true ) == true ) // Left
                        //    {
                        //        lbb.color -= darker; // Left(-x) - Bottom - Back(+z)
                        //        lbf.color -= darker; // Left(-x) - Bottom - Front(-z)
                        //    }
                        //    if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z+1, true ) == true ) // UpperLeft
                        //    {
                        //        lbb.color -= darker; // Left(-x) - Bottom - Back(+z)
                        //    }
                        //}
                    }

                    bool smooth = false;

                    if( smooth == true )
                    {
                        if( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x, y+1, z  , true ) == false )
                        {
                            // Left - Top - Front
                            if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z  , true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+2, z  , true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z-1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+2, z-1, true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z-1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+2, z-1, true ) == false ) )
                            {
                                ltf.pos.y += m_BlockSize.x/2;
                            }
                            //else
                            //if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y  , z  , true ) == false &&
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y  , z-1, true ) == false &&
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y  , z-1, true ) == false ) &&
                            //    ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z  , true ) == true  ||
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z-1, true ) == true  ||
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z-1, true ) == true  ) )
                            //{
                            //    ltf.y -= m_BlockSize.x/2;
                            //}

                            // Left - Top - Back
                            if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z  , true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+2, z  , true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+1, z+1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y+2, z+1, true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z+1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+2, z+1, true ) == false ) )
                            {
                                ltb.pos.y += m_BlockSize.x/2;
                            }
                            //else
                            //if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y  , z  , true ) == false &&
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y  , z+1, true ) == false &&
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y  , z+1, true ) == false ) &&
                            //    ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z  , true ) == true  ||
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x-1, y-1, z+1, true ) == true  ||
                            //      IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z+1, true ) == true  ) )
                            //{
                            //    ltb.y -= m_BlockSize.x/2;
                            //}

                            // Right - Top - Front
                            if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z  , true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+2, z  , true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z-1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+2, z-1, true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z-1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+2, z-1, true ) == false ) )
                            {
                                rtf.pos.y += m_BlockSize.x/2;
                            }
                            else
                            if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y  , z  , true ) == false ) &&
                                  //IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y  , z-1, true ) == false &&
                                  //IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y  , z-1, true ) == false ) &&
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z  , true ) == true  ||
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z-1, true ) == true  ||
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z-1, true ) == true  ) )
                            {
                                rtf.pos.y -= m_BlockSize.x/2;
                            }

                            // Right - Top - Back
                            if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z  , true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+2, z  , true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+1, z+1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y+2, z+1, true ) == false ) ||
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+1, z+1, true ) == true  &&
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y+2, z+1, true ) == false ) )
                            {
                                rtb.pos.y += m_BlockSize.x/2;
                            }
                            else
                            if( ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y  , z  , true ) == false ) &&
                                  //IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y  , z+1, true ) == false &&
                                  //IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y  , z+1, true ) == false ) &&
                                ( IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z  , true ) == true  ||
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x+1, y-1, z+1, true ) == true  ||
                                  IsNearbyWorldBlockEnabled( worldactivechunkarrayindex, x  , y-1, z+1, true ) == true  ) )
                            {
                                rtb.pos.y -= m_BlockSize.x/2;
                            }

                            //lbf.y;
                            //lbb.y;
                            //rbf.Set( xright, ybottom, zfront );
                            //rbb.Set( xright, ybottom, zback  );
                        }
                    }

                    float uleft   = (float)(TileSides_Col[blocktypetextureindex]+0) / m_TextureTileCount.x;
                    float uright  = (float)(TileSides_Col[blocktypetextureindex]+1) / m_TextureTileCount.x;
                    float vtop    = (float)(TileSides_Row[blocktypetextureindex]+0) / m_TextureTileCount.y;
                    float vbottom = (float)(TileSides_Row[blocktypetextureindex]+1) / m_TextureTileCount.y;

                    unsigned int neighbourindex;

                    //unsigned char numneighbours = (unsigned char)CountNeighbouringBlocks( worldactivechunkarrayindex, x, y, z, false );
                    //if( numneighbours < 20 )
                    //    numneighbours = 1;
                    //ColorByte blockcolor = ColorByte( 255 / numneighbours, 255 / numneighbours, 255 / numneighbours, 255 );

                    // front
                    if( z == 0 )
                        neighbourindex = (m_ChunkSize.z-1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x);
                    else
                        neighbourindex = (z-1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x);

                    if( z == 0 && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y, worldpos.z-1 ) )
                    {
                    }
                    else if( z == 0 || (m_pBlockEnabledBits[neighbourindex/32] & (1 << (neighbourindex%32))) == 0 ) //m_pBlocks[(z-1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos = ltf.pos; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;    // upper left
                        pVerts[1].pos = rtf.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;    // upper right
                        pVerts[2].pos = lbf.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom; // lower left
                        pVerts[3].pos = rbf.pos; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom; // lower right
                        pVerts[0].color = ltf.color;
                        pVerts[1].color = rtf.color;
                        pVerts[2].color = lbf.color;
                        pVerts[3].color = rbf.color;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, 0, -1 );
                        pVerts += 4;
                        vertcount += 4;
                    }

                    // back
                    if( z == (m_ChunkSize.z-1) )
                        neighbourindex = (0) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x);
                    else
                        neighbourindex = (z+1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x);

                    if( z == (m_ChunkSize.z-1) && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y, worldpos.z+1 ) )
                    {
                    }
                    else if( z == (m_ChunkSize.z-1) || (m_pBlockEnabledBits[neighbourindex/32] & (1 << (neighbourindex%32))) == 0 ) //m_pBlocks[(z+1) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos = rtb.pos; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos = ltb.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos = rbb.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos = lbb.pos; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        pVerts[0].color = rtb.color;
                        pVerts[1].color = ltb.color;
                        pVerts[2].color = rbb.color;
                        pVerts[3].color = lbb.color;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, 0, 1 );
                        pVerts += 4;
                        vertcount += 4;
                    }

                    // left
                    if( x == 0 )
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (m_ChunkSize.x-1);
                    else
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x-1);

                    if( x == 0 && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x-1, worldpos.y, worldpos.z ) )
                    {
                    }
                    else if( x == 0 || (m_pBlockEnabledBits[neighbourindex/32] & (1 << (neighbourindex%32))) == 0 ) //m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x-1)].IsEnabled() == false )
                    {
                        pVerts[0].pos = ltb.pos; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos = ltf.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos = lbb.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos = lbf.pos; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        pVerts[0].color = ltb.color;
                        pVerts[1].color = ltf.color;
                        pVerts[2].color = lbb.color;
                        pVerts[3].color = lbf.color;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( -1, 0, 0 );
                        pVerts += 4;
                        vertcount += 4;
                    }

                    // right
                    if( x == (m_ChunkSize.x-1) )
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (0);
                    else
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x+1);

                    if( x == (m_ChunkSize.x-1) && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x+1, worldpos.y, worldpos.z ) )
                    {
                    }
                    else if( x == (m_ChunkSize.x-1) || (m_pBlockEnabledBits[neighbourindex/32] & (1 << (neighbourindex%32))) == 0 ) //m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y) * m_ChunkSize.x + (x+1)].IsEnabled() == false )
                    {
                        pVerts[0].pos = rtf.pos; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos = rtb.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos = rbf.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos = rbb.pos; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        pVerts[0].color = rtf.color;
                        pVerts[1].color = rtb.color;
                        pVerts[2].color = rbf.color;
                        pVerts[3].color = rbb.color;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 1, 0, 0 );
                        pVerts += 4;
                        vertcount += 4;
                    }

                    // bottom
                    if( y == 0 )
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (m_ChunkSize.y-1) * m_ChunkSize.x + (x);
                    else
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (y-1) * m_ChunkSize.x + (x);

                    if( y == 0 && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y-1, worldpos.z ) )
                    {
                    }
                    else if( y == 0 || (m_pBlockEnabledBits[neighbourindex/32] & (1 << (neighbourindex%32))) == 0 ) //m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y-1) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        pVerts[0].pos = lbf.pos; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                        pVerts[1].pos = rbf.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                        pVerts[2].pos = lbb.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                        pVerts[3].pos = rbb.pos; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                        pVerts[0].color = lbf.color;
                        pVerts[1].color = rbf.color;
                        pVerts[2].color = lbb.color;
                        pVerts[3].color = rbb.color;
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, -1, 0 );
                        pVerts += 4;
                        vertcount += 4;
                    }

                    uleft   = (float)(TileTops_Col[blocktypetextureindex]+0) / m_TextureTileCount.x;
                    uright  = (float)(TileTops_Col[blocktypetextureindex]+1) / m_TextureTileCount.x;
                    vtop    = (float)(TileTops_Row[blocktypetextureindex]+0) / m_TextureTileCount.y;
                    vbottom = (float)(TileTops_Row[blocktypetextureindex]+1) / m_TextureTileCount.y;

                    // top
                    if( y == (m_ChunkSize.y-1) )
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (0) * m_ChunkSize.x + (x);
                    else
                        neighbourindex = (z) * m_ChunkSize.y * m_ChunkSize.x + (y+1) * m_ChunkSize.x + (x);

                    if( y == (m_ChunkSize.y-1) && m_pWorld && m_pWorld->IsBlockEnabled( worldpos.x, worldpos.y+1, worldpos.z ) )
                    {
                    }
                    else if( y == (m_ChunkSize.y-1) || (m_pBlockEnabledBits[neighbourindex/32] & (1 << (neighbourindex%32))) == 0 ) //m_pBlocks[(z) * m_ChunkSize.y * m_ChunkSize.x + (y+1) * m_ChunkSize.x + (x)].IsEnabled() == false )
                    {
                        if( rtb.color.r + ltf.color.r > ltb.color.r + rtf.color.r )
                        {
                            pVerts[0].pos = ltb.pos; pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;
                            pVerts[1].pos = rtb.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;
                            pVerts[2].pos = ltf.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom;
                            pVerts[3].pos = rtf.pos; pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom;
                            pVerts[0].color = ltb.color;
                            pVerts[1].color = rtb.color;
                            pVerts[2].color = ltf.color;
                            pVerts[3].color = rtf.color;
                        }
                        else
                        {
                            pVerts[2].pos = ltb.pos; pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vtop;
                            pVerts[0].pos = rtb.pos; pVerts[0].uv.x = uright; pVerts[0].uv.y = vtop;
                            pVerts[3].pos = ltf.pos; pVerts[3].uv.x = uleft;  pVerts[3].uv.y = vbottom;
                            pVerts[1].pos = rtf.pos; pVerts[1].uv.x = uright; pVerts[1].uv.y = vbottom;
                            pVerts[2].color = ltb.color;
                            pVerts[0].color = rtb.color;
                            pVerts[3].color = ltf.color;
                            pVerts[1].color = rtf.color;
                        }
                        for( int i=0; i<4; i++ )
                            pVerts[i].normal.Set( 0, 1, 0 );
                        pVerts += 4;
                        vertcount += 4;
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

        if( pPreallocatedVerts == 0 )
        {
            CopyVertsIntoVBO( pActualVerts, vertcount );
        }
        else
        {
            *pVertCount = vertcount;
        }
    }

    if( pPreallocatedVerts == 0 )
    {
        g_pEngineCore->GetSingleFrameMemoryStack()->RewindStack( memstart );
    }

#if MYFW_PROFILING_ENABLED
    double Timing_End = MyTime_GetSystemTime();

    //LOGInfo( "VoxelChunk", "Chunk offset (%d, %d, %d) - time to build %f\n",
    //                       m_ChunkOffset.x, m_ChunkOffset.y, m_ChunkOffset.z,
    //                       (Timing_End - Timing_Start) * 1000 );

    if( pTimeToBuild )
    {
        *pTimeToBuild = (float)((Timing_End - Timing_Start) * 1000);
    }
#endif

    //LOGInfo( "VoxelWorld", "RebuildMesh() End - %d, %d, %d\n", m_ChunkPosition.x, m_ChunkPosition.y, m_ChunkPosition.z );

    m_LockedInThreadedOp = false;

    return true;
}

void VoxelChunk::CopyVertsIntoVBO(Vertex_XYZUVNorm_RGBA* pVerts, int vertcount)
{
#if MYFW_PROFILING_ENABLED
    //double Timing_Start = MyTime_GetSystemTime();
#endif

    int numblocks = m_ChunkSize.x * m_ChunkSize.y * m_ChunkSize.z;

    int maxverts = 6*4*numblocks;

    if( vertcount > 0 )
    {
        // Copy the data into the OpenGL buffers.
        if( vertcount > 0 )
        {
            if( vertcount > 256*256 )
            {
                LOGInfo( "VoxelWorld", "Too many verts needed in chunk - %d\n", maxverts );
            }

            BufferDefinition* pVertBuffer = m_SubmeshList[0]->m_pVertexBuffer;
            pVertBuffer->TempBufferData( vertcount * GetStride( 0 ), pVerts );
        }

        // if not a world object, fill an index buffer... TODO: remove this, indices can be static.
        if( m_pWorld == 0 )
        {
            unsigned int numquads = vertcount / 4;
            unsigned int indexbuffersize = numquads * 6;
        
            MyStackAllocator::MyStackPointer memstart = g_pEngineCore->GetSingleFrameMemoryStack()->GetCurrentLocation();

            unsigned int bytestoallocate = indexbuffersize * sizeof(unsigned short);
            unsigned short* pIndices = (unsigned short*)g_pEngineCore->GetSingleFrameMemoryStack()->AllocateBlock( bytestoallocate );

            for( unsigned int i=0; i<numquads; i++ )
            {
                pIndices[i*6+0] = (unsigned short)(i*4 + g_SpriteVertexIndices[0]);
                pIndices[i*6+1] = (unsigned short)(i*4 + g_SpriteVertexIndices[1]);
                pIndices[i*6+2] = (unsigned short)(i*4 + g_SpriteVertexIndices[2]);
                pIndices[i*6+3] = (unsigned short)(i*4 + g_SpriteVertexIndices[3]);
                pIndices[i*6+4] = (unsigned short)(i*4 + g_SpriteVertexIndices[4]);
                pIndices[i*6+5] = (unsigned short)(i*4 + g_SpriteVertexIndices[5]);
            }

            m_SubmeshList[0]->m_pIndexBuffer->TempBufferData( bytestoallocate, pIndices );
            
            g_pEngineCore->GetSingleFrameMemoryStack()->RewindStack( memstart );
        }

        m_SubmeshList[0]->m_NumIndicesToDraw = vertcount / 4 * 6;
        //LOGInfo( "VoxelChunk", "Num indices: %d\n", indexcount );

        CalculateBounds();
    }
    else
    {
        m_SubmeshList[0]->m_NumIndicesToDraw = 0;

        Vector3 center( 0, 0, 0 );
        Vector3 extents( 0, 0, 0 );
        
        MyAABounds* pBounds = GetBounds();
        pBounds->Set( center, extents );

        RemoveFromRenderGraph();
    }

    // if any of the neighbouring chunks wasn't ready, then we likely created extra faces to close outside walls.
    // rebuild the mesh to get rid of those sides.
    // also fix ambient occlusion issues
    if( m_pWorld )
    {
        m_MeshOptimized = true;

        for( int i=0; i<6; i++ )
        {
            Vector3Int neighbourchunkpos = m_ChunkPosition;

            if( i == 0 ) neighbourchunkpos.x += 1;
            if( i == 1 ) neighbourchunkpos.x -= 1;
            if( i == 2 ) neighbourchunkpos.y += 1;
            if( i == 3 ) neighbourchunkpos.y -= 1;
            if( i == 4 ) neighbourchunkpos.z += 1;
            if( i == 5 ) neighbourchunkpos.z -= 1;

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

#if MYFW_PROFILING_ENABLED
    //double Timing_End = MyTime_GetSystemTime();

    //LOGInfo( "VoxelChunk", "Chunk offset (%d, %d, %d) - time to copy to VBO %f\n",
    //                       m_ChunkOffset.x, m_ChunkOffset.y, m_ChunkOffset.z,
    //                       (Timing_End - Timing_Start) * 1000 );
#endif
}

void VoxelChunk::AddToRenderGraph(void* pUserData, MaterialDefinition* pMaterial)
{
    if( m_pRenderGraphObject != 0 )
        return;

    m_pRenderGraphObject = g_pComponentSystemManager->GetRenderGraph()->AddObject(
        &m_Transform, this, m_SubmeshList[0],
        pMaterial, MyRE::PrimitiveType_Triangles, 0, 1, pUserData );
}

void VoxelChunk::OverrideRenderGraphObjectTransform(MyMatrix* pTransform)
{
    MyAssert( m_pRenderGraphObject != 0 );
    if( m_pRenderGraphObject == 0 )
        return;

    m_pRenderGraphObject->m_pTransform = pTransform;
}

void VoxelChunk::RemoveFromRenderGraph()
{
    if( m_pRenderGraphObject == 0 )
        return;

    g_pComponentSystemManager->GetRenderGraph()->RemoveObject( m_pRenderGraphObject );
    m_pRenderGraphObject = 0;
}

// ============================================================================================================================
// MyMesh overrides
// ============================================================================================================================
void VoxelChunk::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    MyMesh::SetMaterial( pMaterial, 0 );

    if( m_pRenderGraphObject != 0 )
    {
        m_pRenderGraphObject->SetMaterial( pMaterial, true );
    }
}

void VoxelChunk::PreDraw()
{
    //MyMesh::PreDraw(); // doesn't do anything

    m_WasVisibleLastFrame = true;

    if( m_pWorld )
    {
        if( m_LockedInThreadedOp == false )
            m_pWorld->SetChunkVisible( this );
    }
}

// ============================================================================================================================
// Space conversions
// ============================================================================================================================
Vector3Int VoxelChunk::GetWorldPosition(Vector3 scenepos)
{
    Vector3Int worldpos;

    worldpos.x = (int)floor( (scenepos.x/*+m_BlockSize.x/2*/) / m_BlockSize.x );
    worldpos.y = (int)floor( (scenepos.y/*+m_BlockSize.y/2*/) / m_BlockSize.y );
    worldpos.z = (int)floor( (scenepos.z/*+m_BlockSize.z/2*/) / m_BlockSize.z );

    return worldpos;
}

VoxelBlock* VoxelChunk::GetBlockFromLocalPos(Vector3Int localpos)
{
    VoxelBlock* pBlock = &m_pBlocks[localpos.z * m_ChunkSize.y * m_ChunkSize.x + localpos.y * m_ChunkSize.x + localpos.x];

    return pBlock;
}

unsigned int VoxelChunk::GetBlockIndexFromLocalPos(Vector3Int localpos)
{
    return localpos.z * m_ChunkSize.y * m_ChunkSize.x + localpos.y * m_ChunkSize.x + localpos.x;
}

unsigned int VoxelChunk::GetBlockIndexFromWorldPos(Vector3Int worldpos)
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

    for( int i=0; i<6; i++ )
    {
        switch( i )
        {
        case 0: plane.Set( Vector3(-1,0,0), Vector3(   0, 0, 0) ); break;
        case 1: plane.Set( Vector3( 1,0,0), Vector3(bs.x, 0, 0) ); break;
        case 2: plane.Set( Vector3(0,-1,0), Vector3(0,    0, 0) ); break;
        case 3: plane.Set( Vector3(0, 1,0), Vector3(0, bs.y, 0) ); break;
        case 4: plane.Set( Vector3(0,0,-1), Vector3(0, 0,    0) ); break;
        case 5: plane.Set( Vector3(0,0, 1), Vector3(0, 0, bs.z) ); break;
        }

        plane.IntersectRay( startpos, endpos, &result );

        if( ( i >=0 && i <= 1 && result.y > 0 && result.y < bs.y && result.z > 0 && result.z < bs.z ) ||
            ( i >=2 && i <= 3 && result.x > 0 && result.x < bs.x && result.z > 0 && result.z < bs.z ) ||
            ( i >=4 && i <= 5 && result.x > 0 && result.x < bs.x && result.y > 0 && result.y < bs.y ) )
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
void VoxelChunk::ChangeBlockState(Vector3Int worldpos, unsigned int type, bool enabled)
{
    m_MapWasEdited = true;

    unsigned int index = GetBlockIndexFromWorldPos( worldpos );

    m_pBlocks[index].SetBlockType( type );
    if( enabled )
        m_pBlockEnabledBits[index/32] |= (1 << (index%32));
    else
        m_pBlockEnabledBits[index/32] &= ~(1 << (index%32));
    //m_pBlocks[index].SetEnabled( enabled );
}
