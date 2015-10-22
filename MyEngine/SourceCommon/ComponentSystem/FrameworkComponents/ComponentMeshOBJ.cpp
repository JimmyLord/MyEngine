//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
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

    //AddVariablePointer( pList, "Material", true, true, 0, ComponentMeshOBJ::StaticOnValueChanged, ComponentMeshOBJ::StaticOnDrop, 0, ComponentMeshOBJ::StaticGetPointerValue, ComponentMeshOBJ::StaticSetPointerValue, ComponentMeshOBJ::StaticGetPointerDesc, ComponentMeshOBJ::StaticSetPointerDesc );
    AddVariablePointer( pList, "OBJ", true, true, "File", 0, ComponentMeshOBJ::StaticOnDropOBJ, 0, ComponentMeshOBJ::StaticGetPointerValue, ComponentMeshOBJ::StaticSetPointerValue, ComponentMeshOBJ::StaticGetPointerDesc, ComponentMeshOBJ::StaticSetPointerDesc );
    //AddVariable( pList, "File", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), false, true, 0, ComponentMeshOBJ::StaticOnValueChangedCV, ComponentMeshOBJ::StaticOnDropCV, 0 );
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

void ComponentMeshOBJ::SetPointerValue(ComponentVariable* pVar, void* newvalue)
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
        MyAssert( m_pMesh );
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
            MyFileObject* pFile = g_pFileManager->FindFileByName( newdesc );
            if( pFile )
            {
                MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
                SetMesh( pMesh );
            }
        }
    }
}

#if MYFW_USING_WX
void ComponentMeshOBJ::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMeshOBJ::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "MeshOBJ" );
}

void ComponentMeshOBJ::OnLeftClick(unsigned int count, bool clear)
{
    ComponentMesh::OnLeftClick( count, clear );
}

void ComponentMeshOBJ::FillPropertiesWindow(bool clear, bool addcomponentvariables)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "MeshOBJ", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
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

void* ComponentMeshOBJ::OnDropOBJ(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );
        //MyAssert( m_pMesh );

        //size_t len = strlen( pFile->m_FullPath );
        const char* filenameext = pFile->m_ExtensionWithDot;

        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            oldvalue = m_pMesh->m_pSourceFile;

            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }

        if( strcmp( filenameext, ".mymesh" ) == 0 )
        {
            oldvalue = m_pMesh->m_pSourceFile;

            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }

        g_pPanelWatch->m_NeedsRefresh = true;
    }

    return oldvalue;
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

void ComponentMeshOBJ::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentMesh::Draw( pMatViewProj, pShaderOverride, drawcount );
}
