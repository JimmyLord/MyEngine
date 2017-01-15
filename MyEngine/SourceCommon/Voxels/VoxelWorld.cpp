//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "VoxelBlock.h"
#include "VoxelChunk.h"
#include "VoxelJobs.h"
#include "VoxelWorld.h"

VoxelWorld::VoxelWorld()
{
    m_NumChunkPointersAllocated = 0;
    m_VoxelBlockEnabledBitsSingleAllocation = 0;
    m_VoxelBlockSingleAllocation = 0;
    m_VoxelChunkSingleAllocation = 0;
    m_MeshBuilderVertsSingleAllocation = 0;
    m_pActiveWorldChunkPtrs = 0;

    m_WorldSize.Set( 0, 0, 0 );
    m_ChunkSize.Set( 16, 16, 16 );
    m_BlockSize.Set( 1, 1, 1 );

    m_WorldOffset.Set( 0, 0, 0 );
    m_DesiredOffset.Set( 0, 0, 0 );

    m_pMaterial = 0;
    m_pSharedIndexBuffer = 0;

    m_pMapGenCallbackFunc = 0;

    m_MaxWorldSize.Set( 0, 0, 0 );
    m_pSaveFile = 0;
    m_jJSONSavedMapData = 0;

    for( int i=0; i<MAX_GENERATORS; i++ )
    {
        m_pChunkGenerators[i] = MyNew VoxelChunkGenerator;
    }
    m_NumActiveChunkGenerators = 0;

    for( int i=0; i<MAX_BUILDERS; i++ )
    {
        m_pMeshBuilders[i] = MyNew VoxelMeshBuilder;
    }
    m_NumActiveMeshBuilders = 0;
}

VoxelWorld::~VoxelWorld()
{
    for( int i=0; i<MAX_GENERATORS; i++ )
    {
        delete m_pChunkGenerators[i];
    }

    for( int i=0; i<MAX_BUILDERS; i++ )
    {
        delete m_pMeshBuilders[i];
    }

    for( CPPListNode* pNode = m_pChunksFree.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( CPPListNode* pNode = m_pChunksNotVisible.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( CPPListNode* pNode = m_pChunksLoading.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( CPPListNode* pNode = m_pChunksBeingGenerated.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( CPPListNode* pNode = m_pChunksWaitingForMesh.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( CPPListNode* pNode = m_pChunksBeingMeshed.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        pChunk->Release();
    }

    for( unsigned int i=0; i<m_NumChunkPointersAllocated; i++ )
    {
        m_VoxelChunkSingleAllocation[i].RemoveFinalRefIfCreatedOnStackToAvoidAssertInDestructor();
    }
    delete[] m_VoxelChunkSingleAllocation;
    delete[] m_MeshBuilderVertsSingleAllocation;
    delete[] m_pActiveWorldChunkPtrs;

    delete[] m_VoxelBlockEnabledBitsSingleAllocation;
    delete[] m_VoxelBlockSingleAllocation;

    SAFE_RELEASE( m_pMaterial );
    SAFE_RELEASE( m_pSharedIndexBuffer );
    SAFE_RELEASE( m_pSaveFile );
    if( m_jJSONSavedMapData )
    {
        cJSON_Delete( m_jJSONSavedMapData );
    }
}

void VoxelWorld::Initialize(Vector3Int visibleworldsize)
{
    // Make sure init is only called once.
    MyAssert( m_NumChunkPointersAllocated == 0 );

    m_pSharedIndexBuffer = g_pBufferManager->CreateBuffer();
    m_pSharedIndexBuffer->InitializeBuffer( 0, 0, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, (VertexFormats)2, 0, "IBO", "VoxelWorld" );
    BuildSharedIndexBuffer();
    
    SetWorldSize( visibleworldsize );

    Vector3 chunkoffset = m_ChunkSize.MultiplyComponents( m_BlockSize );

    // make 0,0,0 the bottom left corner. TODO: pass in an offset
    m_WorldOffset.Set( 0, 0, 0 );
    m_DesiredOffset.Set( 0, 0, 0 );

    for( int z=0; z<m_WorldSize.z; z++ )
    {
        for( int y=0; y<m_WorldSize.y; y++ )
        {
            for( int x=0; x<m_WorldSize.x; x++ )
            {
                unsigned int chunkindex = (unsigned int)(z * m_WorldSize.y * m_WorldSize.x + y * m_WorldSize.x + x);
                VoxelChunk* pChunk = &m_VoxelChunkSingleAllocation[chunkindex];
                
                unsigned int chunksize = m_ChunkSize.x*m_ChunkSize.y*m_ChunkSize.z;

                int num4bytecontainersneeded = chunksize / 32;
                if( chunksize % 32 != 0 )
                    num4bytecontainersneeded += 1;
                uint32* pBlockEnabledBits = &m_VoxelBlockEnabledBitsSingleAllocation[chunkindex * num4bytecontainersneeded];

                VoxelBlock* pBlocks = &m_VoxelBlockSingleAllocation[chunkindex * chunksize];

                m_pChunksFree.MoveHead( pChunk );

                PrepareChunk( Vector3Int( x, y, z ), pBlockEnabledBits, pBlocks );
            }
        }
    }
}

void VoxelWorld::Tick(double timepassed)
{
    // If ever our single shared index buffer isn't ready (startup, lost context, etc), create the indices
    if( m_pSharedIndexBuffer->m_Dirty )
    {
        BuildSharedIndexBuffer();
    }

    // Only make chunks once the save file is fully loaded, if there's a save file.
    if( m_pSaveFile && m_jJSONSavedMapData == 0 )
    {
        if( m_pSaveFile->m_FileLoadStatus < FileLoadStatus_Success )
        {
            return;
        }

        if( m_pSaveFile->m_FileLoadStatus == FileLoadStatus_Success )
        {
            m_jJSONSavedMapData = cJSON_Parse( m_pSaveFile->m_pBuffer );
            if( m_jJSONSavedMapData == 0 )
                m_jJSONSavedMapData = cJSON_CreateObject();

            Vector3 blocksize = m_BlockSize;
            cJSONExt_GetFloatArray( m_jJSONSavedMapData, "BlockSize", &blocksize.x, 3 );

            Vector3Int chunksize = m_ChunkSize;
            cJSONExt_GetIntArray( m_jJSONSavedMapData, "ChunkSize", &chunksize.x, 3 );

            // TODO: adjust world's sizes to match.
            MyAssert( blocksize == m_BlockSize );
            MyAssert( chunksize == m_ChunkSize );
        }
    }

    // Only call if there are no active mesh builders.  No new builders will get created if world center isn't desired center.
    if( m_NumActiveChunkGenerators == 0 && m_NumActiveMeshBuilders == 0 )
    {
        SetWorldCenterForReal( m_DesiredOffset + m_WorldSize/2 );
    }

    // Sort chunks that need generating based on distance from world center (which is likely the player location)
    SortChunkList( &m_pChunksLoading );

    // Deal with generate chunk jobs that completed and start new ones
    int chunksGeneratedThisFrame = DealWithGeneratedChunkJobs();

    // Sort chunks that need meshing based on distance from world center (which is likely the player location)
    SortChunkList( &m_pChunksWaitingForMesh );

    // Deal with meshing jobs that completed and start new ones
    int chunksMeshedThisFrame = DealWithMeshedChunkJobs();

    // Some stats posted in an ImGui window
    {
        ImGui::SetNextWindowSize( ImVec2(150,100), ImGuiSetCond_FirstUseEver );
        ImGui::Begin( "Voxel World Stats" );

        ImGui::Text( "Active Gen's: %d", m_NumActiveChunkGenerators );
        ImGui::Text( "Active Mesher's: %d", m_NumActiveMeshBuilders );

        ImGui::Text( "Chunks Gen'd this frame: %d", chunksGeneratedThisFrame );
        ImGui::Text( "Chunks Mesh'd this frame: %d", chunksMeshedThisFrame );        

        ImGui::End();
    }
}

int VoxelWorld::DealWithGeneratedChunkJobs()
{
    int jobscomplete = 0;

    // if any previous chunks was finished generating, free up the ChunkGenerator.
    for( int i=0; i<MAX_GENERATORS; i++ )
    {
        if( m_pChunkGenerators[i]->m_IsFinished )
        {
            m_pChunkGenerators[i]->m_IsFinished = false;

            VoxelChunk* pChunk = m_pChunkGenerators[i]->m_pChunk;

            if( pChunk )
            {
                jobscomplete++;
                m_NumActiveChunkGenerators--;
                m_pChunkGenerators[i]->m_pChunk = 0;

                if( pChunk->m_MapCreated == true )
                    m_pChunksWaitingForMesh.MoveTail( pChunk );
           }
        }
    }

    // if any chunks aren't initialized, either load from json or call generate func (threaded)
    VoxelChunk* pChunk = (VoxelChunk*)m_pChunksLoading.GetHead();
    if( pChunk )
    {
        // Don't add chunk generation jobs if a new center is requested.
        if( m_DesiredOffset == m_WorldOffset )
        {
            int maxtogenerateinoneframe = 100;

            for( int i=0; i<maxtogenerateinoneframe; i++ )
            {
                if( pChunk == 0 )
                    break;

                MyAssert( pChunk->IsMapCreated() == false );

                Vector3Int chunkpos = GetChunkPosition( pChunk->GetChunkOffset() );
                cJSON* jChunk = GetJSONObjectForChunk( chunkpos );

                if( jChunk )
                {
                    pChunk->ImportFromJSONObject( jChunk );
                    m_pChunksWaitingForMesh.MoveTail( pChunk );
                }
                else
                {
                    // find an open VoxelChunkGenerator
                    int freeChunkGeneratorIndex = 0;
                    for( freeChunkGeneratorIndex=0; freeChunkGeneratorIndex<MAX_GENERATORS; freeChunkGeneratorIndex++ )
                    {
                        if( m_pChunkGenerators[freeChunkGeneratorIndex]->m_pChunk == 0 )
                            break;
                    }
                    if( freeChunkGeneratorIndex == MAX_GENERATORS )
                        break;

                    // Move the chunk to the next list in the chain
                    m_pChunksBeingGenerated.MoveTail( pChunk );

                    if( pChunk )
                    {
                        VoxelChunkGenerator* pChunkGenerator = m_pChunkGenerators[freeChunkGeneratorIndex];
                        m_NumActiveChunkGenerators++;

                        pChunkGenerator->m_IsStarted = false;
                        pChunkGenerator->m_IsFinished = false;
                        pChunkGenerator->m_pChunk = pChunk;

                        pChunk->m_LockedInThreadedOp = true;
                                
                        MyAssert( pChunkGenerator->m_pChunk->m_MapCreated == false );
                        g_pJobManager->AddJob( pChunkGenerator );
                            
                        //pChunk->GenerateMap();
                        //m_pChunksWaitingForMesh.MoveTail( pChunk );
                    }
                }

                pChunk = (VoxelChunk*)m_pChunksLoading.GetHead();
            }
        }
    }

    return jobscomplete;
}

int VoxelWorld::DealWithMeshedChunkJobs()
{
    int jobscomplete = 0;

    // if any previous mesh was finished building, copy the verts into a VBO and free up the MeshBuilder.
    for( int i=0; i<MAX_BUILDERS; i++ )
    {
        if( m_pMeshBuilders[i]->m_IsFinished )
        {
            m_pMeshBuilders[i]->m_IsFinished = false;

            VoxelChunk* pChunk = m_pMeshBuilders[i]->m_pChunk;

            if( pChunk )
            {
                jobscomplete++;

                m_NumActiveMeshBuilders--;
                m_pMeshBuilders[i]->m_pChunk = 0;

                //LOGInfo( "VoxelWorld", "Chunk offset (%d, %d, %d) - Time to build %f\n",
                //                       pChunk->m_ChunkOffset.x, pChunk->m_ChunkOffset.y, pChunk->m_ChunkOffset.z,
                //                       m_pMeshBuilders[i]->m_TimeToBuild );

                pChunk->CopyVertsIntoVBO( m_pMeshBuilders[i]->m_pVerts, m_pMeshBuilders[i]->m_VertCount );
                if( pChunk->m_MeshReady == true )
                    m_pChunksVisible.MoveTail( pChunk );
           }
        }
    }

    // if any chunks are done loading/generating, build the mesh (threaded)
    // TODO: don't build if neighbours aren't generated
    VoxelChunk* pChunk = (VoxelChunk*)m_pChunksWaitingForMesh.GetHead();
    if( pChunk )
    {
        // Don't add mesh building jobs if a new center is requested.
        if( m_DesiredOffset == m_WorldOffset )
        {
            // build one mesh for each meshbuilder object (added to MyJobManager queue)
            int maxtobuildsimultaneously = MAX_BUILDERS;
            for( int i=0; i<maxtobuildsimultaneously; i++ )
            {
                if( pChunk == 0 )
                    break;

                // find an open meshbuilder
                int freeMeshBuilderIndex = 0;
                for( freeMeshBuilderIndex=0; freeMeshBuilderIndex<MAX_BUILDERS; freeMeshBuilderIndex++ )
                {
                    if( m_pMeshBuilders[freeMeshBuilderIndex]->m_pChunk == 0 )
                        break;
                }
                if( freeMeshBuilderIndex == MAX_BUILDERS )
                    break;
                
                // Move the chunk to the next list in the chain
                m_pChunksBeingMeshed.MoveTail( pChunk );

                {
                    pChunk->SetMaterial( m_pMaterial, 0 );

                    MyAssert( pChunk->m_MapCreated == true );

                    VoxelMeshBuilder* pMeshBuilder = m_pMeshBuilders[freeMeshBuilderIndex];
                    m_NumActiveMeshBuilders++;
                
                    pMeshBuilder->m_IsStarted = false;
                    pMeshBuilder->m_IsFinished = false;
                    pMeshBuilder->m_pChunk = pChunk;

                    pChunk->m_LockedInThreadedOp = true;

                    g_pJobManager->AddJob( pMeshBuilder );
                }

                pChunk = (VoxelChunk*)m_pChunksWaitingForMesh.GetHead();
            }
        }
    }

    return jobscomplete;
}

void VoxelWorld::SetWorldSize(Vector3Int visibleworldsize)
{
    // TODO: remove this once function is properly implemented
    MyAssert( m_NumChunkPointersAllocated == 0 );
    
    unsigned int pointersneeded = visibleworldsize.x * visibleworldsize.y * visibleworldsize.z;

    if( pointersneeded > m_NumChunkPointersAllocated )
    {
        // TODO: allocate an array with enough pointers for the new size if it's bigger than the old,
        //       copy the existing chunk ptrs over
        //       delete the old array

        m_NumChunkPointersAllocated = pointersneeded;

        unsigned int numberofblocksinachunk = m_ChunkSize.x*m_ChunkSize.y*m_ChunkSize.z;

        m_MeshBuilderVertsSingleAllocation = MyNew Vertex_XYZUVNorm_RGBA[numberofblocksinachunk*6*4 * MAX_BUILDERS];
        for( int i=0; i<MAX_BUILDERS; i++ )
        {
            m_pMeshBuilders[i]->m_pVerts = &m_MeshBuilderVertsSingleAllocation[numberofblocksinachunk*6*4 * i];
        }

        m_VoxelChunkSingleAllocation = MyNew VoxelChunk[pointersneeded];
        m_pActiveWorldChunkPtrs = MyNew VoxelChunk*[pointersneeded];

        unsigned int numberofblocksneeded = pointersneeded * numberofblocksinachunk;
        int num4bytecontainersneeded = numberofblocksneeded / 32;
        if( numberofblocksneeded % 32 != 0 )
            num4bytecontainersneeded += 1;
        m_VoxelBlockEnabledBitsSingleAllocation = MyNew uint32[num4bytecontainersneeded];
        m_VoxelBlockSingleAllocation = MyNew VoxelBlock[numberofblocksneeded];

        LOGInfo( LOGTag, "VoxelWorld Allocation -> blocks %d + %d\n",
            pointersneeded * m_ChunkSize.x*m_ChunkSize.y*m_ChunkSize.z * sizeof( VoxelBlock ),
            num4bytecontainersneeded * sizeof(uint32) );

        // give each chunk/mesh a single ref, removed manually before deleting the array.
        for( unsigned int i=0; i<pointersneeded; i++ )
        {
            m_VoxelChunkSingleAllocation[i].AddRef();
        }
    }

    m_WorldSize = visibleworldsize;
}

void VoxelWorld::SetWorldCenter(Vector3 scenepos)
{
    Vector3Int worldpos;
    worldpos.x = (int)floor( (scenepos.x / m_BlockSize.x) / m_ChunkSize.x );
    worldpos.y = (int)floor( (scenepos.y / m_BlockSize.y) / m_ChunkSize.y );
    worldpos.z = (int)floor( (scenepos.z / m_BlockSize.z) / m_ChunkSize.z );

    SetWorldCenter( worldpos );
}

void VoxelWorld::SetWorldCenter(Vector3Int newworldcenter)
{
    m_DesiredOffset = newworldcenter - m_WorldSize/2;
}

void VoxelWorld::SetWorldCenterForReal(Vector3Int newworldcenter)
{
//#if MYFW_PROFILING_ENABLED
//    static double Timing_LastFrameTime = 0;
//
//    double Timing_Start = MyTime_GetSystemTime();
//#endif

    // Since chunks meshes are built on a thread and sample from neighbour chunks
    //   this can only be done when all rebuild mesh jobs are idle
    MyAssert( m_NumActiveMeshBuilders == 0 );
    MyAssert( m_NumActiveChunkGenerators == 0 );

    Vector3Int currentworldcenter = m_WorldOffset + m_WorldSize/2;
    if( newworldcenter == currentworldcenter )
        return;

    //LOGInfo( "VoxelWorld", "SetWorldCenter: %d, %d, %d\n", newworldcenter.x, newworldcenter.y, newworldcenter.z );

    int numchunks = m_WorldSize.x * m_WorldSize.y * m_WorldSize.z;

    // shift the current chunks on the x-axis
    {
        int stepx = newworldcenter.x - currentworldcenter.x;

        while( stepx > 0 ) // shift all chunks to the left (x-axis)
        {
            m_WorldOffset.x += 1;

            for( int x=0; x<m_WorldSize.x; x++ )
                for( int y=0; y<m_WorldSize.y; y++ )
                    for( int z=0; z<m_WorldSize.z; z++ )
                        ShiftChunk( Vector3Int( x, y, z ), Vector3Int( x+1, y, z ), x == (m_WorldSize.x - 1) ? true : false );

            stepx--;
        }

        while( stepx < 0 ) // shift all chunks to the right (x-axis)
        {
            m_WorldOffset.x -= 1;

            for( int x=m_WorldSize.x-1; x>=0; x-- ) // loop backwards
                for( int y=0; y<m_WorldSize.y; y++ )
                    for( int z=0; z<m_WorldSize.z; z++ )
                        ShiftChunk( Vector3Int( x, y, z ), Vector3Int( x-1, y, z ), x == 0 ? true : false );

            stepx++;
        }
    }

    // shift the current chunks on the y-axis
    {
        int stepy = newworldcenter.y - currentworldcenter.y;

        while( stepy > 0 ) // shift all chunks up (y-axis)
        {
            m_WorldOffset.y += 1;

            for( int y=0; y<m_WorldSize.y; y++ )
                for( int x=0; x<m_WorldSize.x; x++ )
                    for( int z=0; z<m_WorldSize.z; z++ )
                        ShiftChunk( Vector3Int( x, y, z ), Vector3Int( x, y+1, z ), y == (m_WorldSize.y - 1) ? true : false );

            stepy--;
        }

        while( stepy < 0 ) // shift all chunks down (y-axis)
        {
            m_WorldOffset.y -= 1;

            for( int y=m_WorldSize.y-1; y>=0; y-- ) // loop backwards
                for( int x=0; x<m_WorldSize.x; x++ )
                    for( int z=0; z<m_WorldSize.z; z++ )
                        ShiftChunk( Vector3Int( x, y, z ), Vector3Int( x, y-1, z ), y == 0 ? true : false );

            stepy++;
        }
    }

    // shift the current chunks on the z-axis
    {
        int stepz = newworldcenter.z - currentworldcenter.z;

        while( stepz > 0 ) // shift all chunks in (z-axis)
        {
            m_WorldOffset.z += 1;

            for( int z=0; z<m_WorldSize.z; z++ )
                for( int y=0; y<m_WorldSize.y; y++ )
                    for( int x=0; x<m_WorldSize.x; x++ )
                        ShiftChunk( Vector3Int( x, y, z ), Vector3Int( x, y, z+1 ), z == (m_WorldSize.z - 1) ? true : false );

            stepz--;
        }

        while( stepz < 0 ) // shift all chunks out (y-axis)
        {
            m_WorldOffset.z -= 1;

            for( int z=m_WorldSize.z-1; z>=0; z-- ) // loop backwards
                for( int y=0; y<m_WorldSize.y; y++ )
                    for( int x=0; x<m_WorldSize.x; x++ )
                        ShiftChunk( Vector3Int( x, y, z ), Vector3Int( x, y, z-1 ), z == 0 ? true : false );

            stepz++;
        }
    }

//#if MYFW_PROFILING_ENABLED
//    double Timing_End = MyTime_GetSystemTime();
//
//    float functime = (float)((Timing_End - Timing_Start)*1000);
//
//    //LOGInfo( "VoxelWorld", "SetWorldCenter functime: %0.2f\n", functime );
//#endif
}

void VoxelWorld::SetChunkVisible(VoxelChunk* pChunk)
{
    if( pChunk->m_MeshReady )
    {
        MyAssert( pChunk->m_MapCreated == true );
        m_pChunksVisible.MoveTail( pChunk );
    }
    else if( pChunk->m_MapCreated )
    {
        m_pChunksWaitingForMesh.MoveTail( pChunk );
    }
    else
    {
        m_pChunksLoading.MoveTail( pChunk );
    }
}

void VoxelWorld::SortChunkList(CPPListHead* pChunkList)
{
    // sort chunks quickly
    
    // create 10 buckets, place each chunk in one bucket than put them all back into the list passed in.
    CPPListHead buckets[10];

    Vector3Int worldCenter = m_WorldOffset + m_WorldSize/2;

    CPPListNode* pNextNode;
    for( CPPListNode* pNode = pChunkList->GetHead(); pNode; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        VoxelChunk* pChunk = (VoxelChunk*)pNode;

        Vector3Int offset = pChunk->m_ChunkPosition;
        Vector3Int diff = offset - worldCenter;

        // artificially increase y diff, i.e. prefer chunks on our plane
        if( diff.y < -1 || diff.y > 1 )
            diff.y *= 5;

        int distance = (int)diff.Length();

        if( distance > 9 )
            distance = 9;

        buckets[distance].MoveTail( pChunk );
    }

    // put all chunks back into the list passed in
    for( int i=0; i<10; i++ )
    {
        pChunkList->Append( &buckets[i] );
    }
}

void VoxelWorld::UpdateVisibility(void* pUserData)
{
    // Add all visible chunks to scene graph
    //for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; pNode = pNode->GetNext() )
    //{
    //    VoxelChunk* pChunk = (VoxelChunk*)pNode;

    //    pChunk->AddToSceneGraph( pUserData, m_pMaterial );
    //}

    // Add all chunks to scene graph, let scene graph decide which are visible // TODO: make sure scene graph is an octree
    for( unsigned int i=0; i<m_NumChunkPointersAllocated; i++ )
    {
        VoxelChunk* pChunk = m_pActiveWorldChunkPtrs[i];

        if( pChunk->IsReady() && pChunk->MeshHasVerts() == false )
        {
            // don't add prepped but empty chunks back to scene graph
            int bp = 1;
        }
        else
        {
            pChunk->AddToSceneGraph( pUserData, m_pMaterial );
        }
    }
}

void VoxelWorld::SetMaterial(MaterialDefinition* pMaterial)
{
    pMaterial->AddRef();
    if( m_pMaterial )
        m_pMaterial->Release();
    m_pMaterial = pMaterial;

    for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;

        pChunk->SetMaterial( m_pMaterial, 0 );        
    }
}

void VoxelWorld::SetSaveFile(MyFileObject* pFile)
{
    if( pFile )
        pFile->AddRef();

    SAFE_RELEASE( m_pSaveFile );
    m_pSaveFile = pFile;
}

void VoxelWorld::SaveTheWorld()
{
#if MYFW_NACL
    return;
#else
    MyAssert( m_pSaveFile );

    if( m_jJSONSavedMapData == 0 )
        m_jJSONSavedMapData = cJSON_CreateObject();

    if( cJSON_GetObjectItem( m_jJSONSavedMapData, "BlockSize" ) )
    {
        cJSON_DeleteItemFromObject( m_jJSONSavedMapData, "BlockSize" );
        cJSON_DeleteItemFromObject( m_jJSONSavedMapData, "ChunkSize" );
    }
    cJSONExt_AddFloatArrayToObject( m_jJSONSavedMapData, "BlockSize", &m_BlockSize.x, 3 );
    cJSONExt_AddIntArrayToObject( m_jJSONSavedMapData, "ChunkSize", &m_ChunkSize.x, 3 );

    for( int z=0; z<m_WorldSize.z; z++ )
    {
        for( int y=0; y<m_WorldSize.y; y++ )
        {
            for( int x=0; x<m_WorldSize.x; x++ )
            {
                Vector3Int chunkpos = m_WorldOffset + Vector3Int( x, y, z );

                VoxelChunk* pChunk = GetActiveChunk( chunkpos );
                SaveChunk( pChunk );
            }
        }
    }

    if( m_jJSONSavedMapData )
    {
        FILE* pFile = 0;
#if MYFW_WINDOWS
        fopen_s( &pFile, m_pSaveFile->m_FullPath, "wb" );
#else
        pFile = fopen( m_pSaveFile->m_FullPath, "wb" );
#endif

        char* jsonstring = cJSON_Print( m_jJSONSavedMapData );
        fprintf( pFile, "%s", jsonstring );
        cJSONExt_free( jsonstring );

        fclose( pFile );
    }
#endif // MYFW_NACL
}

void VoxelWorld::SaveChunk(VoxelChunk* pChunk)
{
    if( m_jJSONSavedMapData == 0 )
        return;

    if( pChunk == 0 || pChunk->IsMapEdited() == false )
        return;

    Vector3Int chunkpos = pChunk->GetChunkOffset();
    chunkpos.x /= m_ChunkSize.x;
    chunkpos.y /= m_ChunkSize.y;
    chunkpos.z /= m_ChunkSize.z;

    char strx[20];
    char stry[20];
    char strz[20];

    sprintf_s( strx, 20, "%d", chunkpos.x );
    sprintf_s( stry, 20, "%d", chunkpos.y );
    sprintf_s( strz, 20, "%d", chunkpos.z );

    cJSON* jZ = cJSON_GetObjectItem( m_jJSONSavedMapData, strz );
    if( jZ == 0 )
    {
        jZ = cJSON_CreateObject();
        cJSON_AddItemToObject( m_jJSONSavedMapData, strz, jZ );
    }

    if( jZ )
    {
        cJSON* jY = cJSON_GetObjectItem( jZ, stry );
        if( jY == 0 )
        {
            jY = cJSON_CreateObject();
            cJSON_AddItemToObject( jZ, stry, jY );
        }

        if( jY )
        {
            cJSON* jX = cJSON_GetObjectItem( jY, strx );

            cJSON* jChunk = pChunk->ExportAsJSONObject( true );

            if( jX )
            {
                cJSON_ReplaceItemInObject( jY, strx, jChunk );
            }
            else
            {
                cJSON_AddItemToObject( jY, strx, jChunk );
            }
        }
    }
}

// ============================================================================================================================
// Protected/Internal functions
// ============================================================================================================================
void VoxelWorld::BuildSharedIndexBuffer()
{
    unsigned int numquads = 65536 / 4;
    unsigned int indexbuffersize = numquads * 6;
        
    MyStackAllocator::MyStackPointer memstart = g_pEngineCore->m_SingleFrameMemoryStack.GetCurrentLocation();
    unsigned short* pIndices = (unsigned short*)g_pEngineCore->m_SingleFrameMemoryStack.AllocateBlock( indexbuffersize );

    for( unsigned int i=0; i<numquads; i++ )
    {
        pIndices[i*6+0] = (unsigned short)(i*4 + g_SpriteVertexIndices[0]);
        pIndices[i*6+1] = (unsigned short)(i*4 + g_SpriteVertexIndices[1]);
        pIndices[i*6+2] = (unsigned short)(i*4 + g_SpriteVertexIndices[2]);
        pIndices[i*6+3] = (unsigned short)(i*4 + g_SpriteVertexIndices[3]);
        pIndices[i*6+4] = (unsigned short)(i*4 + g_SpriteVertexIndices[4]);
        pIndices[i*6+5] = (unsigned short)(i*4 + g_SpriteVertexIndices[5]);
    }

    m_pSharedIndexBuffer->TempBufferData( numquads * 6, pIndices );

    g_pEngineCore->m_SingleFrameMemoryStack.RewindStack( memstart );
}

bool VoxelWorld::IsChunkActive(Vector3Int chunkpos)
{
    if( chunkpos.x >= m_WorldOffset.x && chunkpos.x < m_WorldOffset.x + m_WorldSize.x &&
        chunkpos.y >= m_WorldOffset.y && chunkpos.y < m_WorldOffset.y + m_WorldSize.y &&
        chunkpos.z >= m_WorldOffset.z && chunkpos.z < m_WorldOffset.z + m_WorldSize.z )
    {
        return true;
    }

    return false;
}

unsigned int VoxelWorld::GetActiveChunkArrayIndex(Vector3Int chunkpos)
{
    return GetActiveChunkArrayIndex( chunkpos.x, chunkpos.y, chunkpos.z );
}

unsigned int VoxelWorld::GetActiveChunkArrayIndex(int chunkx, int chunky, int chunkz)
{
    unsigned int index = (chunkz - m_WorldOffset.z) * m_WorldSize.y * m_WorldSize.x +
                         (chunky - m_WorldOffset.y) * m_WorldSize.x +
                         (chunkx - m_WorldOffset.x);

    MyAssert( index < m_NumChunkPointersAllocated );

    return index;
}

VoxelChunk* VoxelWorld::GetActiveChunk(unsigned int arrayindex)
{
    MyAssert( arrayindex < m_NumChunkPointersAllocated );
    MyAssert( arrayindex < (unsigned int)(m_WorldSize.x * m_WorldSize.y * m_WorldSize.z) );

    return m_pActiveWorldChunkPtrs[arrayindex];
}

VoxelChunk* VoxelWorld::GetActiveChunk(Vector3Int chunkpos)
{
    return GetActiveChunk( GetActiveChunkArrayIndex( chunkpos ) );
}

VoxelChunk* VoxelWorld::GetActiveChunk(int chunkx, int chunky, int chunkz)
{
    return GetActiveChunk( GetActiveChunkArrayIndex( chunkx, chunky, chunkz ) );
}

void VoxelWorld::PrepareChunk(Vector3Int chunkpos, uint32* pPreallocatedBlockEnabledBits, VoxelBlock* pBlocks)
{
    VoxelChunk* pChunk = (VoxelChunk*)m_pChunksFree.GetHead();
    if( pChunk == 0 )
    {
        LOGInfo( "VoxelWorld", "Attempting to prepare chunk, but none available\n" );
        return;
    }

    Vector3Int chunkblockoffset = m_ChunkSize.MultiplyComponents( chunkpos );
    Vector3 chunkposition = chunkblockoffset.MultiplyComponents( m_BlockSize );

    unsigned int arrayindex = GetActiveChunkArrayIndex( chunkpos );
    m_pActiveWorldChunkPtrs[arrayindex] = pChunk;

    pChunk->Initialize( this, chunkposition, chunkblockoffset, m_BlockSize );
    if( pBlocks != 0 )
        pChunk->SetChunkSize( m_ChunkSize, pPreallocatedBlockEnabledBits, pBlocks );

    m_pChunksNotVisible.MoveTail( pChunk );
}

void VoxelWorld::ShiftChunk(Vector3Int to, Vector3Int from, bool isedgeblock)
{
    int tooffset = to.z * m_WorldSize.y * m_WorldSize.x + to.y * m_WorldSize.x + to.x;
                    
    VoxelChunk* pChunk = m_pActiveWorldChunkPtrs[tooffset];
    if( pChunk )
    {
        m_pChunksFree.MoveTail( pChunk );
        SaveChunk( pChunk );
        pChunk->RemoveFromSceneGraph();
        pChunk->Clear();
    }

    if( isedgeblock )
    {
        PrepareChunk( m_WorldOffset + to, 0, 0 );
    }
    else
    {
        int fromoffset = from.z * m_WorldSize.y * m_WorldSize.x + from.y * m_WorldSize.x + from.x;
        m_pActiveWorldChunkPtrs[tooffset] = m_pActiveWorldChunkPtrs[fromoffset];
        m_pActiveWorldChunkPtrs[fromoffset] = 0;
    }
}

cJSON* VoxelWorld::GetJSONObjectForChunk(Vector3Int chunkpos)
{
    if( m_jJSONSavedMapData == 0 )
        return 0;

    char strx[20];
    char stry[20];
    char strz[20];

    sprintf_s( strx, 20, "%d", chunkpos.x );
    sprintf_s( stry, 20, "%d", chunkpos.y );
    sprintf_s( strz, 20, "%d", chunkpos.z );

    cJSON* jZ = cJSON_GetObjectItem( m_jJSONSavedMapData, strz );
    if( jZ )
    {
        cJSON* jY = cJSON_GetObjectItem( jZ, stry );
        if( jY )
        {
            cJSON* jX = cJSON_GetObjectItem( jY, strx );
            if( jX )
            {
                return jX;
            }
        }
    }

    return 0;
}

// ============================================================================================================================
// Map generation
// ============================================================================================================================
void VoxelWorld::SetMapGenerationCallbackFunction(VoxelWorld_GenerateMap_CallbackFunction pFunc)
{
    m_pMapGenCallbackFunc = pFunc;
}

VoxelWorld_GenerateMap_CallbackFunction VoxelWorld::GetMapGenerationCallbackFunction()
{
    return m_pMapGenCallbackFunc;
}

// ============================================================================================================================
// Space conversions
// ============================================================================================================================
Vector3Int VoxelWorld::GetWorldPosition(Vector3 scenepos)
{
    Vector3Int worldpos;

    worldpos.x = (int)floor( (scenepos.x+0.5) / m_BlockSize.x );
    worldpos.y = (int)floor( (scenepos.y+0.5) / m_BlockSize.y );
    worldpos.z = (int)floor( (scenepos.z+0.5) / m_BlockSize.z );

    return worldpos;
}

Vector3Int VoxelWorld::GetChunkPosition(Vector3Int worldpos)
{
    Vector3Int chunkpos;

    chunkpos.x = (int)floor( (float)worldpos.x / m_ChunkSize.x );
    chunkpos.y = (int)floor( (float)worldpos.y / m_ChunkSize.y );
    chunkpos.z = (int)floor( (float)worldpos.z / m_ChunkSize.z );

    return chunkpos;
}

// ============================================================================================================================
// Collision/Block queries
// ============================================================================================================================
VoxelBlock* VoxelWorld::GetBlock(Vector3Int worldpos)
{
    return GetBlock( worldpos.x, worldpos.y, worldpos.z );
}

VoxelBlock* VoxelWorld::GetBlock(int worldx, int worldy, int worldz)
{
    Vector3Int chunkpos = GetChunkPosition( Vector3Int(worldx,worldy,worldz) );
    VoxelChunk* pChunk = GetActiveChunk( chunkpos );

    Vector3Int localpos( worldx%m_ChunkSize.x, worldy%m_ChunkSize.y, worldz%m_ChunkSize.z );
    if( localpos.x < 0 ) localpos.x += m_ChunkSize.x;
    if( localpos.y < 0 ) localpos.y += m_ChunkSize.y;
    if( localpos.z < 0 ) localpos.z += m_ChunkSize.z;

    return pChunk->GetBlockFromLocalPos( localpos );
}

VoxelChunk* VoxelWorld::GetChunkContainingWorldPosition(Vector3Int worldpos)
{
    Vector3Int chunkpos = GetChunkPosition( worldpos );
    VoxelChunk* pChunk = GetActiveChunk( chunkpos );

    return pChunk;
}

bool VoxelWorld::IsBlockEnabled(Vector3Int worldpos, bool blockexistsifnotready)
{
    return IsBlockEnabled( worldpos.x, worldpos.y, worldpos.z, blockexistsifnotready );
}

bool VoxelWorld::IsBlockEnabled(int worldx, int worldy, int worldz, bool blockexistsifnotready)
{
    if( worldx < (m_WorldOffset.x * m_ChunkSize.x) || worldx >= (m_WorldOffset.x + m_WorldSize.x) * m_ChunkSize.x ||
        worldy < (m_WorldOffset.y * m_ChunkSize.y) || worldy >= (m_WorldOffset.y + m_WorldSize.y) * m_ChunkSize.y ||
        worldz < (m_WorldOffset.z * m_ChunkSize.z) || worldz >= (m_WorldOffset.z + m_WorldSize.z) * m_ChunkSize.z )
    {
        return blockexistsifnotready;
    }

    Vector3Int chunkpos = GetChunkPosition( Vector3Int(worldx,worldy,worldz) );
    VoxelChunk* pChunk = GetActiveChunk( chunkpos );

    Vector3Int localpos( worldx%m_ChunkSize.x, worldy%m_ChunkSize.y, worldz%m_ChunkSize.z );
    if( localpos.x < 0 ) localpos.x += m_ChunkSize.x;
    if( localpos.y < 0 ) localpos.y += m_ChunkSize.y;
    if( localpos.z < 0 ) localpos.z += m_ChunkSize.z;

    return pChunk->IsBlockEnabled( localpos, blockexistsifnotready );
}

bool VoxelWorld::IsBlockEnabledAroundLocation(Vector3 scenepos, float radius, bool blockexistsifnotready)
{
    for( int i=0; i<4; i++ )
    {
        float xoff;
        float zoff;

        if( i == 0 )      { xoff = radius * -1; zoff = radius * -1; }
        else if( i == 1 ) { xoff = radius * -1; zoff = radius *  1; }
        else if( i == 2 ) { xoff = radius *  1; zoff = radius * -1; }
        else if( i == 3 ) { xoff = radius *  1; zoff = radius *  1; }

        Vector3 cornerscenepos( scenepos.x + xoff, scenepos.y, scenepos.z + zoff );
        if( IsBlockEnabled( GetWorldPosition( cornerscenepos ), blockexistsifnotready ) )
        {
            return true;
        }
    }

    return false;
}

float VoxelWorld::GetSceneYForNextBlockBelowPosition(Vector3 scenepos, float radius)
{
    //m_WorldSize.Set( 10, 10, 10 );
    //m_ChunkSize.Set( 16, 16, 16 );
    //m_BlockSize.Set( 1, 1, 1 );

    Vector3Int worldpos;
    float highesty = -FLT_MAX;
    for( int i=0; i<4; i++ )
    {
        float xoff;
        float zoff;

        if( i == 0 )      { xoff = radius * -1; zoff = radius * -1; }
        else if( i == 1 ) { xoff = radius * -1; zoff = radius *  1; }
        else if( i == 2 ) { xoff = radius *  1; zoff = radius * -1; }
        else if( i == 3 ) { xoff = radius *  1; zoff = radius *  1; }

        // Move player up a bit, then corner
        Vector3 cornerscenepos( scenepos.x + xoff, scenepos.y, scenepos.z + zoff );
        worldpos = GetWorldPosition( cornerscenepos );

        bool enabled = IsBlockEnabled( worldpos, true );

        while( !enabled )
        {
            worldpos.y--;
            enabled = IsBlockEnabled( worldpos, true );
        }

        float sceney = worldpos.y * m_BlockSize.y + m_BlockSize.y;

        //if( i == 0 )
        //    LOGInfo( "VoxelWorld", "Y Check: (%d,%d,%d) %0.2f\n", worldpos.x, worldpos.y, worldpos.z, sceney );

        if( sceney > highesty )
            highesty = sceney;
    }

    //LOGInfo( "VoxelWorld", "Y Check: (%d,%d,%d) %0.2f\n", worldpos.x, worldpos.y, worldpos.z, highesty );

    return highesty;
}

bool VoxelWorld::RayCastSingleBlockFindFaceHit(Vector3Int worldpos, Vector3 startpos, Vector3 endpos, Vector3* pPoint, Vector3* pNormal)
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

bool VoxelWorld::RayCast(Vector3 startpos, Vector3 endpos, float step, VoxelRayCastResult* pResult)
{
    // Lazy raycast, will pass through blocks if step too big, will always pass through corners.

    // Init some vars and figure out the length and direction of the ray we're casting.
    Vector3 currentscenepos = startpos;
    Vector3 dir = endpos - startpos;
    float len = dir.Length();
    dir.Normalize();

    // Init last worldpos to something that isn't the current world pos.
    Vector3Int lastworldpos = GetWorldPosition( currentscenepos ) + Vector3Int( 1, 1, 1 );
    
    while( true )
    {
        Vector3Int worldpos = GetWorldPosition( currentscenepos );

        // If the worldpos is different than the previous loop, check for a block.
        if( worldpos != lastworldpos && IsBlockEnabled( worldpos ) == true )
        {
            if( pResult )
            {
                pResult->m_Hit = true;
                pResult->m_BlockWorldPosition = worldpos;

                // Find the normal for the side of the block that was hit.
                RayCastSingleBlockFindFaceHit( worldpos, startpos, endpos, &pResult->m_BlockFacePoint, &pResult->m_BlockFaceNormal );
            }
            return true;
        }

        // Store the world position we just checked.
        lastworldpos = worldpos;

        // Move forward along our line.
        len -= step;
        currentscenepos += dir * step;

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

void VoxelWorld::GetMouseRayBadly(Vector2 mousepos, Vector3* start, Vector3* end)
{
    MyAssert( start != 0 );
    MyAssert( end != 0 );

    ComponentCamera* pCamera = g_pComponentSystemManager->GetFirstCamera( false );

    // Convert mouse coord into clip space.
    Vector2 mouseclip;
    mouseclip.x = 0.0f; //(mousepos.x / pCamera->m_DesiredWidth) * 2.0f - 1.0f;
    mouseclip.y = 0.0f; //(mousepos.y / pCamera->m_DesiredHeight) * 2.0f - 1.0f;

    // Convert the mouse ray into view space from clip space.
    MyMatrix invProj = pCamera->m_Camera3D.m_matProj;
    invProj.Inverse();
    Vector4 rayview = invProj * Vector4( mouseclip, -1, 1 );

    // Convert the mouse ray into world space from view space.
    MyMatrix invView = pCamera->m_Camera3D.m_matView;
    invView.Inverse();
    Vector3 rayworld = (invView * Vector4( rayview.x, rayview.y, 1, 0 )).XYZ();

    // Define the ray.
    Vector3 pos = pCamera->m_pComponentTransform->GetLocalPosition();
    Vector3 raystart = pos + rayworld * 3;
    Vector3 rayend = pos + rayworld * 10000;

    *start = raystart;
    *end = rayend;
}

// ============================================================================================================================
// Add/Remove blocks
// ============================================================================================================================
void VoxelWorld::ChangeBlockState(Vector3Int worldpos, unsigned int type, bool enabled)
{
    Vector3Int chunkpos = GetChunkPosition( worldpos );
    VoxelChunk* pChunk = GetActiveChunk( chunkpos );

    pChunk->ChangeBlockState( worldpos, type, enabled );

    pChunk->RebuildMesh( 1 );

    VoxelChunk* pChunksToRebuild[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int numchunkstorebuild = 0;

    // Create a list of up to 7 chunks to rebuild
    for( int x = -1; x <= 1; x++ )
    {
        for( int y = -1; y <= 1; y++ )
        {
            for( int z = -1; z <= 1; z++ )
            {
                Vector3Int neighbourpos = worldpos + Vector3Int( x, y, z );

                Vector3Int neighbourchunkpos = GetChunkPosition( neighbourpos );
                if( IsChunkActive( neighbourchunkpos ) )
                {
                    VoxelChunk* pNeighbourChunk = GetActiveChunk( neighbourchunkpos );
                    if( pNeighbourChunk && pNeighbourChunk != pChunk )
                    {
                        int i = 0;
                        while( true )
                        {
                            if( pChunksToRebuild[i] == pNeighbourChunk || pChunksToRebuild[i] == 0 )
                                break;

                            i++;
                        }
                        MyAssert( i < 7 );
                        pChunksToRebuild[i] = pNeighbourChunk;
                    }
                }
            }
        }
    }

    // rebuild up to 7 chunks
    int chunksrebuilt = 1;
    for( int i=0; i<7; i++ )
    {
        if( pChunksToRebuild[i] )
        {
            pChunksToRebuild[i]->RebuildMesh( 1 );
            chunksrebuilt++;
        }
    }

    LOGInfo( LOGTag, "Chunks rebuilt: %d\n", chunksrebuilt );
}
