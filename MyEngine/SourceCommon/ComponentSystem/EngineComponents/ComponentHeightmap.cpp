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

    m_Heights = nullptr;

    m_pHeightmapFile = nullptr;
    m_WaitingForHeightmapFileToFinishLoading = false;

    m_pHeightmapTexture = nullptr;
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
    AddVar( pList, "HeightmapFile", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pHeightmapFile ), true, true, "File Heightmap", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, (CVarFunc_DropTarget)&ComponentHeightmap::OnDrop, nullptr );
    AddVar( pList, "HeightmapTexture", ComponentVariableType_TexturePtr, MyOffsetOf( pThis, &pThis->m_pHeightmapTexture ), true, true, "Texture", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, (CVarFunc_DropTarget)&ComponentHeightmap::OnDrop, nullptr );
}

void ComponentHeightmap::Reset()
{
    ComponentMesh::Reset();

    m_Size.Set( 10.0f, 10.0f );
    m_VertCount.Set( 128, 128 );

    UnregisterHeightmapFileLoadingCallbacks( true );
    SAFE_RELEASE( m_pHeightmapFile );
    m_HeightmapFileSize.Set( 0, 0 );

    UnregisterHeightmapTextureLoadingCallbacks( true );
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

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)pDropItem->m_Value;

        // Check if file is either a texture or a .myheightmap file and assign it if it is.
        TextureManager* pTextureManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetTextureManager();
        TextureDefinition* pTexture = pTextureManager->FindTexture( pFile );

        if( pTexture != nullptr )
        {
            oldPointer = m_pHeightmapTexture;
            SetHeightmapTexture( pTexture );

            // Rebuild the heightmap.
            CreateHeightmap();
        }
        else if( pFile == nullptr || strcmp( pFile->GetExtensionWithDot(), ".myheightmap" ) == 0 )
        {
            oldPointer = m_pHeightmapFile;
            SetHeightmapFile( pFile );

            // Rebuild the heightmap.
            CreateHeightmap();
        }
    }

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

    if( pVar->m_Offset == MyOffsetOf( this, &m_pHeightmapFile ) )
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
    g_pEngineCore->SetEditorInterface( EditorInterfaceType::HeightmapEditor );
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

    if( m_VertCount != m_HeightmapFileSize && m_HeightmapFileSize.x != 0 )
    {
        ImGui::Text( "TextureSize (%dx%d) doesn't match.", m_HeightmapFileSize.x, m_HeightmapFileSize.y );
        if( ImGui::Button( "Change vertCount" ) )
        {
            // TODO: Undo.
            m_VertCount = m_HeightmapFileSize;
            GenerateHeightmapMesh( true, true, true );
        }
    }

    if( m_VertCount != m_HeightmapTextureSize && m_HeightmapTextureSize.x != 0 )
    {
        ImGui::Text( "TextureSize (%dx%d) doesn't match.", m_HeightmapTextureSize.x, m_HeightmapTextureSize.y );
        if( ImGui::Button( "Change vertCount" ) )
        {
            // TODO: Undo.
            m_VertCount = m_HeightmapTextureSize;
            GenerateHeightmapMesh( true, true, true );
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
    SetHeightmapFile( other.m_pHeightmapFile );
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

void ComponentHeightmap::SetHeightmapFile(MyFileObject* pFile)
{
    UnregisterHeightmapFileLoadingCallbacks( true );
    m_HeightmapFileSize.Set( 0, 0 );
    SAFE_DELETE_ARRAY( m_Heights );

    if( pFile )
        pFile->AddRef();
    SAFE_RELEASE( m_pHeightmapFile );
    m_pHeightmapFile = pFile;
}

void ComponentHeightmap::UnregisterHeightmapFileLoadingCallbacks(bool force)
{
    if( m_WaitingForHeightmapFileToFinishLoading )
    {
        if( m_pHeightmapFile->IsFinishedLoading() || force )
        {
            m_pHeightmapFile->UnregisterFileFinishedLoadingCallback( this );
            m_WaitingForHeightmapFileToFinishLoading = false;
        }
    }
}

void ComponentHeightmap::OnFileFinishedLoadingHeightmapFile(MyFileObject* pFile) // StaticOnFileFinishedLoadingHeightmapFile
{
    CreateHeightmap();
    UnregisterHeightmapFileLoadingCallbacks( false );
}

void ComponentHeightmap::SetHeightmapTexture(TextureDefinition* pTexture)
{
    UnregisterHeightmapTextureLoadingCallbacks( true );
    m_HeightmapTextureSize.Set( 0, 0 );
    SAFE_DELETE_ARRAY( m_Heights );

    if( pTexture )
        pTexture->AddRef();
    SAFE_RELEASE( m_pHeightmapTexture );
    m_pHeightmapTexture = pTexture;
}

void ComponentHeightmap::UnregisterHeightmapTextureLoadingCallbacks(bool force)
{
    if( m_WaitingForTextureFileToFinishLoading )
    {
        if( m_pHeightmapFile->IsFinishedLoading() || force )
        {
            m_pHeightmapTexture->GetFile()->UnregisterFileFinishedLoadingCallback( this );
            m_WaitingForTextureFileToFinishLoading = false;
        }
    }
}

void ComponentHeightmap::OnFileFinishedLoadingHeightmapTexture(MyFileObject* pFile) // StaticOnFileFinishedLoadingHeightmapTexture
{
    CreateHeightmap();
    UnregisterHeightmapTextureLoadingCallbacks( false );
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
    bool createFromTexture = true;
    if( m_pHeightmapTexture == nullptr )
    {
//#if MYFW_EDITOR
//        if( true )
//        {
//            createFromFile = false;
//            MyAssert( m_Heights == nullptr );
//            LoadFromHeightmap( "Data/Meshes/TestHeightmap.myheightmap" );
//        }
//        else
//#endif
        if( m_pHeightmapFile )
        {
            createFromTexture = false;
            MyAssert( m_Heights == nullptr );
            LoadFromHeightmap();
        }
        else
        {
            createFromTexture = false;
            MyAssert( m_Heights == nullptr );
            m_Heights = MyNew float[m_VertCount.x * m_VertCount.y];
            memset( m_Heights, 0, sizeof(float) * m_VertCount.x * m_VertCount.y );
        }
    }

    if( GenerateHeightmapMesh( createFromTexture, true, true ) )
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
bool ComponentHeightmap::GenerateHeightmapMesh(bool createFromTexture, bool sizeChanged, bool rebuildNormals)
{
    if( createFromTexture && (m_pHeightmapTexture == nullptr || m_pHeightmapTexture->GetFile() == nullptr) )
    {
        MyAssert( false );
    }

    //LOGInfo( LOGTag, "ComponentHeightmap::GenerateHeightmapMesh\n" );

    // Sanity check on vertex count.
    if( m_VertCount.x <= 0 || m_VertCount.y <= 0 || (uint64)m_VertCount.x * (uint64)m_VertCount.y > UINT_MAX )
    {
        LOGError( LOGTag, "vertCount can't be negative.\n" );
        return false;
    }

    // If the heightmap texture is still loading, register a callback.
    if( createFromTexture == true )
    {
        if( m_pHeightmapTexture )
        {
            if( m_pHeightmapTexture->GetFile()->IsFinishedLoading() == false )
            {
                if( m_WaitingForTextureFileToFinishLoading == false )
                {
                    m_WaitingForTextureFileToFinishLoading = true;
                    m_pHeightmapTexture->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingHeightmapTexture );
                }
                return false;
            }
        }

        if( m_pHeightmapFile )
        {
            if( m_pHeightmapFile->IsFinishedLoading() == false )
            {
                if( m_WaitingForTextureFileToFinishLoading == false )
                {
                    m_WaitingForTextureFileToFinishLoading = true;
                    m_pHeightmapFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingHeightmapTexture );
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
    Vector2 uvRange( 0, 0 );
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

            for( int y = 0; y < vertCount.y; y++ )
            {
                for( int x = 0; x < vertCount.x; x++ )
                {
                    unsigned int index = (unsigned int)(y * vertCount.x + x);

                    Vector2 texCoord( (float)x/vertCount.x * texWidth, (float)y/vertCount.y * texHeight );
                    int texIndex = (int)( (int)texCoord.y * texWidth + (int)texCoord.x );
                    float height = pixelBuffer[texIndex*4] / 255.0f;

                    m_Heights[index] = height;
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
                pVerts[index].pos.y = bottomLeftPos.y + m_Heights[index];
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

    // Calculate normals.
    if( rebuildNormals )
    {
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

void ComponentHeightmap::SaveAsMyMesh(const char* filename)
{
    m_pMesh->ExportToFile( filename );
}

void ComponentHeightmap::SaveAsHeightmap(const char* filename)
{
    char outputFilename[260];
    size_t filenameLen = strlen( filename );
    if( filenameLen > 12 && strcmp( &filename[filenameLen-12], ".myheightmap" ) == 0 )
        sprintf_s( outputFilename, 260, "%s", filename );
    else
        sprintf_s( outputFilename, 260, "%s.myheightmap", filename );

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
    fwrite( m_Heights, sizeof(float), m_VertCount.x * m_VertCount.y, file );

    fclose( file );
}

#if MYFW_EDITOR
void ComponentHeightmap::LoadFromHeightmap(const char* filename)
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

    SAFE_DELETE_ARRAY( m_Heights );
    m_Heights = MyNew float[m_VertCount.x * m_VertCount.y];
    fread( m_Heights, 4, m_VertCount.x * m_VertCount.y, file );

    fclose( file );
}
#endif

void ComponentHeightmap::LoadFromHeightmap()
{
    MyAssert( m_pHeightmapFile && m_pHeightmapFile->IsFinishedLoading() );

    const char* pBuffer = m_pHeightmapFile->GetBuffer();

    unsigned int offset = 0;
    unsigned int versionCode = *(unsigned int*)&pBuffer[offset]; offset += sizeof(unsigned int);
    MyAssert( versionCode == 1 );

    m_VertCount.x = *(int*)&pBuffer[offset]; offset += sizeof(int);
    m_VertCount.y = *(int*)&pBuffer[offset]; offset += sizeof(int);

    SAFE_DELETE_ARRAY( m_Heights );
    m_Heights = MyNew float[m_VertCount.x * m_VertCount.y];
    memcpy( m_Heights, &pBuffer[offset], sizeof(float) * m_VertCount.x * m_VertCount.y );
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

bool ComponentHeightmap::FindCollisionPoint(const Vector3& currentPosition, const Vector3& start, const Vector3& dir, Vector2Int tile1, Vector2Int tile2, Vector3* pResult) const
{
    // Tile 1 and tile 2 are passed in.
    // There are 16 cases based on the general direction of the vector and whether it crosses a row/column.
    // There are 6 cases handled explicitly (2 more are duplicates) and the other 8 are the vectors in reverse.

    // -----2B----  d = dir vector.
    // |  / |  / |
    // | / dd /  |  1A = tile1.
    // 1Bdd-2A----  2A = tile2.
    // dd / |  / |
    // | /  | /  |  1B = 1 tile higher or to the right of tile1.
    // 1A---------  1Z = 1 tile lower or to the left of tile1.

    // Cast ray against the triangles between these 4 points.
    {
        Vector2 tileSize( m_Size.x / (m_VertCount.x-1), m_Size.y / (m_VertCount.y-1) );

        struct tri
        {
            Vector2Int tiles[3];
            int indices[3];
        };

        int numTris = 0;
        tri tris[4];

        // 4 cases (1 duplicate) for x>z with x direction being positive.
        //      positive z                negative z
        // 2 tris        2 tris      4 tris        2 tris
        //     2B                    1B--2C     (repeat case)
        //    / |                    |  / |
        //   / dd>                  >dd/  |
        // 1Bdd2A   or   1B--2B      1Add2B   or   1B--2B
        //>dd /          |  /dd>     |  /dd>      >ddd/ |
        // | /          >dd/  |      | /  |        | /ddd>
        // 1A            1A--2A      1Z--2A        1A--2A
        if( fabs(dir.x) > fabs(dir.z) )
        {
            // Code below relies on tile1 being to the left of tile2.
            if( dir.x < 0 )
            {
                MySwap( tile1, tile2 );
            }

            // Add all possible triangles to a list.
            // Triangles in even entries of list must be "upper left" triangles.
            // Triangles in odd entries of list must be "lower right" triangles.
            //  ------
            //  |UL/ |
            //  | /LR|
            //  ------
            if( tile1.y == tile2.y ) // Case 2 & 4.
            {
                numTris = 2;
                tris[0].tiles[0].Set( tile1.x, tile1.y   ); // 1A   //  2 tris
                tris[0].tiles[1].Set( tile1.x, tile1.y+1 ); // 1B   //
                tris[0].tiles[2].Set( tile2.x, tile2.y+1 ); // 2B   //  1B--2B
                                                                    // >ddd/ |
                tris[1].tiles[0].Set( tile1.x, tile1.y   ); // 1A   //  | /ddd>
                tris[1].tiles[1].Set( tile2.x, tile2.y+1 ); // 2B   //  1A--2A
                tris[1].tiles[2].Set( tile2.x, tile2.y   ); // 2A
            }
            else if( (dir.x > 0 && dir.z > 0) || (dir.x < 0 && dir.z < 0) ) // Case 1.
            {
                numTris = 2;                                        //  2 tris 
                tris[0].tiles[0].Set( tile1.x, tile1.y   ); // 1A   //      2B 
                tris[0].tiles[1].Set( tile1.x, tile1.y+1 ); // 1B   //     / | 
                tris[0].tiles[2].Set( tile2.x, tile2.y   ); // 2A   //    / dd>
                                                                    //  1Bdd2A 
                tris[1].tiles[0].Set( tile1.x, tile1.y+1 ); // 1B   // >dd /   
                tris[1].tiles[1].Set( tile2.x, tile2.y+1 ); // 2B   //  | /    
                tris[1].tiles[2].Set( tile2.x, tile2.y   ); // 2A   //  1A     
            }                                                      
            else //if( dir.z < 0 ) // Case 3.
            {
                numTris = 4;
                tris[0].tiles[0].Set( tile1.x, tile1.y   ); // 1A   //  4 tris
                tris[0].tiles[1].Set( tile1.x, tile1.y+1 ); // 1B   //  1B--2C
                tris[0].tiles[2].Set( tile2.x, tile2.y+2 ); // 2C   //  |  / |
                                                                    // >dd/  |
                tris[1].tiles[0].Set( tile1.x, tile1.y   ); // 1A   //  1Add2B
                tris[1].tiles[1].Set( tile2.x, tile2.y+2 ); // 2C   //  |  /dd>
                tris[1].tiles[2].Set( tile2.x, tile2.y+1 ); // 2B   //  | /  |
                                                                    //  1Z--2A
                tris[2].tiles[0].Set( tile1.x, tile1.y-1 ); // 1Z
                tris[2].tiles[1].Set( tile1.x, tile1.y   ); // 1A
                tris[2].tiles[2].Set( tile2.x, tile2.y+1 ); // 2B

                tris[3].tiles[0].Set( tile1.x, tile1.y-1 ); // 1Z
                tris[3].tiles[1].Set( tile2.x, tile2.y+1 ); // 2B
                tris[3].tiles[2].Set( tile2.x, tile2.y   ); // 2A
            }
        }
        // 4 cases (1 duplicate) for z>x with z direction being positive.
        //        positive x                    negative x
        //    2 tris        2 tris          4 tris        2 tris
        //                                             (repeat case)
        //       ^             ^            ^               ^
        //     2Ad--2B      2A-d2B       2A-d2B---2C      2Ad-2B
        //    / d  /        |  d |       |  /dd  / |      | d/ |
        //   / d| /         | d  |       | /  |d/  |      | /d |
        // 1A-d1B       or  1Ad-1B       1Z--1A-d-1B  or  1A-d1B
        //    ^               ^                 ^            ^
        else //if( fabs(dir.z) > fabs(dir.x) )
        {
            // Code below relies on tile1 being to the below of tile2.
            if( dir.z < 0 )
            {
                MySwap( tile1, tile2 );
            }

            // Add all possible triangles to a list.
            // Triangles in even entries of list must be "lower right" triangles.
            // Triangles in odd entries of list must be "upper left" triangles.
            //  ------
            //  |UL/ |
            //  | /LR|
            //  ------
            if( tile1.x == tile2.x ) // Case 2 & 4.
            {
                numTris = 2;                                        // 2 tris
                tris[0].tiles[0].Set( tile1.x  , tile1.y ); // 1A   //    ^
                tris[0].tiles[1].Set( tile2.x  , tile2.y ); // 2A   // 2A-d2B
                tris[0].tiles[2].Set( tile2.x+1, tile2.y ); // 2B   // |  d |
                                                                    // | d  |
                tris[1].tiles[0].Set( tile1.x  , tile1.y ); // 1A   // 1Ad-1B
                tris[1].tiles[1].Set( tile2.x+1, tile2.y ); // 2B   //   ^
                tris[1].tiles[2].Set( tile1.x+1, tile1.y ); // 1B
            }
            //else if( dir.x > 0 ) // Case 1.
            else if( (dir.z > 0 && dir.x > 0) || (dir.z < 0 && dir.x < 0) ) // Case 1.
            {
                numTris = 2;                                        //   2 tris
                tris[0].tiles[0].Set( tile1.x  , tile1.y ); // 1A   //       ^    
                tris[0].tiles[1].Set( tile2.x  , tile2.y ); // 2A   //     2Ad--2B
                tris[0].tiles[2].Set( tile1.x+1, tile1.y ); // 1B   //    / d  /  
                                                                    //   / d| /   
                tris[1].tiles[0].Set( tile1.x+1, tile1.y ); // 1B   // 1A-d1B     
                tris[1].tiles[1].Set( tile2.x  , tile2.y ); // 2A   //    ^       
                tris[1].tiles[2].Set( tile2.x+1, tile2.y ); // 2B
            }                                                      
            else //if( dir.x < 0 ) // Case 3.
            {
                numTris = 4;                                        //   4 tris           
                tris[0].tiles[0].Set( tile1.x  , tile1.y ); // 1A   //    ^       
                tris[0].tiles[1].Set( tile2.x+2, tile2.y ); // 2C   // 2A-d2B---2C
                tris[0].tiles[2].Set( tile1.x+1, tile1.y ); // 1B   // |  /dd  / |
                                                                    // | /  |d/  |
                tris[1].tiles[0].Set( tile1.x  , tile1.y ); // 1A   // 1Z--1A-d-1B
                tris[1].tiles[1].Set( tile2.x+1, tile2.y ); // 2B   //        ^   
                tris[1].tiles[2].Set( tile2.x+2, tile2.y ); // 2C

                tris[2].tiles[0].Set( tile1.x-1, tile1.y ); // 1Z
                tris[2].tiles[1].Set( tile2.x+1, tile2.y ); // 2B
                tris[2].tiles[2].Set( tile1.x  , tile1.y ); // 1A

                tris[3].tiles[0].Set( tile1.x-1, tile1.y ); // 1Z
                tris[3].tiles[1].Set( tile2.x  , tile2.y ); // 2A
                tris[3].tiles[2].Set( tile2.x+1, tile2.y ); // 2B
            }
        }

        for( int t=0; t<numTris; t++ )
        {
            tri* pTri;

            // If direction is going in the negative x direction, loop over the triangle list in reverse.
            if( dir.x > 0 )
                pTri = &tris[t];
            else
                pTri = &tris[numTris-t-1];

            // If all heights are below our position, then don't check triangles. Also checking bounds.
            bool checkTriangles = false;
            for( int i=0; i<3; i++ )
            {
                // If out of bounds, then skip this triangle.
                if( pTri->tiles[i].x < 0 || pTri->tiles[i].x >= m_VertCount.x || pTri->tiles[i].y < 0 || pTri->tiles[i].y >= m_VertCount.y )
                {
                    checkTriangles = false;
                    break;
                }

                pTri->indices[i] = pTri->tiles[i].y * m_VertCount.x + pTri->tiles[i].x;
                if( currentPosition.y < m_Heights[pTri->indices[i]] )
                    checkTriangles = true;
            }
            
            if( checkTriangles == false )
                continue; // Go next next triangle if there is one.

            // Intersect against the single triangle.
            // Assumes it's a right-angle triangle with matching width/height with diagonal going from lower-left to upper-right.
            Vector3 pos0( pTri->tiles[0].x * tileSize.x, m_Heights[pTri->indices[0]], pTri->tiles[0].y * tileSize.y );
            Vector3 pos1( pTri->tiles[1].x * tileSize.x, m_Heights[pTri->indices[1]], pTri->tiles[1].y * tileSize.y );
            Vector3 pos2( pTri->tiles[2].x * tileSize.x, m_Heights[pTri->indices[2]], pTri->tiles[2].y * tileSize.y );

            Plane plane;
            Vector3 result;

            Vector3 normal = (pos1 - pos0).Cross( pos2 - pos0 );
            plane.Set( normal, pos0 );
            if( plane.IntersectRay( currentPosition, dir, &result ) )
            {
                // Determine if the tri is at upper left of the quad (i.e. x's < z's )
                //                     or at lower right of the quad (i.e. x's > z's )
                bool lowerRightTriangle = fabs(dir.x) > fabs(dir.z) ?
                                            // x > z, if dir is +x, even entries are upper left, odds are lower right.
                                            dir.x > 0 ? (t%2 == 1) : (t%2 == 0) : 
                                            // z > x, if dir is +z, odd entries are upper left, even are lower right.
                                            dir.z > 0 ? (t%2 == 0) : (t%2 == 1);
                if( result.x >= pos0.x && result.x < pos2.x &&
                    result.z >= pos0.z && result.z < pos1.z &&
                    ( result.x - pTri->tiles[0].x * tileSize.x < result.z - pTri->tiles[0].y * tileSize.y ||
                      lowerRightTriangle ) )
                {
                    if( pResult )
                        *pResult = result;
                    return true;
                }
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
    int* loopVariable; // We either loop over the edges on the x-axis or the y-axis.
    int loopLimit; // Which tile to stop looping on, either -1 if moving left over the tiles, or vertCount if moving right.
    Vector2 tilePos = currentPosition.XZ() / m_Size * Vector2( m_VertCount.x-1.0f, m_VertCount.y-1.0f );
    Vector2 tileIncrement;
    Vector2Int lastTileCoords( -1, -1 );

    if( fabs(dir.x) > fabs(dir.z) )
    {
        loopVariable = &tileCoords.x;

        // Make vector 'tileSize' long on the x-axis.
        dir = dir / fabs(dir.x) * tileSize.x;
        tileIncrement.Set( dir.x >= 0 ? 1.0f : -1.0f, dir.z / fabs(dir.x) );

        // Set the vertex that the loop will stop on.
        // If the vector is partway into a tile, take a partial step to reach the edge of that tile.
        // If the vector is sitting on the starting edge of a tile, take a full step to the next tile.
        if( dir.x > 0 )
        {
            loopLimit = m_VertCount.x;

            float perc = 1 - fmod( tilePos.x, 1.0f );
            currentPosition += dir * perc;
            lastTileCoords = tileCoords;
            tilePos += tileIncrement * perc;
        }
        else //if( dir.x <= 0 )
        {
            loopLimit = -1;

            float perc = fmod( tilePos.x, 1.0f );
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
        dir = dir / fabs(dir.z) * tileSize.y;
        tileIncrement.Set( dir.x / fabs(dir.z), dir.z >= 0 ? 1.0f : -1.0f );

        // Set the vertex that the loop will stop on.
        // If the vector is partway into a tile, take a partial step to reach the edge of that tile.
        // If the vector is sitting on the starting edge of a tile, take a full step to the next tile.
        if( dir.z > 0 )
        {
            loopLimit = m_VertCount.y;

            float perc = 1 - fmod( tilePos.y, 1.0f );
            currentPosition += dir * perc;
            lastTileCoords = tileCoords;
            tilePos += tileIncrement * perc;
        }
        else //if( dir.z <= 0 )
        {
            loopLimit = -1;

            float perc = fmod( tilePos.y, 1.0f );
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

    return false;
}

// Editor tools.
bool ComponentHeightmap::Raise(Vector3 position, float amount, float radius, float softness, bool rebuild)
{
    bool meshChanged = false;

    Vector2 tileSize( m_Size.x / (m_VertCount.x-1), m_Size.y / (m_VertCount.y-1) );
    Vector2Int center = (m_VertCount-1) * (position.XZ()/m_Size) + Vector2( 0.5f, 0.5f );

    Vector2Int start( center.x - (int)(radius/tileSize.x), center.y - (int)(radius/tileSize.x) );
    Vector2Int end( center.x + (int)(radius/tileSize.x), center.y + (int)(radius/tileSize.x) );

    if( start.x < 0 ) start.x = 0;
    if( start.y < 0 ) start.y = 0;
    if( end.x >= m_VertCount.x ) end.x = m_VertCount.x-1;
    if( end.y >= m_VertCount.y ) end.y = m_VertCount.x-1;

    for( int y=start.y; y<=end.y; y++ )
    {
        float diffY = y*tileSize.y - position.z;

        for( int x=start.x; x<=end.x; x++ )
        {
            float diffX = x*tileSize.x - position.x;

            float diff2 = diffX*diffX + diffY*diffY;
            float radius2 = radius*radius;

            if( diff2 < radius2 )
            {
                float expDist = diff2 / radius2; // 0 at center, 1 at edge.

                float perc = min( 1.0f, 1 - expDist + softness );

                m_Heights[y * m_VertCount.x + x] += amount * perc;
                meshChanged = true;
            }
        }
    }

    if( rebuild && meshChanged )
    {
        GenerateHeightmapMesh( false, false, false );
    }

    return meshChanged;
}
