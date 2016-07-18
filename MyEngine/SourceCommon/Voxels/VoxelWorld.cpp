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

VoxelWorld::VoxelWorld()
{
    m_NumChunkPointersAllocated = 0;
    m_pActiveWorldChunkPtrs = 0;

    m_WorldSize.Set( 0, 0, 0 );
    m_ChunkSize.Set( 16, 16, 16 );
    m_BlockSize.Set( 1, 1, 1 );

    m_WorldOffset.Set( 0, 0, 0 );
}

VoxelWorld::~VoxelWorld()
{
    delete[] m_pActiveWorldChunkPtrs;

    for( CPPListNode* pNode = m_pChunksFree.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        delete pChunk;
    }

    for( CPPListNode* pNode = m_pChunksLoading.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        delete pChunk;
    }    

    for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;
        pNode = pNode->GetNext();

        delete pChunk;
    }
}

void VoxelWorld::Initialize(Vector3Int visibleworldsize)
{
    // Make sure init is only called once.
    MyAssert( m_NumChunkPointersAllocated == 0 );
    
    SetWorldSize( visibleworldsize );

    Vector3 chunkoffset = m_ChunkSize.MultiplyComponents( m_BlockSize );

    // make 0,0,0 the bottom left corner. TODO: pass in an offset
    m_WorldOffset.Set( 0, 0, 0 );

    for( int z=0; z<m_WorldSize.z; z++ )
    {
        for( int y=0; y<m_WorldSize.y; y++ )
        {
            for( int x=0; x<m_WorldSize.x; x++ )
            {
                VoxelChunk* pChunk = MyNew VoxelChunk;
                m_pChunksFree.AddHead( pChunk );

                PrepareChunk( Vector3Int( x, y, z ) );
            }
        }
    }
}

static Vector3Int g_WorldCenterChunkOffset;
signed char ChunkDistanceCmpFunc(CPPListNode *a, CPPListNode *b)
{
    Vector3Int offseta = ((VoxelChunk*)a)->GetChunkOffset();
    Vector3Int offsetb = ((VoxelChunk*)b)->GetChunkOffset();

    int distancea = (offseta - g_WorldCenterChunkOffset).LengthSquared();
    int distanceb = (offsetb - g_WorldCenterChunkOffset).LengthSquared();

    if( distancea == distanceb )
        return 0;

    if( distancea < distanceb )
        return -1;

    return 1;
}

void VoxelWorld::Tick(double timepassed)
{
    // Sort chunks based on distance from world center (which is likely the player location)
    g_WorldCenterChunkOffset = (m_WorldOffset + m_WorldSize/2).MultiplyComponents( m_ChunkSize );
    m_pChunksLoading.Sort( ChunkDistanceCmpFunc );

    // build the mesh for a single chunk per frame.
    VoxelChunk* pChunk = (VoxelChunk*)m_pChunksLoading.GetHead();

    if( pChunk )
    {
        pChunk->CreateMap();
        pChunk->RebuildMesh();
        m_pChunksVisible.MoveTail( pChunk );
        return;
    }

    //pChunk = (VoxelChunk*)m_pChunksVisible.GetHead();
    //while( pChunk )
    //{
    //    pChunk->RebuildMesh();
    //    pChunk = (VoxelChunk*)pChunk->GetNext();
    //}
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
        m_pActiveWorldChunkPtrs = MyNew VoxelChunk*[pointersneeded];
    }

    m_WorldSize = visibleworldsize;
}

void VoxelWorld::SetWorldCenter(Vector3 scenepos)
{
    Vector3Int worldpos;
    worldpos.x = (int)(scenepos.x / m_BlockSize.x) / m_ChunkSize.x;
    worldpos.y = (int)(scenepos.y / m_BlockSize.y) / m_ChunkSize.x;
    worldpos.z = (int)(scenepos.z / m_BlockSize.z) / m_ChunkSize.x;

    SetWorldCenter( worldpos );
}

void VoxelWorld::SetWorldCenter(Vector3Int newworldcenter)
{
#if MYFW_PROFILING_ENABLED
    static double Timing_LastFrameTime = 0;

    double Timing_Start = MyTime_GetSystemTime();
#endif

    Vector3Int currentworldcenter = m_WorldOffset + m_WorldSize/2;
    if( newworldcenter == currentworldcenter )
        return;

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

#if MYFW_PROFILING_ENABLED
    double Timing_End = MyTime_GetSystemTime();

    float functime = (float)((Timing_End - Timing_Start)*1000);

    LOGInfo( "VoxelWorld", "SetWorldCenter functime: %0.2f\n", functime );
#endif
}

void VoxelWorld::UpdateVisibility(void* pUserData)
{
    // TODO: Update list of visible chunks

    // Add all visible chunks to scene graph
    for( CPPListNode* pNode = m_pChunksVisible.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        VoxelChunk* pChunk = (VoxelChunk*)pNode;

        pChunk->AddToSceneGraph( pUserData );
    }
}

// ============================================================================================================================
// Protected/Internal functions
// ============================================================================================================================
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

void VoxelWorld::PrepareChunk(Vector3Int chunkpos)
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

    pChunk->Initialize( this, chunkposition, m_ChunkSize, chunkblockoffset );

    m_pChunksLoading.MoveTail( pChunk );
}

void VoxelWorld::ShiftChunk(Vector3Int to, Vector3Int from, bool isedgeblock)
{
    int tooffset = to.z * m_WorldSize.y * m_WorldSize.x + to.y * m_WorldSize.x + to.x;
                    
    VoxelChunk* pChunk = m_pActiveWorldChunkPtrs[tooffset];
    if( pChunk )
    {
        m_pChunksFree.MoveTail( pChunk );
        pChunk->RemoveFromSceneGraph();
    }

    if( isedgeblock )
    {
        PrepareChunk( m_WorldOffset + to );
    }
    else
    {
        int fromoffset = from.z * m_WorldSize.y * m_WorldSize.x + from.y * m_WorldSize.x + from.x;
        m_pActiveWorldChunkPtrs[tooffset] = m_pActiveWorldChunkPtrs[fromoffset];
        m_pActiveWorldChunkPtrs[fromoffset] = 0;
    }
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

    return pChunk->IsBlockEnabled( worldx, worldy, worldz, blockexistsifnotready );
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
        Vector3 cornerscenepos( scenepos.x + xoff, scenepos.y + m_BlockSize.y * 1.1f, scenepos.z + zoff );
        worldpos = GetWorldPosition( cornerscenepos );
        //if( i == 0 )
        //    LOGInfo( "VoxelWorld", "Y Check: (%d,%d,%d)\n", worldpos.x, worldpos.y, worldpos.z );

        bool enabled = IsBlockEnabled( worldpos, true );

        while( !enabled )
        {
            worldpos.y--;
            enabled = IsBlockEnabled( worldpos, true );
        }

        float sceney = worldpos.y * m_BlockSize.y + m_BlockSize.y;

        if( sceney > highesty )
            highesty = sceney;
    }

    return highesty;
}

bool VoxelWorld::RaycastSingleBlockFindFaceHit(Vector3Int worldpos, Vector3 startpos, Vector3 endpos, Vector3* pPoint, Vector3* pNormal)
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

    for( int i=0; i<6; i++ )
    {
        switch( i )
        {
        case 0: plane.Set( Vector3(-1,0,0), Vector3(-m_BlockSize.x/2, 0, 0) ); break;
        case 1: plane.Set( Vector3( 1,0,0), Vector3( m_BlockSize.x/2, 0, 0) ); break;
        case 2: plane.Set( Vector3(0,-1,0), Vector3(0, -m_BlockSize.y/2, 0) ); break;
        case 3: plane.Set( Vector3(0, 1,0), Vector3(0,  m_BlockSize.y/2, 0) ); break;
        case 4: plane.Set( Vector3(0,0,-1), Vector3(0, 0, -m_BlockSize.z/2) ); break;
        case 5: plane.Set( Vector3(0,0, 1), Vector3(0, 0,  m_BlockSize.z/2) ); break;
        }

        plane.IntersectRay( startpos, endpos, &result );

        if( ( i >=0 && i <= 1 && result.y > -0.5 && result.y < 0.5 && result.z > -0.5 && result.z < 0.5 ) ||
            ( i >=2 && i <= 3 && result.x > -0.5 && result.x < 0.5 && result.z > -0.5 && result.z < 0.5 ) ||
            ( i >=4 && i <= 5 && result.x > -0.5 && result.x < 0.5 && result.y > -0.5 && result.y < 0.5 ) )
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

bool VoxelWorld::Raycast(Vector3 startpos, Vector3 endpos, float step, VoxelRaycastResult* pResult)
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
                RaycastSingleBlockFindFaceHit( worldpos, startpos, endpos, &pResult->m_BlockFacePoint, &pResult->m_BlockFaceNormal );
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
    mouseclip.x = (mousepos.x / pCamera->m_WindowWidth) * 2.0f - 1.0f;
    mouseclip.y = (mousepos.y / pCamera->m_WindowHeight) * 2.0f - 1.0f;

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
void VoxelWorld::ChangeBlockState(Vector3Int worldpos, bool enabled)
{
    Vector3Int chunkpos = GetChunkPosition( worldpos );
    VoxelChunk* pChunk = GetActiveChunk( chunkpos );

    pChunk->ChangeBlockState( worldpos, enabled );

    pChunk->RebuildMesh();

    // Check 6 neighbours, and rebuild them if applicable.
    for( int i=0; i<6; i++ )
    {
        Vector3Int neighbourpos = worldpos;

        if( i == 0 ) neighbourpos.x += 1;
        if( i == 1 ) neighbourpos.x -= 1;
        if( i == 2 ) neighbourpos.y += 1;
        if( i == 3 ) neighbourpos.y -= 1;
        if( i == 4 ) neighbourpos.z += 1;
        if( i == 5 ) neighbourpos.z -= 1;

        Vector3Int neighbourchunkpos = GetChunkPosition( neighbourpos );
        VoxelChunk* pNeighbourChunk = GetActiveChunk( neighbourchunkpos );
        if( pNeighbourChunk && pNeighbourChunk != pChunk )
        {
            pNeighbourChunk->RebuildMesh();
        }
    }
}