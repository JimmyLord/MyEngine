//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentMeshOBJ::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMeshOBJ ); //_VARIABLE_LIST

ComponentMeshOBJ::ComponentMeshOBJ()
: ComponentMesh()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshOBJ::~ComponentMeshOBJ()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentMeshOBJ::RegisterVariables(CPPListHead* pList, ComponentMeshOBJ* pThis) //_VARIABLE_LIST
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
            return m_pMesh->m_pSourceFile;
    }

    return 0;
}

void ComponentMeshOBJ::SetPointerValue(ComponentVariable* pVar, void* newvalue) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
    {
        MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( (MyFileObject*)newvalue );
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

        MyFileObject* pFile = m_pMesh->m_pSourceFile;
        if( pFile )
            return pFile->m_FullPath;
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
            MyFileObject* pFile = g_pFileManager->RequestFile( newdesc ); // adds a ref
            if( pFile )
            {
                MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile ); // doesn't add a ref
                if( pMesh == 0 )
                {
                    pMesh = MyNew MyMesh();
                    if( strcmp( pFile->m_ExtensionWithDot, ".obj" ) == 0 )
                    {
                        pMesh->CreateFromOBJFile( pFile );
                    }
                    if( strcmp( pFile->m_ExtensionWithDot, ".mymesh" ) == 0 )
                    {
                        pMesh->CreateFromMyMeshFile( pFile );
                    }
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
        //if( m_pMesh && m_pMesh->m_pSourceFile )
        //    desc = m_pMesh->m_pSourceFile->m_FullPath;
        //g_pPanelWatch->AddPointerWithDescription( "File", 0, desc, this, ComponentMeshOBJ::StaticOnDropOBJ );

        if( addcomponentvariables )
        {
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
        }
    }
}

void* ComponentMeshOBJ::OnValueChanged(ComponentVariable* pVar, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( finishedchanging )
    {
        if( strcmp( pVar->m_Label, "OBJ" ) == 0 )
        {
            MyAssert( pVar->m_ControlID != -1 );

            wxString text = g_pPanelWatch->m_pVariables[pVar->m_ControlID].m_Handle_TextCtrl->GetValue();
            if( text == "" || text == "none" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

                if( m_pMesh )
                    oldpointer = m_pMesh->m_pSourceFile;
                SetMesh( 0 );
            }
        }
    }

    return oldpointer;
}

void* ComponentMeshOBJ::OnDropOBJ(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldpointer = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );
        //MyAssert( m_pMesh );

        //size_t len = strlen( pFile->m_FullPath );
        const char* filenameext = pFile->m_ExtensionWithDot;

        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            if( m_pMesh )
                oldpointer = m_pMesh->m_pSourceFile;

            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }

        if( strcmp( filenameext, ".mymesh" ) == 0 )
        {
            if( m_pMesh )
                oldpointer = m_pMesh->m_pSourceFile;

            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }

        g_pPanelWatch->m_NeedsRefresh = true;
    }

    return oldpointer;
}
#endif //MYFW_USING_WX

cJSON* ComponentMeshOBJ::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentMesh::ExportAsJSONObject( savesceneid );

    //if( m_pMesh && m_pMesh->m_pSourceFile )
    //    cJSON_AddStringToObject( component, "OBJ", m_pMesh->m_pSourceFile->m_FullPath );

    return component;
}

void ComponentMeshOBJ::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentMesh::ImportFromJSONObject( jsonobj, sceneid );

    //cJSON* objstringobj = cJSON_GetObjectItem( jsonobj, "OBJ" );
    //if( objstringobj )
    //{
    //    MyFileObject* pFile = g_pFileManager->FindFileByName( objstringobj->valuestring );
    //    if( pFile )
    //    {
    //        MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
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
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnSurfaceChanged );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshOBJ, OnFileRenamed );
    }
}

void ComponentMeshOBJ::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentMeshOBJ::SetMesh(MyMesh* pMesh)
{
    if( pMesh )
        pMesh->AddRef();

    SAFE_RELEASE( m_pMesh );
    m_pMesh = pMesh;

    if( pMesh )
    {
        // parse the mesh file in case this is the first instance of it.
        if( pMesh->m_MeshReady == false )
            pMesh->ParseFile();

        // if we didn't already have a material set for any submesh, use the default material from the new mesh's submesh.
        // TODO: this doesn't happen elsewhere if the mesh isn't ready because the file was still loading.
        if( pMesh->m_MeshReady == true )
        {
            for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
            {
                if( m_MaterialList[i] == 0 ) // if we don't already have a material set:
                {
                    if( i < pMesh->m_SubmeshList.Count() ) // check if the new mesh has a known material and set it:
                    {
                        MyAssert( pMesh->m_SubmeshList[i] );
                        MaterialDefinition* pMaterial = pMesh->m_SubmeshList[i]->GetMaterial();
                        if( pMaterial )
                            SetMaterial( pMaterial, i );
                    }
                }
                // decided to leave it without clearing the material if the new mesh doesn't have as many submeshes.
                //else
                //{
                //    SetMaterial( 0, i ); // clear the material if the new mesh doesn't have as many submeshes.
                //}
            }
        }
    }
}

void ComponentMeshOBJ::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    ComponentMesh::DrawCallback( pCamera, pMatViewProj, pShaderOverride );
}
