//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentMeshOBJ.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMeshOBJ ); //_VARIABLE_LIST

ComponentMeshOBJ::ComponentMeshOBJ(ComponentSystemManager* pComponentSystemManager)
: ComponentMesh( pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshOBJ::~ComponentMeshOBJ()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentMeshOBJ::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentMeshOBJ* pThis) //_VARIABLE_LIST
{
    ComponentMesh::RegisterVariables( pList, pThis );

    AddVarPointer( pList, "OBJ", true, true, "File",
        (CVarFunc_GetPointerValue)&ComponentMeshOBJ::GetPointerValue, (CVarFunc_SetPointerValue)&ComponentMeshOBJ::SetPointerValue, (CVarFunc_GetPointerDesc)&ComponentMeshOBJ::GetPointerDesc, (CVarFunc_SetPointerDesc)&ComponentMeshOBJ::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentMeshOBJ::OnValueChanged, (CVarFunc_DropTarget)&ComponentMeshOBJ::OnDropOBJ, 0 );
}

void ComponentMeshOBJ::Reset()
{
    ComponentMesh::Reset();

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

void* ComponentMeshOBJ::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
    {
        if( m_pMesh )
            return m_pMesh->GetFile();
    }

    return 0;
}

void ComponentMeshOBJ::SetPointerValue(ComponentVariable* pVar, const void* newvalue) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
    {
        MeshManager* pMeshManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetMeshManager();
        MyMesh* pMesh = pMeshManager->FindMeshBySourceFile( (MyFileObject*)newvalue );
        SetMesh( pMesh );
    }
}

const char* ComponentMeshOBJ::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
    {
        //MyAssert( m_pMesh );
        if( m_pMesh == 0 )
            return "none";

        MyFileObject* pFile = m_pMesh->GetFile();
        if( pFile )
            return pFile->GetFullPath();
        else
            return "none";
    }

    return "fix me";
}

void ComponentMeshOBJ::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
    {
        MyAssert( newdesc );
        if( newdesc )
        {
            FileManager* pFileManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetFileManager();
            MyFileObject* pFile = pFileManager->RequestFile( newdesc ); // adds a ref
            if( pFile )
            {
                MeshManager* pMeshManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetMeshManager();
                MyMesh* pMesh = pMeshManager->FindMeshBySourceFile( pFile ); // doesn't add a ref
                if( pMesh == 0 )
                {
                    pMesh = MyNew MyMesh( m_pComponentSystemManager->GetEngineCore() );
                    pMesh->SetSourceFile( pFile );
                    SetMesh( pMesh );
                    pMesh->Release();
                }
                else
                {
                    SetMesh( pMesh );
                }
                pFile->Release(); // free ref from RequestFile
            }
        }
    }
}

#if MYFW_EDITOR
#if MYFW_USING_WX
void ComponentMeshOBJ::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMeshOBJ::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "MeshOBJ", ObjectListIcon_Component );
}

void ComponentMeshOBJ::OnLeftClick(unsigned int count, bool clear)
{
    ComponentMesh::OnLeftClick( count, clear );
}

void ComponentMeshOBJ::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "MeshOBJ", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentMesh::FillPropertiesWindow( clear );

        //const char* desc = "no mesh";
        //if( m_pMesh && m_pMesh->GetFile() )
        //    desc = m_pMesh->GetFile()->GetFullPath();
        //g_pPanelWatch->AddPointerWithDescription( "File", 0, desc, this, ComponentMeshOBJ::StaticOnDropOBJ );

        if( addcomponentvariables )
        {
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
        }
    }
}
#endif //MYFW_USING_WX

void* ComponentMeshOBJ::OnDropOBJ(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pNewFile = (MyFileObject*)pDropItem->m_Value;
        MyAssert( pNewFile );
        //MyAssert( m_pMesh );

        //size_t len = strlen( pFile->GetFullPath() );
        const char* filenameext = pNewFile->GetExtensionWithDot();

        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            if( m_pMesh )
                oldPointer = m_pMesh->GetFile();

            SetPointerValue( pVar, pNewFile );

            //// This EditorCommand will call ::SetPointerValue which is expecting a pointer to the new file.
            //g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ComponentVariableIndirectPointerChanged( this, pVar, pFile ) );
        }

        if( strcmp( filenameext, ".mymesh" ) == 0 )
        {
            if( m_pMesh )
                oldPointer = m_pMesh->GetFile();

            SetPointerValue( pVar, pNewFile );

            //// This EditorCommand will call ::SetPointerValue which is expecting a pointer to the new file.
            //g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ComponentVariableIndirectPointerChanged( this, pVar, pFile ) );
        }

#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX
    }

    return oldPointer;
}

void* ComponentMeshOBJ::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( finishedChanging )
    {
        if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
        {
            if( changedByInterface )
            {
#if MYFW_USING_WX
                wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
                if( text == "" || text == "none" )
                {
                    g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

                    if( m_pMesh )
                        oldpointer = m_pMesh->GetFile();

                    g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ComponentVariableIndirectPointerChanged( this, pVar, 0 ) );
                }
#endif //MYFW_USING_WX
            }
            else
            {
                oldpointer = m_pMesh ? m_pMesh->GetFile() : 0;

                MyFileObject* pFile = pNewValue ? (MyFileObject*)pNewValue->GetPointerIndirect() : 0;
                MeshManager* pMeshManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetMeshManager();

                MyMesh* pNewMesh = pFile ? pMeshManager->FindMeshBySourceFile( pFile ) : 0;
                SetMesh( pNewMesh );
            }
        }
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

cJSON* ComponentMeshOBJ::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* component = ComponentMesh::ExportAsJSONObject( savesceneid, saveid );

    //if( m_pMesh && m_pMesh->GetFile() )
    //    cJSON_AddStringToObject( component, "OBJ", m_pMesh->GetFile()->GetFullPath() );

    return component;
}

void ComponentMeshOBJ::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentMesh::ImportFromJSONObject( jsonobj, sceneid );

    //cJSON* objstringobj = cJSON_GetObjectItem( jsonobj, "OBJ" );
    //if( objstringobj )
    //{
    //    FileManager* pFileManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetFileManager();
    //    MyFileObject* pFile = pFileManager->FindFileByName( objstringobj->valuestring );
    //    if( pFile )
    //    {
    //        MeshManager* pMeshManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetMeshManager();
    //        MyMesh* pMesh = pMeshManager->FindMeshBySourceFile( pFile );
    //        SetMesh( pMesh );
    //    }
    //}
}

ComponentMeshOBJ& ComponentMeshOBJ::operator=(const ComponentMeshOBJ& other)
{
    MyAssert( &other != this );

    ComponentMesh::operator=( other );

    return *this;
}

void ComponentMeshOBJ::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnSurfaceChanged );
        MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentMeshOBJ, Draw ); //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnFileRenamed );
    }
}

void ComponentMeshOBJ::UnregisterCallbacks()
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

void ComponentMeshOBJ::SetMesh(MyMesh* pMesh)
{
    if( m_pMesh == pMesh )
        return;

    if( pMesh )
        pMesh->AddRef();

    if( m_pMesh )
        RemoveFromRenderGraph();

    SAFE_RELEASE( m_pMesh );
    m_pMesh = pMesh;

    //if( pMesh )
    //{
    //    // parse the mesh file in case this is the first instance of it.
    //    if( pMesh->IsReady() == false )
    //        pMesh->ParseFile();

    //    // if we didn't already have a material set for any submesh, use the default material from the new mesh's submesh.
    //    // TODO: this doesn't happen elsewhere if the mesh isn't ready because the file was still loading.
    //    if( pMesh->IsReady() == true )
    //    {
    //        for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    //        {
    //            if( m_pMaterials[i] == 0 ) // if we don't already have a material set:
    //            {
    //                if( i < pMesh->GetSubmeshListCount() ) // check if the new mesh has a known material and set it:
    //                {
    //                    MyAssert( pMesh->GetSubmesh( i ) );
    //                    MaterialDefinition* pMaterial = pMesh->GetSubmesh( i )->GetMaterial();
    //                    if( pMaterial )
    //                        SetMaterial( pMaterial, i );
    //                }
    //            }
    //            // decided to leave it without clearing the material if the new mesh doesn't have as many submeshes.
    //            //else
    //            //{
    //            //    SetMaterial( 0, i ); // clear the material if the new mesh doesn't have as many submeshes.
    //            //}
    //        }
    //    }

        if( IsEnabled() && m_pMesh != 0 )
        {
            MyAssert( m_pGameObject->IsEnabled() );
            AddToRenderGraph();
        }
    //}
}

void ComponentMeshOBJ::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    ComponentMesh::DrawCallback( pCamera, pMatProj, pMatView, pShaderOverride );
}
