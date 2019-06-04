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
    Vector3 topLeftPos( -m_Size.x/2, 0, m_Size.y/2 );
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

                pVerts[index].pos.x = topLeftPos.x + size.x / (vertCount.x - 1) * x;
                pVerts[index].pos.y = topLeftPos.y + height;
                pVerts[index].pos.z = topLeftPos.z - size.y / (vertCount.y - 1) * y;

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
            unsigned int indexTL = y > 0  && x > 0  ? (unsigned int)((y-1) * vertCount.x + x-1) : indexC;
            unsigned int indexTC = y > 0            ? (unsigned int)((y-1) * vertCount.x + x  ) : indexC;
            unsigned int indexTR = y > 0  && x < mx ? (unsigned int)((y-1) * vertCount.x + x+1) : indexC;
            unsigned int indexBL = y < my && x > 0  ? (unsigned int)((y+1) * vertCount.x + x-1) : indexC;
            unsigned int indexBC = y < my           ? (unsigned int)((y+1) * vertCount.x + x  ) : indexC;
            unsigned int indexBR = y < my && x < mx ? (unsigned int)((y+1) * vertCount.x + x+1) : indexC;

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

                pIndices[ elementIndex + 0 ] = vertexIndex + 0;
                pIndices[ elementIndex + 1 ] = vertexIndex + 1;
                pIndices[ elementIndex + 2 ] = vertexIndex + (unsigned int)vertCount.x;

                pIndices[ elementIndex + 3 ] = vertexIndex + 1;
                pIndices[ elementIndex + 4 ] = vertexIndex + (unsigned int)vertCount.x + 1;
                pIndices[ elementIndex + 5 ] = vertexIndex + (unsigned int)vertCount.x;
            }
        }
    }

    // Calculate the bounding box.
    Vector3 center( topLeftPos.x + size.x/2, topLeftPos.y, topLeftPos.z + size.y/ 2 );
    m_pMesh->GetBounds()->Set( center, Vector3(size.x/2, 0, size.y/2) );

    m_pMesh->SetReady();

    // TODO: Just a test. Remove me.
    GetHeightAtWorldXZ( 0, 0, nullptr );

    return true;
}

bool ComponentHeightmap::GetPixelIndexAtWorldXZ(const float x, const float z, Vector2Int* pLocalPixel) const
{
    ComponentTransform* pTransform = this->m_pGameObject->GetTransform();
    MyAssert( pTransform );

    MyMatrix* pWorldMat = pTransform->GetWorldTransform();

    Vector3 localPos = pWorldMat->GetInverse() * Vector3( x, 0, z );

    //LOGInfo( LOGTag, "LocalPos: %f, %f, %f", localPos.x, localPos.y, localPos.z );

    // Get the pixel index at localPos.
    Vector3 topLeftPos( -m_Size.x/2, 0, -m_Size.y/2 );

    Vector3 posZero = localPos - topLeftPos;
    Vector2Int pixelIndex = m_VertCount * (posZero.XZ()/m_Size);
    pixelIndex.y = m_VertCount.y - pixelIndex.y - 1;

    if( pLocalPixel )
        pLocalPixel->Set( pixelIndex.x, pixelIndex.y );

    if( pixelIndex.x < 0 || pixelIndex.x >= m_VertCount.x ||
        pixelIndex.y < 0 || pixelIndex.y >= m_VertCount.y )
    {
        //LOGInfo( LOGTag, "ComponentHeightmap::GetHeightAtWorldXZ: Out of bounds" );
        return false;
    }

    //LOGInfo( LOGTag, "ComponentHeightmap::GetLocalPixelAtWorldPos: (%d,%d)", posIndex.x, posIndex.y );

    return true;
}

bool ComponentHeightmap::GetHeightAtWorldXZ(const float x, const float z, float* pFloat) const
{
    float height = 0.0f;

    Vector2Int pixelIndex;
    if( GetPixelIndexAtWorldXZ( x, z, &pixelIndex ) )
    {
        unsigned int index = (unsigned int)(pixelIndex.y * m_VertCount.x + pixelIndex.x);

        if( pFloat )
            *pFloat = m_Heights[index];

        //LOGInfo( LOGTag, "ComponentHeightmap::GetHeightAtWorldXZ: (%d,%d) %f", pixelIndex.x, pixelIndex.y, m_Heights[index] );

        return true;
    }

    return false;
}

bool ComponentHeightmap::RayCast(const Vector3& start, const Vector3& end, Vector3* pResult) const
{
    bool didHit = false;

    Vector3 currentPosition = start;

    Vector2Int startPixelIndex;
    Vector2Int endPixelIndex;
    bool startOnMap = GetPixelIndexAtWorldXZ( start.x, start.z, &startPixelIndex );
    bool endOnMap = GetPixelIndexAtWorldXZ( end.x, end.z, &endPixelIndex );

    if( pResult )
        *pResult = currentPosition;

    return didHit;
}
