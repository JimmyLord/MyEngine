//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentHeightmap.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/Interfaces/EditorInterface_HeightmapEditor.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4996 )
#include "../../Libraries/LodePNG/lodepng.h"
#pragma warning( pop )

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentHeightmap ); //_VARIABLE_LIST

ComponentHeightmap::ComponentHeightmap(ComponentSystemManager* pComponentSystemManager)
: ComponentMesh( pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_pHeightmapTexture = nullptr;
    m_Heights = nullptr;
    m_WaitingForTextureFileToFinishLoading = false;
}

ComponentHeightmap::~ComponentHeightmap()
{
    SAFE_DELETE_ARRAY( m_Heights );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Tick );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnSurfaceChanged );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Draw );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnTouch );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnButtons );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnKeys );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnFileRenamed );
}

void ComponentHeightmap::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentHeightmap* pThis) //_VARIABLE_LIST
{
    ComponentMesh::RegisterVariables( pList, pThis );

    AddVar( pList, "Size", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Size ), true, true, "Size", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "VertCount", ComponentVariableType_Vector2Int, MyOffsetOf( pThis, &pThis->m_VertCount ), true, true, "VertCount", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "HeightmapTexture", ComponentVariableType_TexturePtr, MyOffsetOf( pThis, &pThis->m_pHeightmapTexture ), true, true, "Texture", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, (CVarFunc_DropTarget)&ComponentHeightmap::OnDrop, nullptr );
}

void ComponentHeightmap::Reset()
{
    ComponentMesh::Reset();

    m_Size.Set( 10.0f, 10.0f );
    m_VertCount.Set( 128, 128 );
    SAFE_RELEASE( m_pHeightmapTexture );
    m_HeightmapTextureSize.Set( 0, 0 );
}

#if MYFW_USING_LUA
void ComponentHeightmap::LuaRegister(lua_State* luaState)
{
    luabridge::getGlobalNamespace( luaState )
        .beginClass<ComponentHeightmap>( "ComponentHeightmap" )
            //.addData( "m_SampleVector3", &ComponentHeightmap::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentHeightmap::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
void* ComponentHeightmap::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        oldPointer = m_pHeightmapTexture;
        SetHeightmapTexture( (TextureDefinition*)pDropItem->m_Value );

        // Rebuild the heightmap.
        CreateHeightmap();
    }

    return oldPointer;
}

void* ComponentHeightmap::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_Size ) )
    {
        CreateHeightmap();
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_VertCount ) )
    {
        CreateHeightmap();
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_pHeightmapTexture ) )
    {
        CreateHeightmap();
    }

    return oldPointer;
}

void ComponentHeightmap::OnButtonEditHeightmap()
{
    g_pEngineCore->SetEditorInterface( EditorInterfaceType_HeightmapEditor );
    ((EditorInterface_HeightmapEditor*)g_pEngineCore->GetCurrentEditorInterface())->SetHeightmap( this );
}
#endif //MYFW_EDITOR

//cJSON* ComponentHeightmap::ExportAsJSONObject(bool saveSceneID, bool saveID)
//{
//    cJSON* jComponent = ComponentMesh::ExportAsJSONObject( saveSceneID, saveID );
//
//    return jComponent;
//}
//
//void ComponentHeightmap::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
//{
//    ComponentMesh::ImportFromJSONObject( jComponent, sceneID );
//}

void ComponentHeightmap::FinishImportingFromJSONObject(cJSON* jComponent)
{
    // This will be hit on initial load and on quickload.
    // Only create the mesh on initial load.
    // TODO: Also will rebuild if changes are made during runtime inside editor.
    if( m_pMesh == nullptr
//#if MYFW_EDITOR
//        || m_PrimitiveSettingsChangedAtRuntime
//#endif
        )
    {
        CreateHeightmap();
    }
}

void ComponentHeightmap::AddAllVariablesToWatchPanel()
{
    ComponentBase::AddAllVariablesToWatchPanel();

    if( m_VertCount != m_HeightmapTextureSize )
    {
        ImGui::Text( "TextureSize (%dx%d) doesn't match.", m_HeightmapTextureSize.x, m_HeightmapTextureSize.y );
        if( ImGui::Button( "Change vertCount" ) )
        {
            // TODO: Undo.
            m_VertCount = m_HeightmapTextureSize;
            GenerateHeightmapMesh();
        }
    }

    if( m_Heights != nullptr )
    {
        if( ImGui::Button( "Edit Heightmap" ) )
        {
            OnButtonEditHeightmap();
        }
    }
}

ComponentHeightmap& ComponentHeightmap::operator=(const ComponentHeightmap& other)
{
    MyAssert( &other != this );

    ComponentMesh::operator=( other );

    // TODO: Replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_Size = other.m_Size;
    m_VertCount = other.m_VertCount;
    SetHeightmapTexture( other.m_pHeightmapTexture );

    return *this;
}

void ComponentHeightmap::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnFileRenamed );
    }
}

void ComponentHeightmap::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

//void ComponentHeightmap::TickCallback(float deltaTime)
//{
//}

void ComponentHeightmap::SetHeightmapTexture(TextureDefinition* pTexture)
{
    UnregisterFileLoadingCallback();
    m_HeightmapTextureSize.Set( 0, 0 );
    SAFE_DELETE( m_Heights );

    if( pTexture )
        pTexture->AddRef();
    SAFE_RELEASE( m_pHeightmapTexture );
    m_pHeightmapTexture = pTexture;
}

void ComponentHeightmap::UnregisterFileLoadingCallback()
{
    if( m_WaitingForTextureFileToFinishLoading )
    {
        m_pHeightmapTexture->GetFile()->UnregisterFileFinishedLoadingCallback( this );
        m_WaitingForTextureFileToFinishLoading = false;
    }
}

void ComponentHeightmap::OnFileFinishedLoadingHeightmapTexture(MyFileObject* pFile) // StaticOnFileFinishedLoadingHeightmapTexture
{
    CreateHeightmap();
    UnregisterFileLoadingCallback();
}

void ComponentHeightmap::CreateHeightmap()
{
    if( m_pMesh == nullptr )
    {
        m_pMesh = MyNew MyMesh( m_pComponentSystemManager->GetEngineCore() );
    }
    else
    {
        RemoveFromRenderGraph();
    }

    if( m_pMesh->GetSubmeshListCount() > 0 )
        m_pMesh->GetSubmesh( 0 )->m_PrimitiveType = m_GLPrimitiveType;

    // Generate the actual heightmap.
    if( GenerateHeightmapMesh() )
    {
        m_GLPrimitiveType = m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;

        // Add the Mesh to the main render graph.
        AddToRenderGraph();
    }
    else
    {
        RemoveFromRenderGraph();
    }
}

// Returns true if successfully generated mesh.
bool ComponentHeightmap::GenerateHeightmapMesh()
{
    //LOGInfo( LOGTag, "ComponentHeightmap::GenerateHeightmapMesh\n" );

    // Sanity check on vertex count.
    if( m_VertCount.x <= 0 || m_VertCount.y <= 0 || (uint64)m_VertCount.x * (uint64)m_VertCount.y > UINT_MAX )
    {
        LOGError( LOGTag, "vertCount can't be negative.\n" );
        return false;
    }

    // If the heightmap texture is still loading, register a callback.
    if( m_pHeightmapTexture->GetFile()->IsFinishedLoading() == false )
    {
        if( m_WaitingForTextureFileToFinishLoading == false )
        {
            m_WaitingForTextureFileToFinishLoading = true;
            m_pHeightmapTexture->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingHeightmapTexture );
        }
        return false;
    }

    // Set the parameters. // TODO: Make some of these members.
    Vector2 size = m_Size;
    Vector2Int vertCount = m_VertCount;
    Vector3 bottomLeftPos( 0, 0, 0 ); // Collision methods will fail if this changes.
    Vector2 uvStart( 0, 0 );
    Vector2 uvRange( 0, 0 );
    bool createTriangles = (m_GLPrimitiveType == MyRE::PrimitiveType_Points) ? false : true;

    // Calculate the number of triangles, vertices and indices.
    // TODO: Rewrite this to use triangle strips.
    unsigned int numTris = (vertCount.x - 1) * (vertCount.y - 1) * 2;
    unsigned int numVerts = vertCount.x * vertCount.y;
    unsigned int numIndices = createTriangles ? numTris * 3 : numVerts; // 3 per triangle or 1 per point.

    // Reinitialize the submesh properties along with the vertex and index buffers.
    m_pMesh->RebuildShapeBuffers( numVerts, VertexFormat_XYZUVNorm, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U32, "MyMesh_Plane" );
    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_pMesh->GetSubmesh( 0 )->m_pVertexBuffer->GetData( true );
    unsigned int* pIndices = (unsigned int*)m_pMesh->GetSubmesh( 0 )->m_pIndexBuffer->GetData( true );

    // Generate the vertex positions
    {
        unsigned char* pixelBuffer = nullptr;
        unsigned int texWidth, texHeight;

        // Decode the file into a raw buffer.
        {
            unsigned char* buffer = (unsigned char*)m_pHeightmapTexture->GetFile()->GetBuffer();
            MyAssert( buffer != nullptr );

            int length = m_pHeightmapTexture->GetFile()->GetFileLength();

            unsigned int error = lodepng_decode32( &pixelBuffer, &texWidth, &texHeight, buffer, length );
            MyAssert( error == 0 );
        }

        // Store the heightmap texture size, so we can show a warning if there's a mismatch.
        m_HeightmapTextureSize.Set( (int)texWidth, (int)texHeight );
        
        // Allocate a buffer to store the vertex heights.
        SAFE_DELETE_ARRAY( m_Heights );
        m_Heights = MyNew float[vertCount.x * vertCount.y];

        // Set the vertices.
        for( int y = 0; y < vertCount.y; y++ )
        {
            for( int x = 0; x < vertCount.x; x++ )
            {
                unsigned int index = (unsigned int)(y * vertCount.x + x);

                Vector2 texCoord( (float)x/vertCount.x * texWidth, (float)y/vertCount.y * texHeight );
                int texIndex = (int)( (int)texCoord.y * texWidth + (int)texCoord.x );
                float height = pixelBuffer[texIndex*4] / 255.0f;

                m_Heights[index] = height;

                pVerts[index].pos.x = bottomLeftPos.x + size.x / (vertCount.x - 1) * x;
                pVerts[index].pos.y = bottomLeftPos.y + height;
                pVerts[index].pos.z = bottomLeftPos.z + size.y / (vertCount.y - 1) * y;

                pVerts[index].uv.x = uvStart.x + x * uvRange.x / (vertCount.x - 1);
                pVerts[index].uv.y = uvStart.y + y * uvRange.y / (vertCount.y - 1);

                if( createTriangles == false )
                {
                    pIndices[index] = index;
                }
            }
        }

        // Free the memory allocated by lodepng_decode32.
        free( pixelBuffer );
    }

    // Calculate normals.
    int mx = vertCount.x-1;
    int my = vertCount.y-1;
    for( int y = 0; y < vertCount.y; y++ )
    {
        for( int x = 0; x < vertCount.x; x++ )
        {
            //   TL--TC---TR
            //     \  |  /  
            //      \ | /   
            //        C     
            //      / | \   
            //     /  |  \  
            //   BL--BC---BR

            unsigned int indexC = (unsigned int)(y * vertCount.x + x);
            unsigned int indexTL = y < my && x > 0  ? (unsigned int)((y+1) * vertCount.x + x-1) : indexC;
            unsigned int indexTC = y < my           ? (unsigned int)((y+1) * vertCount.x + x  ) : indexC;
            unsigned int indexTR = y < my && x < mx ? (unsigned int)((y+1) * vertCount.x + x+1) : indexC;
            unsigned int indexBL = y > 0  && x > 0  ? (unsigned int)((y-1) * vertCount.x + x-1) : indexC;
            unsigned int indexBC = y > 0            ? (unsigned int)((y-1) * vertCount.x + x  ) : indexC;
            unsigned int indexBR = y > 0  && x < mx ? (unsigned int)((y-1) * vertCount.x + x+1) : indexC;

            Vector3 posC = pVerts[indexC].pos;
            Vector3 normalTL = (pVerts[indexTL].pos - posC).Cross( pVerts[indexTC].pos - posC );
            Vector3 normalTR = (pVerts[indexTC].pos - posC).Cross( pVerts[indexTR].pos - posC );
            Vector3 normalBL = (pVerts[indexBR].pos - posC).Cross( pVerts[indexBC].pos - posC );
            Vector3 normalBR = (pVerts[indexBC].pos - posC).Cross( pVerts[indexBL].pos - posC );
            
            pVerts[indexC].normal = (normalTL + normalTR + normalBL + normalBR) / 4.0f;
            pVerts[indexC].normal.Normalize();
        }
    }

    // Setup indices.
    if( createTriangles )
    {
        for( int y = 0; y < vertCount.y - 1; y++ )
        {
            for( int x = 0; x < vertCount.x - 1; x++ )
            {
                int elementIndex = (y * (vertCount.x-1) + x) * 6;
                unsigned int vertexIndex = (unsigned int)(y * vertCount.x + x);

                // BL - TL - TR.
                pIndices[ elementIndex + 0 ] = vertexIndex;
                pIndices[ elementIndex + 1 ] = vertexIndex + (unsigned int)vertCount.x;
                pIndices[ elementIndex + 2 ] = vertexIndex + (unsigned int)vertCount.x + 1;

                // BL - TR - BR.
                pIndices[ elementIndex + 3 ] = vertexIndex;
                pIndices[ elementIndex + 4 ] = vertexIndex + (unsigned int)vertCount.x + 1;
                pIndices[ elementIndex + 5 ] = vertexIndex + 1;
            }
        }
    }

    // Calculate the bounding box.
    Vector3 center( bottomLeftPos.x + size.x/2, bottomLeftPos.y, bottomLeftPos.z + size.y/2 );
    m_pMesh->GetBounds()->Set( center, Vector3(size.x/2, 0, size.y/2) );

    m_pMesh->SetReady();

    // TODO: Just a test. Remove me.
    GetHeightAtWorldXZ( 0, 0, nullptr );

    return true;
}

bool ComponentHeightmap::GetTileCoordsAtWorldXZ(const float x, const float z, Vector2Int* pLocalTile, Vector2* pPercIntoTile) const
{
    ComponentTransform* pTransform = this->m_pGameObject->GetTransform();
    MyAssert( pTransform );

    // Get the local position.
    MyMatrix* pWorldMat = pTransform->GetWorldTransform();
    Vector3 localPos = pWorldMat->GetInverse() * Vector3( x, 0, z );

    // Get the tile coordinates.
    Vector2Int tileCoords = (m_VertCount-1) * (localPos.XZ()/m_Size);

    if( pLocalTile )
        pLocalTile->Set( tileCoords.x, tileCoords.y );

    if( tileCoords.x < 0 || tileCoords.x >= m_VertCount.x ||
        tileCoords.y < 0 || tileCoords.y >= m_VertCount.y )
    {
        //LOGInfo( LOGTag, "ComponentHeightmap::GetHeightAtWorldXZ: Out of bounds" );
        return false;
    }

    if( pPercIntoTile )
    {
        Vector2 tileSize( m_Size.x / (m_VertCount.x-1), m_Size.y / (m_VertCount.y-1) );
        pPercIntoTile->x = (localPos.x - tileCoords.x * tileSize.x) / tileSize.x;
        pPercIntoTile->y = (localPos.z - tileCoords.y * tileSize.y) / tileSize.y;
    }

    //LOGInfo( LOGTag, "ComponentHeightmap::GetTileCoordsAtWorldXZ: (%d,%d)", posIndex.x, posIndex.y );

    return true;
}

bool ComponentHeightmap::GetHeightAtWorldXZ(const float x, const float z, float* pFloat) const
{
    float height = 0.0f;

    Vector2Int tileCoords;
    Vector2 percIntoTile;
    if( GetTileCoordsAtWorldXZ( x, z, &tileCoords, &percIntoTile ) )
    {
        // Found here: https://codeplea.com/triangular-interpolation
        //   and here: https://www.youtube.com/watch?v=6E2zjfzMs7c
        // ----
        // | /|  Left triangle X < Y
        // |/ |  Right triangle X > Y
        // ----
        Vector3 p1, p2, p3;
        if( percIntoTile.x <= percIntoTile.y ) // Left triangle X < Y
        {
            p1 = Vector3( 0, m_Heights[(tileCoords.y    ) * m_VertCount.x + tileCoords.x    ], 0 ); // BL
            p2 = Vector3( 0, m_Heights[(tileCoords.y + 1) * m_VertCount.x + tileCoords.x    ], 1 ); // TL
            p3 = Vector3( 1, m_Heights[(tileCoords.y + 1) * m_VertCount.x + tileCoords.x + 1], 1 ); // TR
        }
        else //if( percIntoTile.x > percIntoTile.y ) // Right triangle X > Y
        {
            p1 = Vector3( 0, m_Heights[(tileCoords.y    ) * m_VertCount.x + tileCoords.x    ], 0 ); // BL
            p2 = Vector3( 1, m_Heights[(tileCoords.y + 1) * m_VertCount.x + tileCoords.x + 1], 1 ); // TR
            p3 = Vector3( 1, m_Heights[(tileCoords.y    ) * m_VertCount.x + tileCoords.x + 1], 0 ); // BR
        }

        // Barycentric interpolation.
        float divisor = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
        float w1 = ( (p2.z - p3.z) * (percIntoTile.x - p3.x) + (p3.x - p2.x) * (percIntoTile.y - p3.z) ) / divisor;
        float w2 = ( (p3.z - p1.z) * (percIntoTile.x - p3.x) + (p1.x - p3.x) * (percIntoTile.y - p3.z) ) / divisor;
        float w3 = 1.0f - w1 - w2;
        float height = (w1 * p1.y) + (w2 * p2.y) + (w3 * p3.y);

        if( pFloat )
            *pFloat = height;

        //LOGInfo( LOGTag, "ComponentHeightmap::GetHeightAtWorldXZ: (%d,%d) %f", tileCoords.x, tileCoords.y, m_Heights[index] );

        return true;
    }

    return false;
}

bool ComponentHeightmap::SnapToBounds(Vector3 start, const Vector3& dir, Vector3* pResult) const
{
    // Terrain bounds.
    float minX = 0;
    float maxX = m_Size.x;
    float minZ = 0;
    float maxZ = m_Size.y;

    // If starting outside the bounds, kick out early.
    if( start.x > maxX && dir.x > 0 ) return false;
    if( start.z > maxZ && dir.z > 0 ) return false;
    if( start.x < minX && dir.x < 0 ) return false;
    if( start.z < minZ && dir.z < 0 ) return false;

    if( start.x < 0 && dir.x > 0 ) // Left of bounds moving right.
    {
        float steps = (minX - start.x) / dir.x;
        start += steps * dir;
    }
    else if( start.x > maxX && dir.x < 0 ) // Right of bounds moving left.
    {
        float steps = (maxX - start.x) / dir.x;
        start += steps * dir;
    }
    if( start.z > maxZ && dir.z > 0 ) return false;
    if( start.z < minZ && dir.z < 0 ) return false;

    if( start.z < 0 && dir.z > 0 ) // Below bounds moving up.
    {
        float steps = (minZ - start.z) / dir.z;
        start += steps * dir;
    }
    else if( start.z > maxZ && dir.z < 0 ) // Above bounds moving down.
    {
        float steps = (maxZ - start.z) / dir.z;
        start += steps * dir;
    }
    if( start.x > maxX && dir.x > 0 ) return false;
    if( start.x < minX && dir.x < 0 ) return false;

    *pResult = start;
    return true;
}

bool ComponentHeightmap::FindCollisionPoint(const Vector3& currentPosition, const Vector3& start, const Vector3& dir, const Vector2Int& tile1, const Vector2Int& tile2, Vector3* pResult) const
{
    // TODO: Currently only deals with x>z direction vector where z is positive.

    // -----2B----  d = dir vector.
    // |  / |  / |
    // | / dd /  |  If x > z, make the vector tileSize long on the x-axis.
    // 1Bdd-2A----  Travel from tile edge to tile edge, if we change row then check the height of 3 tiles,
    // dd / |  / |    otherwise test current tile and the one to the left.
    // | /  | /  |
    // 1A---------  1A/B,2A/B = heights from heightmap to check against.
    int tile1AIndex = tile1.y     * m_VertCount.x + tile1.x; // Previous tile.
    int tile1BIndex = (tile1.y+1) * m_VertCount.x + tile1.x; // Tile above previous tile.
    int tile2AIndex = tile2.y     * m_VertCount.x + tile2.x; // Start edge of current tile.
    int tile2BIndex = (tile2.y+1) * m_VertCount.x + tile2.x; // Tile above current tile.

    // If all heights are below our position, then don't check triangles.
    {
        bool checkTriangles = false;

        if( tile1.x >= 0 && tile1.x < m_VertCount.x && tile1.y >= 0 && tile1.y < m_VertCount.y )
        {
            if( currentPosition.y < m_Heights[tile1AIndex] )
                checkTriangles = true;

            if( (tile1.y+1) < m_VertCount.y && currentPosition.y < m_Heights[tile1BIndex] )
                checkTriangles = true;
        }
        if( tile2.x >= 0 && tile2.x < m_VertCount.x && tile2.y >= 0 && tile2.y < m_VertCount.y )
        {
            if( currentPosition.y < m_Heights[tile2AIndex] )
                checkTriangles = true;

            if( (tile2.y+1) < m_VertCount.y && currentPosition.y < m_Heights[tile2BIndex] )
                checkTriangles = true;
        }

        if( checkTriangles == false )
            return false;
    }

    // Cast ray against the 2 triangles between these 4 points.
    {
        Vector2 tileSize( m_Size.x / (m_VertCount.x-1), m_Size.y / (m_VertCount.y-1) );

        // 2 cases for x>z with positive z.
        //     2B
        //    / |
        //   / dd
        // 1Bdd2A   or   1B--2B
        // dd /          |  /dd
        // | /           dd/  |
        // 1A            1A--2A
        Vector3 tile1APos( tile1.x * tileSize.x, m_Heights[tile1AIndex], tile1.y     * tileSize.y );
        Vector3 tile1BPos( tile1.x * tileSize.x, m_Heights[tile1BIndex], (tile1.y+1) * tileSize.y );
        Vector3 tile2APos( tile2.x * tileSize.x, m_Heights[tile2AIndex], tile2.y     * tileSize.y );
        Vector3 tile2BPos( tile2.x * tileSize.x, m_Heights[tile2BIndex], (tile2.y+1) * tileSize.y );

        Plane plane;
        Vector3 result;
        Vector3 normal;

        // Left triangle. 1A-1B-2A or 1A-1B-2B
        if( tile1.y != tile2.y )
            normal = (tile1BPos - tile1APos).Cross( tile2APos - tile1APos ); // 1A-1B-2A
        else
            normal = (tile1BPos - tile1APos).Cross( tile2BPos - tile1APos ); // 1A-1B-2B
        plane.Set( normal, tile1APos );
        if( plane.IntersectRay( currentPosition, dir, &result ) )
        {
            if( result.x >= tile1APos.x && result.x < tile2APos.x &&
                result.z >= tile1APos.z && result.z < tile1BPos.z &&
                result.x - tile1.x * tileSize.x < result.z - tile1.y * tileSize.y )
            {
                if( pResult )
                    *pResult = result;
                return true;
            }
        }

        // Right triangle. 1B-2B-2A or 1A-2B-2A
        if( tile1.y != tile2.y )
            normal = (tile2BPos - tile1BPos).Cross( tile2APos - tile1BPos ); // 1B-2B-2A
        else
            normal = (tile2BPos - tile1APos).Cross( tile2APos - tile1APos ); // 1A-2B-2A
        plane.Set( normal, tile2APos );
        if( plane.IntersectRay( currentPosition, dir, &result ) )
        {
            if( result.x >= tile1APos.x && result.x < tile2APos.x &&
                result.z >= tile2APos.z && result.z < tile2BPos.z &&
                result.x - tile1.x * tileSize.x > result.z - tile2.y * tileSize.y )
            {
                if( pResult )
                    *pResult = result;
                return true;
            }
        }
    }

    return false;
}

bool ComponentHeightmap::RayCast(Vector3 start, Vector3 end, Vector3* pResult) const
{
    // Move ray into terrain space.
    ComponentTransform* pTransform = this->m_pGameObject->GetTransform();
    MyAssert( pTransform );
    MyMatrix* pWorldMat = pTransform->GetWorldTransform();
    start = pWorldMat->GetInverse() * start;
    end = pWorldMat->GetInverse() * end;

    // Get the direction vector.
    Vector3 dir = (end - start).GetNormalized();

    Vector3 currentPosition = start;

    // Snap start to the edge of the first tile it hits.
    // If the vector doesn't collide with the heightmap at all, kick out.
    if( SnapToBounds( start, dir, &currentPosition ) == false )
        return false;
    //SnapToBounds( end, -dir, &end );

    // Get the tile coords.
    Vector2Int tileCoords = (m_VertCount-1) * (currentPosition.XZ()/m_Size);
    MyAssert( tileCoords.x >= 0 && tileCoords.x < m_VertCount.x && tileCoords.y >= 0 && tileCoords.y < m_VertCount.y );
    int tileIndex = tileCoords.y * m_VertCount.x + tileCoords.x;

    // If our currentPosition tile is below the heightmap, kick out.
    if( currentPosition.y < m_Heights[tileIndex] )
    {
        return false;
    }

    // Calculate the tile size. TODO: Make this a member?
    Vector2 tileSize( m_Size.x / (m_VertCount.x-1), m_Size.y / (m_VertCount.y-1) );

    // -----2-----  d = dir vector.
    // |  / |  / |
    // | / dd /  |  If x > z, make the vector tileSize long on the x-axis.
    // 2-dd-1-----  Travel from tile edge to tile edge, if we change row then check the height of 3 tiles,
    // dd / |  / |    otherwise test current tile and the one to the left.
    // | /  | /  |
    // 1----------  1,2 = heights from heightmap to check against.
    if( fabs(dir.x) > fabs(dir.z) )
    {
        // Make vector 'tileSize' long on the x-axis.
        dir = dir / dir.x * tileSize.x;
        Vector2 tilePos( (float)tileCoords.x, currentPosition.z/m_Size.y * (m_VertCount.y-1) );
        Vector2 tileIncrement( 1, dir.z / dir.x );
        Vector2Int lastTileCoords( -1, -1 );

        if( dir.x > 0 )
        {
            while( tileCoords.x < m_VertCount.x )
            {
                tileCoords.Set( (int)tilePos.x, (int)tilePos.y );
                if( tileCoords.x >= 0 && tileCoords.x < m_VertCount.x && tileCoords.y >= 0 && tileCoords.y < m_VertCount.y )
                {
                    // Test for collisions in up to 2 edges. //lastTileCoords and WithY+1, tileCoords and WithY+1.
                    Vector3 result;
                    if( FindCollisionPoint( currentPosition, start, dir, lastTileCoords, tileCoords, &result ) )
                    {
                        if( pResult )
                            *pResult = result;

                        return true;
                    }
                }

                currentPosition += dir;
                lastTileCoords = tileCoords;
                tilePos += tileIncrement;
            }
        }
    }

    //Vector2Int startTileCoords;
    //Vector2Int endTileCoords;
    //bool startOnMap = GetTileCoordsAtWorldXZ( start.x, start.z, &startTileCoords, nullptr );
    //bool endOnMap = GetTileCoordsAtWorldXZ( end.x, end.z, &endTileCoords, nullptr );

    //Vector2 step( (float)endTileCoords.x - startTileCoords.x, (float)endTileCoords.y - startTileCoords.y );
    //step.Normalize();

    //int startTileIndex = startTileCoords.y * m_VertCount.x + startTileCoords.x;
    //int endTileIndex = endTileCoords.y * m_VertCount.x + endTileCoords.x;

    //Vector3 tempPos = Vector3( (float)startTileCoords.x, 0, (float)startTileCoords.y );
    ////Vector3 tempPos = Vector3( (float)endTileCoords.x, 0, (float)endTileCoords.y );
    //Vector3 currentPosition = Vector3( tempPos.x / m_VertCount.x * m_Size.x, 0,
    //                                   tempPos.z / m_VertCount.y * m_Size.y );

    //Vector3 currentPosition = start.WithY( 0 );
    ////Vector3 currentPosition = end.WithY( 0 );

    //if( pResult )
    //    *pResult = currentPosition;

    return false;
}
