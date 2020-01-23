//
// Copyright (c) 2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentTilemap.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"

#if MYFW_EDITOR
#include "../SourceEditor/Documents/EditorDocument_Tilemap.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4996 )
#include "../../Libraries/LodePNG/lodepng.h"
#pragma warning( pop )

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentTilemap ); //_VARIABLE_LIST

ComponentTilemap::ComponentTilemap(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentMesh( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_Tiles = nullptr;

    m_pTilemapFile = nullptr;
    m_WaitingForTilemapFileToFinishLoading = false;

    m_pTilemapTexture = nullptr;
    m_WaitingForTextureFileToFinishLoading = false;
}

ComponentTilemap::~ComponentTilemap()
{
    SAFE_RELEASE( m_pTilemapFile );
    SAFE_RELEASE( m_pTilemapTexture );

    SAFE_DELETE_ARRAY( m_Tiles );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Tick );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnSurfaceChanged );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Draw );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnTouch );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnButtons );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnKeys );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnFileRenamed );
}

void ComponentTilemap::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentTilemap* pThis) //_VARIABLE_LIST
{
    ComponentMesh::RegisterVariables( pList, pThis );

    AddVar( pList, "Size", ComponentVariableType::Vector2, MyOffsetOf( pThis, &pThis->m_Size ), true, true, "Size", (CVarFunc_ValueChanged)&ComponentTilemap::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "VertCount", ComponentVariableType::Vector2Int, MyOffsetOf( pThis, &pThis->m_VertCount ), true, true, "VertCount", (CVarFunc_ValueChanged)&ComponentTilemap::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "TilemapFile", ComponentVariableType::FilePtr, MyOffsetOf( pThis, &pThis->m_pTilemapFile ), true, true, "File Tilemap", (CVarFunc_ValueChanged)&ComponentTilemap::OnValueChanged, (CVarFunc_DropTarget)&ComponentTilemap::OnDrop, nullptr );
    AddVar( pList, "TilemapTexture", ComponentVariableType::TexturePtr, MyOffsetOf( pThis, &pThis->m_pTilemapTexture ), true, true, "Texture", (CVarFunc_ValueChanged)&ComponentTilemap::OnValueChanged, (CVarFunc_DropTarget)&ComponentTilemap::OnDrop, nullptr );
}

void ComponentTilemap::Reset()
{
    ComponentMesh::Reset();

    m_Size.Set( 10.0f, 10.0f );
    m_VertCount.Set( 32, 32 );

    UnregisterTilemapFileLoadingCallbacks( true );
    SAFE_RELEASE( m_pTilemapFile );
    m_TilemapFileSize.Set( 0, 0 );

    UnregisterTilemapTextureLoadingCallbacks( true );
    SAFE_RELEASE( m_pTilemapTexture );
    m_TilemapTextureSize.Set( 0, 0 );

    SAFE_DELETE_ARRAY( m_Tiles );
}

#if MYFW_USING_LUA
void ComponentTilemap::LuaRegister(lua_State* luaState)
{
    luabridge::getGlobalNamespace( luaState )
        .beginClass<ComponentTilemap>( "ComponentTilemap" )
            //.addData( "m_SampleVector3", &ComponentTilemap::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentTilemap::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
void* ComponentTilemap::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)pDropItem->m_Value;

        // Check if file is either a texture or a .mytilemap file and assign it if it is.
        TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();
        TextureDefinition* pTexture = pTextureManager->FindTexture( pFile );

        if( pTexture != nullptr )
        {
            oldPointer = m_pTilemapTexture;
            SetTilemapTexture( pTexture );

            // Rebuild the tilemap.
            CreateTilemap();
        }
        else if( pFile == nullptr || strcmp( pFile->GetExtensionWithDot(), ".mytilemap" ) == 0 )
        {
            oldPointer = m_pTilemapFile;
            SetTilemapFile( pFile );

            // Rebuild the tilemap.
            CreateTilemap();
        }
    }

    if( pDropItem->m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        oldPointer = m_pTilemapTexture;
        SetTilemapTexture( (TextureDefinition*)pDropItem->m_Value );

        // Rebuild the tilemap.
        CreateTilemap();
    }

    return oldPointer;
}

void* ComponentTilemap::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_Size ) )
    {
        CreateTilemap();
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_VertCount ) )
    {
        CreateTilemap();
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_pTilemapFile ) )
    {
        CreateTilemap();
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_pTilemapTexture ) )
    {
        CreateTilemap();
    }

    return oldPointer;
}

void ComponentTilemap::OnButtonEditTilemap()
{
    EngineCore* pEngineCore = m_pEngineCore;

    EditorDocument_Tilemap* pDocument = MyNew EditorDocument_Tilemap( pEngineCore, this );
    pEngineCore->GetEditorState()->OpenDocument( pDocument );
}
#endif //MYFW_EDITOR

//cJSON* ComponentTilemap::ExportAsJSONObject(bool saveSceneID, bool saveID)
//{
//    cJSON* jComponent = ComponentMesh::ExportAsJSONObject( saveSceneID, saveID );
//
//    return jComponent;
//}
//
//void ComponentTilemap::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
//{
//    ComponentMesh::ImportFromJSONObject( jComponent, sceneID );
//}

void ComponentTilemap::FinishImportingFromJSONObject(cJSON* jComponent)
{
}

ComponentTilemap& ComponentTilemap::operator=(const ComponentTilemap& other)
{
    MyAssert( &other != this );

    ComponentMesh::operator=( other );

    // TODO: Replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_Size = other.m_Size;
    m_VertCount = other.m_VertCount;
    SetTilemapFile( other.m_pTilemapFile );
    SetTilemapTexture( other.m_pTilemapTexture );

    return *this;
}

void ComponentTilemap::OnLoad()
{
    ComponentBase::OnLoad();

    CreateTilemap();
}

void ComponentTilemap::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, OnSurfaceChanged );
        MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentTilemap, Draw ); //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTilemap, OnFileRenamed );
    }
}

void ComponentTilemap::UnregisterCallbacks()
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

//void ComponentTilemap::TickCallback(float deltaTime)
//{
//}

void ComponentTilemap::SetTilemapFile(MyFileObject* pFile)
{
    // Don't allow a tilemap to have both a tilemap file and a texture.
    if( pFile != nullptr )
    {
        SetTilemapTexture( nullptr );
    }

    UnregisterTilemapFileLoadingCallbacks( true );
    m_TilemapFileSize.Set( 0, 0 );
    SAFE_DELETE_ARRAY( m_Tiles );

    if( pFile )
        pFile->AddRef();
    SAFE_RELEASE( m_pTilemapFile );
    m_pTilemapFile = pFile;
}

void ComponentTilemap::UnregisterTilemapFileLoadingCallbacks(bool force)
{
    if( m_WaitingForTilemapFileToFinishLoading )
    {
        if( m_pTilemapFile->IsFinishedLoading() || force )
        {
            m_pTilemapFile->UnregisterFileFinishedLoadingCallback( this );
            m_WaitingForTilemapFileToFinishLoading = false;
        }
    }
}

void ComponentTilemap::OnFileFinishedLoadingTilemapFile(MyFileObject* pFile) // StaticOnFileFinishedLoadingTilemapFile
{
    CreateTilemap();
    UnregisterTilemapFileLoadingCallbacks( false );
}

void ComponentTilemap::SetTilemapTexture(TextureDefinition* pTexture)
{
    UnregisterTilemapTextureLoadingCallbacks( true );
    m_TilemapTextureSize.Set( 0, 0 );
    SAFE_DELETE_ARRAY( m_Tiles );

    if( pTexture )
        pTexture->AddRef();
    SAFE_RELEASE( m_pTilemapTexture );
    m_pTilemapTexture = pTexture;
}

void ComponentTilemap::UnregisterTilemapTextureLoadingCallbacks(bool force)
{
    if( m_WaitingForTextureFileToFinishLoading )
    {
        if( m_pTilemapFile->IsFinishedLoading() || force )
        {
            m_pTilemapTexture->GetFile()->UnregisterFileFinishedLoadingCallback( this );
            m_WaitingForTextureFileToFinishLoading = false;
        }
    }
}

void ComponentTilemap::OnFileFinishedLoadingTilemapTexture(MyFileObject* pFile) // StaticOnFileFinishedLoadingTilemapTexture
{
    CreateTilemap();
    UnregisterTilemapTextureLoadingCallbacks( false );
}

void ComponentTilemap::CreateTilemap()
{
    if( m_pMesh == nullptr )
    {
        m_pMesh = MyNew MyMesh( m_pEngineCore );
    }
    else
    {
        RemoveFromRenderGraph();
    }

    if( m_pMesh->GetSubmeshListCount() > 0 )
        m_pMesh->GetSubmesh( 0 )->m_PrimitiveType = m_GLPrimitiveType;

    // Generate the actual tilemap.
    bool createFromTexture = true;
    if( m_pTilemapTexture == nullptr )
    {
        if( m_pTilemapFile && m_Tiles == nullptr )
        {
            createFromTexture = false;
            MyAssert( m_Tiles == nullptr );
            if( m_pTilemapFile->IsFinishedLoading() )
            {
                LoadFromTilemap();
            }
            else
            {
                m_WaitingForTilemapFileToFinishLoading = true;
                m_pTilemapFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingTilemapFile );
                return;
            }
        }
        else
        {
            createFromTexture = false;
            SAFE_DELETE_ARRAY( m_Tiles );
            m_Tiles = MyNew TileIndex[m_VertCount.x * m_VertCount.y];
            memset( m_Tiles, 0, sizeof(TileIndex) * m_VertCount.x * m_VertCount.y );
        }
    }

    if( GenerateTilemapMesh( createFromTexture, true, true ) )
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
bool ComponentTilemap::GenerateTilemapMesh(bool createFromTexture, bool sizeChanged, bool rebuildNormals)
{
    if( createFromTexture && (m_pTilemapTexture == nullptr || m_pTilemapTexture->GetFile() == nullptr) )
    {
        MyAssert( false );
    }

    //LOGInfo( LOGTag, "ComponentTilemap::GenerateTilemapMesh\n" );

    // Sanity check on vertex count.
    if( m_VertCount.x <= 0 || m_VertCount.y <= 0 || (uint64)m_VertCount.x * (uint64)m_VertCount.y > UINT_MAX )
    {
        LOGError( LOGTag, "vertCount can't be negative.\n" );
        return false;
    }

    // If the tilemap texture is still loading, register a callback.
    if( createFromTexture == true )
    {
        if( m_pTilemapTexture )
        {
            if( m_pTilemapTexture->GetFile()->IsFinishedLoading() == false )
            {
                if( m_WaitingForTextureFileToFinishLoading == false )
                {
                    m_WaitingForTextureFileToFinishLoading = true;
                    m_pTilemapTexture->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingTilemapTexture );
                }
                return false;
            }
        }

        if( m_pTilemapFile )
        {
            if( m_pTilemapFile->IsFinishedLoading() == false )
            {
                if( m_WaitingForTextureFileToFinishLoading == false )
                {
                    m_WaitingForTextureFileToFinishLoading = true;
                    m_pTilemapFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingTilemapTexture );
                }
                return false;
            }
        }
    }

    // Set the parameters. // TODO: Make some of these members.
    Vector2 size = m_Size;
    Vector2Int vertCount = m_VertCount;
    Vector3 bottomLeftPos( 0, 0, 0 ); // Collision methods will fail if this changes.
    Vector2 uvStart( 0, 0 );
    Vector2 uvRange( 1, 1 );
    bool createTriangles = (m_GLPrimitiveType == MyRE::PrimitiveType_Points) ? false : true;

    // Calculate the number of triangles, vertices and indices.
    // TODO: Rewrite this to use triangle strips.
    unsigned int numTris = (vertCount.x - 1) * (vertCount.y - 1) * 2;
    unsigned int numVerts = vertCount.x * vertCount.y;
    unsigned int numIndices = createTriangles ? numTris * 3 : numVerts; // 3 per triangle or 1 per point.

    // Reinitialize the submesh properties along with the vertex and index buffers.
    if( sizeChanged )
    {
        m_pMesh->RebuildShapeBuffers( numVerts, VertexFormat_XYZUVNorm, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U32, "MyMesh_Plane" );
    }
    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_pMesh->GetSubmesh( 0 )->m_pVertexBuffer->GetData( true );
    unsigned int* pIndices = (unsigned int*)m_pMesh->GetSubmesh( 0 )->m_pIndexBuffer->GetData( true );

    // Generate the vertex positions
    {
        if( createFromTexture )
        {
            unsigned char* pixelBuffer = nullptr;
            unsigned int texWidth, texHeight;

            // Decode the file into a raw buffer.
            {
                unsigned char* buffer = (unsigned char*)m_pTilemapTexture->GetFile()->GetBuffer();
                MyAssert( buffer != nullptr );

                int length = m_pTilemapTexture->GetFile()->GetFileLength();

                unsigned int error = lodepng_decode32( &pixelBuffer, &texWidth, &texHeight, buffer, length );
                MyAssert( error == 0 );
            }

            // Store the tilemap texture size, so we can show a warning if there's a mismatch.
            m_TilemapTextureSize.Set( (int)texWidth, (int)texHeight );
        
            // Allocate a buffer to store the vertex heights.
            SAFE_DELETE_ARRAY( m_Tiles );
            m_Tiles = MyNew TileIndex[vertCount.x * vertCount.y];

            for( int y = 0; y < vertCount.y; y++ )
            {
                for( int x = 0; x < vertCount.x; x++ )
                {
                    unsigned int index = (unsigned int)(y * vertCount.x + x);

                    Vector2 texCoord( (float)x/vertCount.x * texWidth, (float)y/vertCount.y * texHeight );
                    int texIndex = (int)( (int)texCoord.y * texWidth + (int)texCoord.x );
                    float height = pixelBuffer[texIndex*4] / 255.0f;

                    m_Tiles[index] = (TileIndex)height;
                }
            }

            // Free the memory allocated by lodepng_decode32.
            free( pixelBuffer );
        }

        // Set the vertices.
        for( int y = 0; y < vertCount.y; y++ )
        {
            for( int x = 0; x < vertCount.x; x++ )
            {
                unsigned int index = (unsigned int)(y * vertCount.x + x);

                pVerts[index].pos.x = bottomLeftPos.x + size.x / (vertCount.x - 1) * x;
                pVerts[index].pos.y = bottomLeftPos.y + m_Tiles[index];
                pVerts[index].pos.z = bottomLeftPos.z + size.y / (vertCount.y - 1) * y;

                pVerts[index].uv.x = uvStart.x + x * uvRange.x / (vertCount.x - 1);
                pVerts[index].uv.y = uvStart.y + y * uvRange.y / (vertCount.y - 1);

                if( createTriangles == false )
                {
                    pIndices[index] = index;
                }
            }
        }
    }

    if( rebuildNormals )
    {
        RecalculateNormals( pVerts );
    }

    if( sizeChanged )
    {
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
    }

    m_pMesh->SetReady();

    return true;
}

void ComponentTilemap::RecalculateNormals()
{
    BufferDefinition* pVertexBuffer = m_pMesh->GetSubmesh( 0 )->m_pVertexBuffer;
    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)pVertexBuffer->GetData( true );

    RecalculateNormals( pVerts );

    // Mark the vertex data dirty again after changing the data, since this gets called on a thread.
    pVertexBuffer->MarkDirty();
    m_pMesh->SetReady();
}

void ComponentTilemap::RecalculateNormals(Vertex_XYZUVNorm* pVerts)
{
    Vector2Int vertCount = m_VertCount;

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
}

void ComponentTilemap::SaveAsTilemap(const char* filename)
{
    char outputFilename[260];
    size_t filenameLen = strlen( filename );
    if( filenameLen > 12 && strcmp( &filename[filenameLen-12], ".mytilemap" ) == 0 )
        sprintf_s( outputFilename, 260, "%s", filename );
    else
        sprintf_s( outputFilename, 260, "%s.mytilemap", filename );

    FILE* file;
#if MYFW_WINDOWS
    fopen_s( &file, outputFilename, "wb" );
#else
    file = fopen( outputFilename, "wb" );
#endif

    unsigned int versionCode = 1;
    fwrite( &versionCode, sizeof(int), 1, file );
    fwrite( &m_VertCount.x, sizeof(int), 1, file );
    fwrite( &m_VertCount.y, sizeof(int), 1, file );
    fwrite( m_Tiles, sizeof(int), m_VertCount.x * m_VertCount.y, file );

    fclose( file );
}

#if MYFW_EDITOR
void ComponentTilemap::SaveAsMyMesh(const char* filename)
{
    m_pMesh->ExportToFile( filename );
}

void ComponentTilemap::AddAllVariablesToWatchPanel()
{
    if( m_WaitingForTilemapFileToFinishLoading )
    {
        ImGui::Text( "Tilemap file is still loading..." );
        return;
    }

    //if( g_pEngineCore->GetCurrentEditorInterfaceType() == EditorInterfaceType::TilemapEditor )
    //{
    //    EditorInterface_TilemapEditor* pTilemapEditor = (EditorInterface_TilemapEditor*)g_pEngineCore->GetCurrentEditorInterface();

    //    if( pTilemapEditor->GetTilemapBeingEdited() == this )
    //    {
    //        if( pTilemapEditor->IsBusy() )
    //        {
    //            ImGui::Text( "Tilemap is being edited." );
    //            ImGui::Text( "   Recalculating normals..." );
    //        }
    //        else
    //        {
    //            ImGui::Text( "Tilemap is being edited." );
    //        }

    //        return;
    //    }
    //}

    ComponentBase::AddAllVariablesToWatchPanel();

    if( m_VertCount != m_TilemapFileSize && m_TilemapFileSize.x != 0 )
    {
        ImGui::Text( "TextureSize (%dx%d) doesn't match.", m_TilemapFileSize.x, m_TilemapFileSize.y );
        if( ImGui::Button( "Change vertCount" ) )
        {
            // TODO: Undo.
            m_VertCount = m_TilemapFileSize;
            GenerateTilemapMesh( true, true, true );
        }
    }

    if( m_VertCount != m_TilemapTextureSize && m_TilemapTextureSize.x != 0 )
    {
        ImGui::Text( "TextureSize (%dx%d) doesn't match.", m_TilemapTextureSize.x, m_TilemapTextureSize.y );
        if( ImGui::Button( "Change vertCount" ) )
        {
            // TODO: Undo.
            m_VertCount = m_TilemapTextureSize;
            GenerateTilemapMesh( true, true, true );
        }
    }

    if( m_Tiles != nullptr )
    {
        if( ImGui::Button( "Edit Tilemap" ) )
        {
            OnButtonEditTilemap();
        }
    }
}

void ComponentTilemap::LoadFromTilemap(const char* filename)
{
    FILE* file;
#if MYFW_WINDOWS
    fopen_s( &file, filename, "rb" );
#else
    file = fopen( outputFilename, "rb" );
#endif

    int versionCode;
    fread( &versionCode, 4, 1, file );
    MyAssert( versionCode == 1 );

    fread( &m_VertCount.x, 4, 2, file );

    SAFE_DELETE_ARRAY( m_Tiles );
    m_Tiles = MyNew TileIndex[m_VertCount.x * m_VertCount.y];
    fread( m_Tiles, 4, m_VertCount.x * m_VertCount.y, file );

    fclose( file );
}
#endif

void ComponentTilemap::LoadFromTilemap()
{
    MyAssert( m_pTilemapFile && m_pTilemapFile->IsFinishedLoading() );

    const char* pBuffer = m_pTilemapFile->GetBuffer();

    unsigned int offset = 0;
    unsigned int versionCode = *(unsigned int*)&pBuffer[offset]; offset += sizeof(unsigned int);
    MyAssert( versionCode == 1 );

    m_VertCount.x = *(int*)&pBuffer[offset]; offset += sizeof(int);
    m_VertCount.y = *(int*)&pBuffer[offset]; offset += sizeof(int);

    SAFE_DELETE_ARRAY( m_Tiles );
    m_Tiles = MyNew TileIndex[m_VertCount.x * m_VertCount.y];
    memcpy( m_Tiles, &pBuffer[offset], sizeof(TileIndex) * m_VertCount.x * m_VertCount.y );
}

bool ComponentTilemap::GetTileCoordsAtWorldXZ(const float x, const float z, Vector2Int* pLocalTile, Vector2* pPercIntoTile) const
{
    Vector3 localPos = Vector3( x, 0, z );

    // If dealing with a gameobject, transform position into local space.
    if( this->m_pGameObject )
    {
        ComponentTransform* pTransform = this->m_pGameObject->GetTransform();
        MyAssert( pTransform );
        MyMatrix* pWorldMat = pTransform->GetWorldTransform();
        localPos = pWorldMat->GetInverse() * localPos;
    }

    // Get the tile coordinates.
    Vector2Int tileCoords = (m_VertCount-1) * (localPos.XZ()/m_Size);

    if( pLocalTile )
        pLocalTile->Set( tileCoords.x, tileCoords.y );

    if( tileCoords.x < 0 || tileCoords.x >= m_VertCount.x ||
        tileCoords.y < 0 || tileCoords.y >= m_VertCount.y )
    {
        //LOGInfo( LOGTag, "ComponentTilemap::GetHeightAtWorldXZ: Out of bounds" );
        return false;
    }

    if( pPercIntoTile )
    {
        Vector2 tileSize( m_Size.x / (m_VertCount.x-1), m_Size.y / (m_VertCount.y-1) );
        pPercIntoTile->x = (localPos.x - tileCoords.x * tileSize.x) / tileSize.x;
        pPercIntoTile->y = (localPos.z - tileCoords.y * tileSize.y) / tileSize.y;
    }

    //LOGInfo( LOGTag, "ComponentTilemap::GetTileCoordsAtWorldXZ: (%d,%d)", posIndex.x, posIndex.y );

    return true;
}

bool ComponentTilemap::RayCast(bool rayIsInWorldSpace, Vector3 start, Vector3 end, Vector3* pResult) const
{
    if( m_Tiles == nullptr )
        return false;

    // Move ray into terrain space.
    if( rayIsInWorldSpace )
    {
        ComponentTransform* pTransform = this->m_pGameObject->GetTransform();
        MyAssert( pTransform );
        MyMatrix* pWorldMat = pTransform->GetWorldTransform();
        start = pWorldMat->GetInverse() * start;
        end = pWorldMat->GetInverse() * end;
    }

    // Get the direction vector.
    Vector3 dir = (end - start).GetNormalized();

    Vector3 currentPosition = start;

    //// Snap start to the edge of the first tile it hits.
    //// If the vector doesn't collide with the tilemap at all, kick out.
    //if( SnapToBounds( start, dir, &currentPosition ) == false )
    //    return false;

    // Get the tile coords.
    Vector2Int tileCoords = (m_VertCount-1) * (currentPosition.XZ()/m_Size);
    MyAssert( tileCoords.x >= 0 && tileCoords.x < m_VertCount.x && tileCoords.y >= 0 && tileCoords.y < m_VertCount.y );
    int tileIndex = tileCoords.y * m_VertCount.x + tileCoords.x;

    // If our currentPosition tile is below the tilemap, kick out.
    if( currentPosition.y < m_Tiles[tileIndex] )
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
    // 1----------  1,2 = heights from tilemap to check against.
    int* loopVariable; // We either loop over the edges on the x-axis or the y-axis.
    int loopLimit; // Which tile to stop looping on, either -1 if moving left over the tiles, or vertCount if moving right.
    Vector2 tilePos = currentPosition.XZ() / m_Size * Vector2( m_VertCount.x-1.0f, m_VertCount.y-1.0f );
    Vector2 tileIncrement;
    Vector2Int lastTileCoords( -1, -1 );

    if( fabsf(dir.x) > fabsf(dir.z) )
    {
        loopVariable = &tileCoords.x;

        // Make vector 'tileSize' long on the x-axis.
        dir = dir / fabsf(dir.x) * tileSize.x;
        tileIncrement.Set( dir.x >= 0 ? 1.0f : -1.0f, dir.z / fabsf(dir.x) );

        // Set the vertex that the loop will stop on.
        // If the vector is partway into a tile, take a partial step to reach the edge of that tile.
        // If the vector is sitting on the starting edge of a tile, take a full step to the next tile.
        if( dir.x > 0 )
        {
            loopLimit = m_VertCount.x;

            float perc = 1 - fmodf( tilePos.x, 1.0f );
            currentPosition += dir * perc;
            lastTileCoords = tileCoords;
            tilePos += tileIncrement * perc;
        }
        else //if( dir.x <= 0 )
        {
            loopLimit = -1;

            float perc = fmodf( tilePos.x, 1.0f );
            if( currentPosition.x == m_Size.x )
            {
                perc = 1.0f;
                lastTileCoords.Set( tileCoords.x, tileCoords.y );
            }
            else
            {
                lastTileCoords.Set( tileCoords.x + 1, tileCoords.y );
            }
            currentPosition += dir * perc;
            tilePos += tileIncrement * perc;
        }
    }
    else //if( fabs(dir.x) > fabs(dir.z) )
    {
        loopVariable = &tileCoords.y;

        // Make dir vector 'tileSize' long on the z-axis.
        dir = dir / fabsf(dir.z) * tileSize.y;
        tileIncrement.Set( dir.x / fabsf(dir.z), dir.z >= 0 ? 1.0f : -1.0f );

        // Set the vertex that the loop will stop on.
        // If the vector is partway into a tile, take a partial step to reach the edge of that tile.
        // If the vector is sitting on the starting edge of a tile, take a full step to the next tile.
        if( dir.z > 0 )
        {
            loopLimit = m_VertCount.y;

            float perc = 1 - fmodf( tilePos.y, 1.0f );
            currentPosition += dir * perc;
            lastTileCoords = tileCoords;
            tilePos += tileIncrement * perc;
        }
        else //if( dir.z <= 0 )
        {
            loopLimit = -1;

            float perc = fmodf( tilePos.y, 1.0f );
            if( currentPosition.z == m_Size.y )
            {
                perc = 1.0f;
                lastTileCoords.Set( tileCoords.x, tileCoords.y );
            }
            else
            {
                lastTileCoords.Set( tileCoords.x, tileCoords.y + 1 );
            }

            currentPosition += dir * perc;
            tilePos += tileIncrement * perc;
        }
    }

    // Loop through tiles checking for intersection point.
    while( *loopVariable != loopLimit )
    {
        tileCoords.Set( (int)tilePos.x, (int)tilePos.y );

        if( tileCoords.x >= 0 && tileCoords.x < m_VertCount.x && tileCoords.y >= 0 && tileCoords.y < m_VertCount.y )
        {
            Vector3 result;
            //if( FindCollisionPoint( currentPosition, start, dir, lastTileCoords, tileCoords, &result ) )
            //{
            //    if( pResult )
            //        *pResult = result;

            //    return true;
            //}
        }

        currentPosition += dir;
        lastTileCoords = tileCoords;
        tilePos += tileIncrement;
    }

    return false;
}

bool ComponentTilemap::RayCastAtLocalHeight(bool rayIsInWorldSpace, Vector3 start, Vector3 end, float height, Vector3* pResultAtDesiredHeight, Vector3* pResultOnGround) const
{
    // Move ray into terrain space.
    if( rayIsInWorldSpace )
    {
        ComponentTransform* pTransform = this->m_pGameObject->GetTransform();
        MyAssert( pTransform );
        MyMatrix* pWorldMat = pTransform->GetWorldTransform();
        start = pWorldMat->GetInverse() * start;
        end = pWorldMat->GetInverse() * end;
    }

    // Get the direction vector.
    Vector3 dir = (end - start).GetNormalized();

    Plane plane;
    Vector3 result;

    plane.Set( Vector3( 0, 1, 0 ), Vector3( 0, height, 0 ) );
    if( plane.IntersectRay( start, dir, &result ) )
    {
        *pResultAtDesiredHeight = result;

        pResultOnGround->x = result.x;
        //GetHeightAtWorldXZ( result.x, result.z, &pResultOnGround->y );
        pResultOnGround->z = result.z;

        return true;
    }

    return false;
}

// Editor tools.
bool ComponentTilemap::Tool_Raise(Vector3 position, float amount, float radius, float softness, bool rebuild)
{
    bool meshChanged = false;
    return meshChanged;
}

bool ComponentTilemap::Tool_Level(Vector3 position, float desiredHeight, float radius, float softness, bool rebuild)
{
    bool meshChanged = false;
    return meshChanged;
}
