//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentVoxelWorld.h"
#include "VoxelBlock.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/Interfaces/EditorInterface_VoxelMeshEditor.h"
#endif

#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Octree.h"

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentVoxelWorld ); //_VARIABLE_LIST

ComponentVoxelWorld::ComponentVoxelWorld()
: ComponentRenderable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_BakeWorld = false;
    m_MaxWorldSize.Set( 0, 0, 0 );
    m_pSaveFile = 0;

    m_pVoxelWorld = MyNew VoxelWorld;
    m_pVoxelWorld->Initialize( Vector3Int( 7, 5, 7 ) );

    //m_pVoxelWorld->UpdateVisibility( this );

    m_pMaterial = 0;
}

ComponentVoxelWorld::~ComponentVoxelWorld()
{
    delete m_pVoxelWorld;

    SAFE_RELEASE( m_pMaterial );
    SAFE_RELEASE( m_pSaveFile );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentVoxelWorld::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentVoxelWorld* pThis) //_VARIABLE_LIST
{
    //AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged, (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
#if MYFW_USING_WX
    AddVarPointer( pList, "Material",  true,  true, 0,
        (CVarFunc_GetPointerValue)&ComponentVoxelWorld::GetPointerValue,
        (CVarFunc_SetPointerValue)&ComponentVoxelWorld::SetPointerValue,
        (CVarFunc_GetPointerDesc)&ComponentVoxelWorld::GetPointerDesc,
        (CVarFunc_SetPointerDesc)&ComponentVoxelWorld::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
        (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
#else
    AddVarPointer( pList, "Material",  true,  true, 0,
        (CVarFunc_GetPointerValue)&ComponentVoxelWorld::GetPointerValue,
        (CVarFunc_SetPointerValue)&ComponentVoxelWorld::SetPointerValue,
        (CVarFunc_GetPointerDesc)&ComponentVoxelWorld::GetPointerDesc,
        (CVarFunc_SetPointerDesc)&ComponentVoxelWorld::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
        0, 0 );
#endif

#if MYFW_USING_WX
    AddVar( pList, "Bake World", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_BakeWorld ),
            true, true, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
            (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
#else
    AddVar( pList, "Bake World", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_BakeWorld ),
            true, true, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
            0, 0 );
#endif

    // These are only displayed if "Bake World" is checked.
    {
#if MYFW_USING_WX
        AddVar( pList, "Max World Size", ComponentVariableType_Vector3Int, MyOffsetOf( pThis, &pThis->m_MaxWorldSize ),
                true, false, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
                (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
#else
        AddVar( pList, "Max World Size", ComponentVariableType_Vector3Int, MyOffsetOf( pThis, &pThis->m_MaxWorldSize ),
                true, false, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
                0, 0 );
#endif

#if MYFW_USING_WX
        AddVar( pList, "Save File", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pSaveFile ),
                false, false, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
                (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
#else
        AddVar( pList, "Save File", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pSaveFile ),
                false, false, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged,
                0, 0 );
#endif
    }
}

void ComponentVoxelWorld::Reset()
{
    ComponentRenderable::Reset();

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
            .addFunction( "IsBlockEnabledAroundLocation", &ComponentVoxelWorld::IsBlockEnabledAroundLocation ) // bool ComponentVoxelWorld::IsBlockEnabledAroundLocation(Vector3 scenepos, float radius)
            .addFunction( "GetSceneYForNextBlockBelowPosition", &ComponentVoxelWorld::GetSceneYForNextBlockBelowPosition ) // float ComponentVoxelWorld::GetSceneYForNextBlockBelowPosition(Vector3 scenepos, float radius)
            .addFunction( "AddTileToTileInFocus", &ComponentVoxelWorld::AddTileToTileInFocus ) // void ComponentVoxelWorld::AddTileToTileInFocus(Vector2 mousepos)           
            .addFunction( "DeleteTileInFocus", &ComponentVoxelWorld::DeleteTileInFocus ) // void ComponentVoxelWorld::DeleteTileInFocus(Vector2 mousepos)
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

void ComponentVoxelWorld::SetPointerValue(ComponentVariable* pVar, const void* newvalue)
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        return SetVoxelMeshMaterial( (MaterialDefinition*)newvalue );
    }
}

const char* ComponentVoxelWorld::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        const char* desc = 0;
        
        if( m_pMaterial )
            desc = m_pMaterial->GetMaterialDescription();
        
        if( desc == 0 )
            desc = "none";

        return desc;
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
            SetVoxelMeshMaterial( pMaterial );
            pMaterial->Release();
        }
    }
}

#if MYFW_EDITOR
#if MYFW_USING_WX
void ComponentVoxelWorld::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentVoxelWorld::StaticOnLeftClick, ComponentRenderable::StaticOnRightClick, gameobjectid, "VoxelWorld", ObjectListIcon_Component );
}

void ComponentVoxelWorld::OnLeftClick(unsigned int count, bool clear)
{
    ComponentRenderable::OnLeftClick( count, clear );
}

void ComponentVoxelWorld::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "VoxelWorld", this, ComponentRenderable::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        if( m_BakeWorld )
        {
            FindComponentVariableByLabel( &m_ComponentVariableList_ComponentVoxelWorld, "Max World Size" )->m_DisplayInWatch = true;
            FindComponentVariableByLabel( &m_ComponentVariableList_ComponentVoxelWorld, "Save File" )->m_DisplayInWatch = true;
        }
        else
        {
            FindComponentVariableByLabel( &m_ComponentVariableList_ComponentVoxelWorld, "Max World Size" )->m_DisplayInWatch = false;
            FindComponentVariableByLabel( &m_ComponentVariableList_ComponentVoxelWorld, "Save File" )->m_DisplayInWatch = false;
        }

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        if( m_BakeWorld )
        {
            if( m_pSaveFile == 0 )
                g_pPanelWatch->AddButton( "Create Save File", this, -1, ComponentVoxelWorld::StaticOnButtonCreateSaveFile );
            g_pPanelWatch->AddButton( "Edit Mesh", this, -1, ComponentVoxelWorld::StaticOnButtonEditMesh );
        }
    }
}
#endif //MYFW_USING_WX

void* ComponentVoxelWorld::OnDrop(ComponentVariable* pVar, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        //oldPointer = old component;
        //(ComponentRenderable*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        //oldPointer = old GameObject;
        //(GameObject*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;
        MyAssert( pMaterial );

        oldPointer = m_pMaterial;
        SetVoxelMeshMaterial( pMaterial );

#if MYFW_USING_WX
        // update the panel so new Material name shows up.
        const char* shortdesc = pMaterial->GetMaterialShortDescription();
        if( shortdesc )
            g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.GetControlID() )->m_Description = shortdesc;
#endif //MYFW_USING_WX
    }

    return oldPointer;
}

void* ComponentVoxelWorld::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( strncmp( pVar->m_Label, "Material", strlen("Material") ) == 0 )
    {
        if( changedbyinterface )
        {
#if MYFW_USING_WX
            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
            if( text == "" || text == "none" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

                oldpointer = GetVoxelMeshMaterial();
                // TODO: undo/redo
                SetVoxelMeshMaterial( 0 );
            }
#endif //MYFW_USING_WX
        }
        else if( pNewValue && pNewValue->GetMaterialPtr() != 0 )
        {
            MyAssert( false );
            // TODO: implement this block
        }
    }

    if( strncmp( pVar->m_Label, "Bake World", strlen("Bake World") ) == 0 )
    {
#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX
    }

    if( strcmp( pVar->m_Label, "Save File" ) == 0 )
    {
#if MYFW_USING_WX
        wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
        if( text == "" || text == "none" || text == "no file" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "no file" );
            oldpointer = m_pSaveFile;
            this->SetSaveFile( 0 );
        }
#endif //MYFW_USING_WX
    }

    return oldpointer;
}

void ComponentVoxelWorld::OnButtonCreateSaveFile(int buttonid)
{
    MyAssert( m_pSaveFile == 0 );

    // pop up a file selector dialog.
    {
        // generally offer to create scripts in Scripts folder.
#if MYFW_USING_WX
        wxString initialpath = "./Data/Meshes";

        wxFileDialog FileDialog( g_pEngineMainFrame, _("Create Voxel world file"), initialpath, "", "Voxel world files (*.myvoxelworld)|*.myvoxelworld", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
        if( FileDialog.ShowModal() != wxID_CANCEL )
        {
            wxString wxpath = FileDialog.GetPath();
            char fullpath[MAX_PATH];
            sprintf_s( fullpath, MAX_PATH, "%s", (const char*)wxpath );
            const char* relativepath = GetRelativePath( fullpath );

            if( g_pFileManager->DoesFileExist( relativepath ) == false )
            {
                // create the file
                // TODO: make the file not garbage.
                FILE* pFile = 0;
#if MYFW_WINDOWS
                fopen_s( &pFile, relativepath, "wb" );
#else
                pFile = fopen( relativepath, "wb" );
#endif
                fclose( pFile );
            }

            {
                MyFileObject* pFile = g_pComponentSystemManager->LoadDataFile( relativepath, m_pGameObject->GetSceneID(), 0, true );

                // update the panel so new filename shows up.
                int filecontrolid = FindVariablesControlIDByLabel( "File" );
                if( filecontrolid != -1 )
                    g_pPanelWatch->GetVariableProperties( filecontrolid )->m_Description = pFile->GetFullPath();

                pFile->AddRef();
                m_pSaveFile = pFile;

                g_pPanelWatch->SetNeedsRefresh();
            }
        }
#endif //MYFW_USING_WX
    }
}

void ComponentVoxelWorld::OnButtonEditMesh(int buttonid)
{
    g_pEngineCore->SetEditorInterface( EditorInterfaceType_VoxelMeshEditor );
    ((EditorInterface_VoxelMeshEditor*)g_pEngineCore->GetCurrentEditorInterface())->SetWorldToEdit( this );
}
#endif //MYFW_EDITOR

cJSON* ComponentVoxelWorld::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject( savesceneid, saveid );

    if( m_pSaveFile )
        cJSON_AddStringToObject( jComponent, "Save File", m_pSaveFile->GetFullPath() );

    return jComponent;
}

void ComponentVoxelWorld::ImportFromJSONObject(cJSON* jComponent, SceneID sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jComponent, sceneid );

    ImportVariablesFromJSON( jComponent ); //_VARIABLE_LIST

    cJSON* jSaveFile = cJSON_GetObjectItem( jComponent, "Save File" );
    if( jSaveFile )
    {
        MyFileObject* pFile = g_pEngineFileManager->RequestFile( jSaveFile->valuestring, GetSceneID() );
        MyAssert( pFile );
        if( pFile )
        {
            SetSaveFile( pFile );
            pFile->Release(); // free ref added by RequestFile

            m_pVoxelWorld->SetSaveFile( pFile );
        }
    }
}

ComponentVoxelWorld& ComponentVoxelWorld::operator=(const ComponentVoxelWorld& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    //m_SampleVector3 = other.m_SampleVector3;
    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    return *this;
}

void ComponentVoxelWorld::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnSurfaceChanged );
#if MYFW_EDITOR
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
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
#if MYFW_EDITOR
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
#endif
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentVoxelWorld::SetSaveFile(MyFileObject* pFile)
{
    if( pFile )
        pFile->AddRef();

    SAFE_RELEASE( m_pSaveFile );
    m_pSaveFile = pFile;
}

void ComponentVoxelWorld::SetVoxelMeshMaterial(MaterialDefinition* pMaterial)
{
    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;

    if( m_pVoxelWorld == 0 )
        return;

    m_pVoxelWorld->SetMaterial( pMaterial );
}

void ComponentVoxelWorld::TickCallback(float deltaTime)
{
    if( m_pVoxelWorld == 0 )
        return;

    m_pVoxelWorld->Tick( deltaTime, this );

    GameObject* pPlayer = g_pComponentSystemManager->FindGameObjectByName( "Player" );
    Vector3 pos = pPlayer->GetTransform()->GetWorldPosition();

    m_pVoxelWorld->SetWorldCenter( pos );
    SceneGraph_Base* pSceneGraph = g_pComponentSystemManager->GetSceneGraph();

    // Change the octree dimensions if the player moves too far from the previous center.
    pos.x = MyRoundToMultipleOf( pos.x, 16 );
    pos.y = MyRoundToMultipleOf( pos.y, 16 );
    pos.z = MyRoundToMultipleOf( pos.z, 16 );
    ((SceneGraph_Octree*)pSceneGraph)->Resize( pos.x-32, pos.y-32, pos.z-32, pos.x+32, pos.y+32, pos.z+32 );
}

#if MYFW_EDITOR
static Vector3 g_RayStart;
static Vector3 g_RayEnd;

void ComponentVoxelWorld::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    // Draw a debug ray on screen. // TODO: fix and generalize some debug drawing functions
    //MaterialDefinition* pMaterial = g_pEngineCore->m_pMaterial_Box2DDebugDraw;
    //ShaderGroup* pShaderGroup = pMaterial->GetShader();

    //Vector3 verts[2];
    //verts[0] = g_RayStart;
    //verts[1] = g_RayEnd;

    //Shader_Base* pShader = (Shader_Base*)pShaderGroup->GetShader( ShaderPass_Main, 0, 0 );
    //pShader->ActivateAndProgramShader( 0, 0, 0, pMatProj, pMatView, 0, pMaterial );
    //glVertexAttribPointer( pShader->m_aHandle_Position, 3, GL_FLOAT, false, 0, verts );
    //glLineWidth( 1 );
    //glDrawArrays( GL_LINES, 0, 2 );
    //pShader->DeactivateShader();
}
#endif

// Exposed to Lua, change elsewhere if function signature changes.
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

// Exposed to Lua, change elsewhere if function signature changes.
float ComponentVoxelWorld::GetSceneYForNextBlockBelowPosition(Vector3 scenepos, float radius)
{
    if( m_pVoxelWorld == 0 )
        return 0;

    return m_pVoxelWorld->GetSceneYForNextBlockBelowPosition( scenepos, radius );
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentVoxelWorld::AddTileToTileInFocus(Vector2 mousepos)
{
    if( m_pVoxelWorld == 0 )
        return;

    Vector3 start, end;
    m_pVoxelWorld->GetMouseRayBadly( mousepos, &start, &end );

#if MYFW_EDITOR
    g_RayStart = start;
    g_RayEnd = end;
#endif

    VoxelRayCastResult result;
    if( m_pVoxelWorld->RayCast( start, end, 0.01f, &result ) )
    {
        LOGInfo( "VoxelWorld", "Ray hit (%d, %d, %d)\n", result.m_BlockWorldPosition.x, result.m_BlockWorldPosition.y, result.m_BlockWorldPosition.z );

        if( result.m_BlockFaceNormal.x == -1 ) result.m_BlockWorldPosition.x--;
        if( result.m_BlockFaceNormal.x ==  1 ) result.m_BlockWorldPosition.x++;
        if( result.m_BlockFaceNormal.y == -1 ) result.m_BlockWorldPosition.y--;
        if( result.m_BlockFaceNormal.y ==  1 ) result.m_BlockWorldPosition.y++;
        if( result.m_BlockFaceNormal.z == -1 ) result.m_BlockWorldPosition.z--;
        if( result.m_BlockFaceNormal.z ==  1 ) result.m_BlockWorldPosition.z++;

        m_pVoxelWorld->ChangeBlockState( result.m_BlockWorldPosition, 1, true );
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentVoxelWorld::DeleteTileInFocus(Vector2 mousepos)
{
    if( m_pVoxelWorld == 0 )
        return;

    Vector3 start, end;
    m_pVoxelWorld->GetMouseRayBadly( mousepos, &start, &end );

    VoxelRayCastResult result;
    if( m_pVoxelWorld->RayCast( start, end, 0.01f, &result ) )
    {
        LOGInfo( "VoxelWorld", "Ray hit (%d, %d, %d)\n", result.m_BlockWorldPosition.x, result.m_BlockWorldPosition.y, result.m_BlockWorldPosition.z );

        m_pVoxelWorld->ChangeBlockState( result.m_BlockWorldPosition, 1, false );
    }
}

void ComponentVoxelWorld::AddToSceneGraph()
{
    if( m_pVoxelWorld == 0 )
        return;

    m_pVoxelWorld->AddToSceneGraph();
}

void ComponentVoxelWorld::RemoveFromSceneGraph()
{
    if( m_pVoxelWorld == 0 )
        return;

    m_pVoxelWorld->RemoveFromSceneGraph();
}

void ComponentVoxelWorld::PushChangesToSceneGraphObjects()
{
}
