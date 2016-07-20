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

#if MYFW_USING_WX
bool ComponentVoxelWorld::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentVoxelWorld ); //_VARIABLE_LIST

ComponentVoxelWorld::ComponentVoxelWorld()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pVoxelWorld = MyNew VoxelWorld;
    m_pVoxelWorld->Initialize( Vector3Int( 10, 10, 10 ) );

    m_pVoxelWorld->UpdateVisibility( this );

    m_pMaterial = 0;
}

ComponentVoxelWorld::~ComponentVoxelWorld()
{
    delete m_pVoxelWorld;

    SAFE_RELEASE( m_pMaterial );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentVoxelWorld::RegisterVariables(CPPListHead* pList, ComponentVoxelWorld* pThis) //_VARIABLE_LIST
{
    //AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged, (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
    AddVarPointer( pList, "Material",  true,  true, 0,
        (CVarFunc_GetPointerValue)&ComponentVoxelWorld::GetPointerValue,
        (CVarFunc_SetPointerValue)&ComponentVoxelWorld::SetPointerValue,
        (CVarFunc_GetPointerDesc)&ComponentVoxelWorld::GetPointerDesc,
        (CVarFunc_SetPointerDesc)&ComponentVoxelWorld::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
        (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
}

void ComponentVoxelWorld::Reset()
{
    ComponentBase::Reset();

    //m_SampleVector3.Set( 0, 0, 0 );
    SAFE_RELEASE( m_pMaterial );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentVoxelWorld::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentVoxelWorld>( "ComponentVoxelWorld" )
            //.addData( "m_SampleVector3", &ComponentVoxelWorld::m_SampleVector3 )
            .addFunction( "IsBlockEnabledAroundLocation", &ComponentVoxelWorld::IsBlockEnabledAroundLocation )
            .addFunction( "GetSceneYForNextBlockBelowPosition", &ComponentVoxelWorld::GetSceneYForNextBlockBelowPosition )
            .addFunction( "AddTileToTileInFocus", &ComponentVoxelWorld::AddTileToTileInFocus )            
            .addFunction( "DeleteTileInFocus", &ComponentVoxelWorld::DeleteTileInFocus )            
        .endClass();
}
#endif //MYFW_USING_LUA

void* ComponentVoxelWorld::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        return m_pMaterial;
    }

    return 0;
}

void ComponentVoxelWorld::SetPointerValue(ComponentVariable* pVar, void* newvalue)
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        return SetMaterial( (MaterialDefinition*)newvalue );
    }
}

const char* ComponentVoxelWorld::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
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

void ComponentVoxelWorld::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
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
void ComponentVoxelWorld::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentVoxelWorld::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "VoxelWorld", ObjectListIcon_Component );
}

void ComponentVoxelWorld::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentVoxelWorld::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "VoxelWorld", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentVoxelWorld::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
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
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pMaterial->m_pFile->m_FilenameWithoutExtension;
    }

    return oldvalue;
}

void* ComponentVoxelWorld::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    //if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    //{
    //    MyAssert( pVar->m_ControlID != -1 );
    //}
    if( strncmp( pVar->m_Label, "Material", strlen("Material") ) == 0 )
    {
        MyAssert( pVar->m_ControlID != -1 );

        wxString text = g_pPanelWatch->m_pVariables[pVar->m_ControlID].m_Handle_TextCtrl->GetValue();
        if( text == "" || text == "none" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

            oldpointer = GetMaterial();
            SetMaterial( 0 );
        }
    }

    return oldpointer;
}
#endif //MYFW_USING_WX

//cJSON* ComponentVoxelWorld::ExportAsJSONObject(bool savesceneid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );
//
//    ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST
//
//    return jComponent;
//}
//
//void ComponentVoxelWorld::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
//{
//    ComponentBase::ImportFromJSONObject( jComponent, sceneid );
//
//    ImportVariablesFromJSON( jComponent ); //_VARIABLE_LIST
//}

ComponentVoxelWorld& ComponentVoxelWorld::operator=(const ComponentVoxelWorld& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    //m_SampleVector3 = other.m_SampleVector3;
    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    return *this;
}

void ComponentVoxelWorld::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnSurfaceChanged );
#if _DEBUG
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, Draw );
#endif
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnFileRenamed );
    }
}

void ComponentVoxelWorld::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
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

void ComponentVoxelWorld::SetMaterial(MaterialDefinition* pMaterial)
{
    pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;

    if( m_pVoxelWorld == 0 )
        return;

    m_pVoxelWorld->SetMaterial( pMaterial );
}

void ComponentVoxelWorld::TickCallback(double TimePassed)
{
    if( m_pVoxelWorld == 0 )
        return;

    m_pVoxelWorld->Tick( TimePassed );
    m_pVoxelWorld->UpdateVisibility( this );

    GameObject* pPlayer = g_pComponentSystemManager->FindGameObjectByName( "Player" );
    Vector3 pos = pPlayer->GetTransform()->GetWorldPosition();

    m_pVoxelWorld->SetWorldCenter( pos );
}

#if _DEBUG
static Vector3 g_RayStart;
static Vector3 g_RayEnd;

void ComponentVoxelWorld::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
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

bool ComponentVoxelWorld::IsBlockEnabledAroundLocation(Vector3 scenepos, float radius)
{
    if( m_pVoxelWorld == 0 )
        return false;

    if( m_pVoxelWorld->IsBlockEnabledAroundLocation( scenepos, radius, true ) )
    {
        return true;
    }

    return false;
}

float ComponentVoxelWorld::GetSceneYForNextBlockBelowPosition(Vector3 scenepos, float radius)
{
    if( m_pVoxelWorld == 0 )
        return 0;

    return m_pVoxelWorld->GetSceneYForNextBlockBelowPosition( scenepos, radius );
}

void ComponentVoxelWorld::AddTileToTileInFocus(Vector2 mousepos)
{
    if( m_pVoxelWorld == 0 )
        return;

    Vector3 start, end;
    m_pVoxelWorld->GetMouseRayBadly( mousepos, &start, &end );

#if _DEBUG
    g_RayStart = start;
    g_RayEnd = end;
#endif

    VoxelRaycastResult result;
    if( m_pVoxelWorld->Raycast( start, end, 0.01f, &result ) )
    {
        LOGInfo( "VoxelWorld", "Ray hit (%d, %d, %d)\n", result.m_BlockWorldPosition.x, result.m_BlockWorldPosition.y, result.m_BlockWorldPosition.z );

        if( result.m_BlockFaceNormal.x == -1 ) result.m_BlockWorldPosition.x--;
        if( result.m_BlockFaceNormal.x ==  1 ) result.m_BlockWorldPosition.x++;
        if( result.m_BlockFaceNormal.y == -1 ) result.m_BlockWorldPosition.y--;
        if( result.m_BlockFaceNormal.y ==  1 ) result.m_BlockWorldPosition.y++;
        if( result.m_BlockFaceNormal.z == -1 ) result.m_BlockWorldPosition.z--;
        if( result.m_BlockFaceNormal.z ==  1 ) result.m_BlockWorldPosition.z++;

        m_pVoxelWorld->ChangeBlockState( result.m_BlockWorldPosition, true );
    }
}

void ComponentVoxelWorld::DeleteTileInFocus(Vector2 mousepos)
{
    if( m_pVoxelWorld == 0 )
        return;

    Vector3 start, end;
    m_pVoxelWorld->GetMouseRayBadly( mousepos, &start, &end );

    VoxelRaycastResult result;
    if( m_pVoxelWorld->Raycast( start, end, 0.01f, &result ) )
    {
        LOGInfo( "VoxelWorld", "Ray hit (%d, %d, %d)\n", result.m_BlockWorldPosition.x, result.m_BlockWorldPosition.y, result.m_BlockWorldPosition.z );

        m_pVoxelWorld->ChangeBlockState( result.m_BlockWorldPosition, false );
    }
}
