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

#if MYFW_USING_WX
bool ComponentVoxelMesh::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentVoxelMesh ); //_VARIABLE_LIST

ComponentVoxelMesh::ComponentVoxelMesh()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_MeshSize.Set( 4, 4, 4 );

    m_pVoxelChunk = MyNew VoxelChunk;
    m_pVoxelChunk->Initialize( 0, Vector3(0,0,0), m_MeshSize, Vector3Int(0,0,0), Vector3(0.2f,0.2f,0.2f) );
    VoxelBlock* pBlocks = m_pVoxelChunk->GetBlocks();
    for( int z=0; z<m_MeshSize.z; z++ )
    {
        for( int y=0; y<m_MeshSize.y; y++ )
        {
            for( int x=0; x<m_MeshSize.x; x++ )
            {
                VoxelBlock* pBlock = &pBlocks[z*m_MeshSize.y*m_MeshSize.x + y*m_MeshSize.x + x];

                pBlock->SetEnabled( true );
            }
        }
    }
    m_pVoxelChunk->RebuildMesh( 1 );
    m_pVoxelChunk->AddToSceneGraph( this, 0 );

    m_pMaterial = 0;
}

ComponentVoxelMesh::~ComponentVoxelMesh()
{
    delete m_pVoxelChunk;

    SAFE_RELEASE( m_pMaterial );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentVoxelMesh::RegisterVariables(CPPListHead* pList, ComponentVoxelMesh* pThis) //_VARIABLE_LIST
{
    AddVarPointer( pList, "Material",  true,  true, 0,
        (CVarFunc_GetPointerValue)&ComponentVoxelMesh::GetPointerValue,
        (CVarFunc_SetPointerValue)&ComponentVoxelMesh::SetPointerValue,
        (CVarFunc_GetPointerDesc)&ComponentVoxelMesh::GetPointerDesc,
        (CVarFunc_SetPointerDesc)&ComponentVoxelMesh::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentVoxelMesh::OnValueChanged,
        (CVarFunc_DropTarget)&ComponentVoxelMesh::OnDrop, 0 );

    AddVar( pList, "MaxSize", ComponentVariableType_Vector3Int, MyOffsetOf( pThis, &pThis->m_MeshSize ), true, true, 0, (CVarFunc_ValueChanged)&ComponentVoxelMesh::OnValueChanged, (CVarFunc_DropTarget)&ComponentVoxelMesh::OnDrop, 0 );
}

void ComponentVoxelMesh::Reset()
{
    ComponentBase::Reset();

    SAFE_RELEASE( m_pMaterial );

    m_MeshSize.Set( 16, 16, 16 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentVoxelMesh::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentVoxelMesh>( "ComponentVoxelMesh" )
            //.addData( "m_SampleVector3", &ComponentVoxelMesh::m_SampleVector3 )
            //.addFunction( "AddTileToTileInFocus", &ComponentVoxelMesh::AddTileToTileInFocus )            
            //.addFunction( "DeleteTileInFocus", &ComponentVoxelMesh::DeleteTileInFocus )            
        .endClass();
}
#endif //MYFW_USING_LUA

void* ComponentVoxelMesh::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        return m_pMaterial;
    }

    return 0;
}

void ComponentVoxelMesh::SetPointerValue(ComponentVariable* pVar, void* newvalue)
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        return SetMaterial( (MaterialDefinition*)newvalue );
    }
}

const char* ComponentVoxelMesh::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        if( m_pMaterial && m_pMaterial->m_pFile )
            return m_pMaterial->m_pFile->m_FullPath;
        else
            return "none";
    }

    return "fix me";
}

void ComponentVoxelMesh::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        MyAssert( newdesc );
        if( newdesc )
        {
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( newdesc );
            SetMaterial( pMaterial );
            pMaterial->Release();
        }
    }
}

#if MYFW_USING_WX
void ComponentVoxelMesh::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentVoxelMesh::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "VoxelChunk", ObjectListIcon_Component );
}

void ComponentVoxelMesh::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentVoxelMesh::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "VoxelChunk", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        g_pPanelWatch->AddButton( "Edit Mesh", this, ComponentVoxelMesh::StaticOnButtonEditMesh );
    }
}

void* ComponentVoxelMesh::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        (ComponentBase*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        (GameObject*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
        MyAssert( pMaterial );

        oldvalue = m_pMaterial;
        SetMaterial( pMaterial );

        // update the panel so new Material name shows up.
        if( pMaterial->m_pFile )
            g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.m_ID )->m_Description = pMaterial->m_pFile->m_FilenameWithoutExtension;
    }

    return oldvalue;
}

void* ComponentVoxelMesh::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_MeshSize ) )
    {
        MyAssert( pVar->m_ControlID != -1 );

        // TODO: resize the max voxel size.
    }

    if( strncmp( pVar->m_Label, "Material", strlen("Material") ) == 0 )
    {
        MyAssert( pVar->m_ControlID != -1 );

        wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->m_Handle_TextCtrl->GetValue();
        if( text == "" || text == "none" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

            oldpointer = GetMaterial();
            SetMaterial( 0 );
        }
    }

    return oldpointer;
}

void ComponentVoxelMesh::OnButtonEditMesh()
{
    g_pEngineCore->SetEditorInterface( EditorInterfaceType_VoxelMeshEditor );
    ((EditorInterface_VoxelMeshEditor*)g_pEngineCore->GetCurrentEditorInterface())->SetMeshToEdit( this );
}
#endif //MYFW_USING_WX

//cJSON* ComponentVoxelMesh::ExportAsJSONObject(bool savesceneid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );
//
//    ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST
//
//    return jComponent;
//}

void ComponentVoxelMesh::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jComponent, sceneid );

    ImportVariablesFromJSON( jComponent ); //_VARIABLE_LIST

    // update the scenegraph object transform when component is finished loading.
    MyMatrix* pTransform = m_pGameObject->m_pComponentTransform->GetWorldTransform();
    m_pVoxelChunk->OverrideSceneGraphObjectTransform( pTransform );
}

ComponentVoxelMesh& ComponentVoxelMesh::operator=(const ComponentVoxelMesh& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    m_MeshSize = other.m_MeshSize;

    return *this;
}

void ComponentVoxelMesh::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, OnSurfaceChanged );
#if _DEBUG
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, Draw );
#endif
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelMesh, OnFileRenamed );
    }
}

void ComponentVoxelMesh::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
#if _DEBUG
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
#endif
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentVoxelMesh::SetMaterial(MaterialDefinition* pMaterial)
{
    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;

    if( m_pVoxelChunk == 0 )
        return;

    m_pVoxelChunk->SetMaterial( pMaterial );
}

//void ComponentVoxelMesh::TickCallback(double TimePassed)
//{
//    if( m_pVoxelChunk == 0 )
//        return;
//
//    m_pVoxelChunk->Tick( TimePassed );
//    m_pVoxelChunk->UpdateVisibility( this );
//
//    GameObject* pPlayer = g_pComponentSystemManager->FindGameObjectByName( "Player" );
//    Vector3 pos = pPlayer->GetTransform()->GetChunkPosition();
//
//    m_pVoxelChunk->SetChunkCenter( pos );
//}

#if _DEBUG
static Vector3 g_RayStart;
static Vector3 g_RayEnd;

void ComponentVoxelMesh::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    // Draw a debug ray on screen. // TODO: fix and generalize some debug drawing functions
    //MaterialDefinition* pMaterial = g_pEngineCore->m_pMaterial_Box2DDebugDraw;
    //ShaderGroup* pShaderGroup = pMaterial->GetShader();

    //Vector3 verts[2];
    //verts[0] = g_RayStart;
    //verts[1] = g_RayEnd;

    //Shader_Base* pShader = (Shader_Base*)pShaderGroup->GetShader( ShaderPass_Main, 0, 0 );
    //pShader->ActivateAndProgramShader( 0, 0, 0, pMatViewProj, 0, pMaterial );
    //glVertexAttribPointer( pShader->m_aHandle_Position, 3, GL_FLOAT, false, 0, verts );
    //glLineWidth( 1 );
    //glDrawArrays( GL_LINES, 0, 2 );
    //pShader->DeactivateShader();
}
#endif

void ComponentVoxelMesh::AddTileToTileInFocus(Vector2 mousepos)
{
//    if( m_pVoxelChunk == 0 )
//        return;
//
//    Vector3 start, end;
//    m_pVoxelChunk->GetMouseRayBadly( mousepos, &start, &end );
//
//#if _DEBUG
//    g_RayStart = start;
//    g_RayEnd = end;
//#endif
//
//    VoxelRayCastResult result;
//    if( m_pVoxelChunk->RayCast( start, end, 0.01f, &result ) )
//    {
//        LOGInfo( "VoxelChunk", "Ray hit (%d, %d, %d)\n", result.m_BlockChunkPosition.x, result.m_BlockChunkPosition.y, result.m_BlockChunkPosition.z );
//
//        if( result.m_BlockFaceNormal.x == -1 ) result.m_BlockChunkPosition.x--;
//        if( result.m_BlockFaceNormal.x ==  1 ) result.m_BlockChunkPosition.x++;
//        if( result.m_BlockFaceNormal.y == -1 ) result.m_BlockChunkPosition.y--;
//        if( result.m_BlockFaceNormal.y ==  1 ) result.m_BlockChunkPosition.y++;
//        if( result.m_BlockFaceNormal.z == -1 ) result.m_BlockChunkPosition.z--;
//        if( result.m_BlockFaceNormal.z ==  1 ) result.m_BlockChunkPosition.z++;
//
//        m_pVoxelChunk->ChangeBlockState( result.m_BlockChunkPosition, true );
//    }
}

void ComponentVoxelMesh::DeleteTileInFocus(Vector2 mousepos)
{
    //if( m_pVoxelChunk == 0 )
    //    return;

    //Vector3 start, end;
    //m_pVoxelChunk->GetMouseRayBadly( mousepos, &start, &end );

    //VoxelRayCastResult result;
    //if( m_pVoxelChunk->RayCast( start, end, 0.01f, &result ) )
    //{
    //    LOGInfo( "VoxelChunk", "Ray hit (%d, %d, %d)\n", result.m_BlockChunkPosition.x, result.m_BlockChunkPosition.y, result.m_BlockChunkPosition.z );

    //    m_pVoxelChunk->ChangeBlockState( result.m_BlockChunkPosition, false );
    //}
}
