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

ComponentMeshOBJ::ComponentMeshOBJ()
: ComponentMesh()
{
    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshOBJ::~ComponentMeshOBJ()
{
}

void ComponentMeshOBJ::Reset()
{
    ComponentMesh::Reset();

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentMeshOBJ::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentMeshOBJ::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "MeshOBJ" );
}

void ComponentMeshOBJ::OnLeftClick(bool clear)
{
    ComponentMesh::OnLeftClick( clear );
}

void ComponentMeshOBJ::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "MeshOBJ", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentMesh::FillPropertiesWindow( clear );

        const char* desc = "no mesh";
        if( m_pMesh && m_pMesh->m_pSourceFile )
            desc = m_pMesh->m_pSourceFile->m_FullPath;
        g_pPanelWatch->AddPointerWithDescription( "File", 0, desc, this, ComponentMeshOBJ::StaticOnDropOBJ );
    }
}

void ComponentMeshOBJ::OnDropOBJ()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );
        //assert( m_pMesh );

        int len = strlen( pFile->m_FullPath );
        const char* filenameext = pFile->m_ExtensionWithDot;

        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }

        if( strcmp( filenameext, ".mymesh" ) == 0 )
        {
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMeshOBJ::ExportAsJSONObject()
{
    cJSON* component = ComponentMesh::ExportAsJSONObject();

    if( m_pMesh && m_pMesh->m_pSourceFile )
        cJSON_AddStringToObject( component, "OBJ", m_pMesh->m_pSourceFile->m_FullPath );

    return component;
}

void ComponentMeshOBJ::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentMesh::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* objstringobj = cJSON_GetObjectItem( jsonobj, "OBJ" );
    if( objstringobj )
    {
        MyFileObject* pFile = g_pFileManager->FindFileByName( objstringobj->valuestring );
        if( pFile )
        {
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );
        }
    }
}

ComponentMeshOBJ& ComponentMeshOBJ::operator=(const ComponentMeshOBJ& other)
{
    assert( &other != this );

    ComponentMesh::operator=( other );

    return *this;
}

void ComponentMeshOBJ::SetMesh(MyMesh* pMesh)
{
    if( pMesh )
        pMesh->AddRef();

    SAFE_RELEASE( m_pMesh );
    m_pMesh = pMesh;
}

void ComponentMeshOBJ::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentMesh::Draw( pMatViewProj, pShaderOverride, drawcount );
}
