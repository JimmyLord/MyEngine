//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentMeshOBJ::ComponentMeshOBJ()
: ComponentMesh()
{
    m_BaseType = BaseComponentType_Renderable;

    m_pOBJFile = 0;
}

ComponentMeshOBJ::~ComponentMeshOBJ()
{
    SAFE_RELEASE( m_pOBJFile );
}

void ComponentMeshOBJ::Reset()
{
    ComponentMesh::Reset();

    if( m_pMesh == 0 )
        m_pMesh = MyNew MyMesh();
}

#if MYFW_USING_WX
void ComponentMeshOBJ::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentMeshOBJ::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "MeshOBJ" );
}

void ComponentMeshOBJ::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    ComponentMesh::FillPropertiesWindow( clear );

    const char* desc = "no mesh";
    if( m_pOBJFile )
        desc = m_pOBJFile->m_FullPath;
    g_pPanelWatch->AddPointerWithDescription( "OBJ", 0, desc, this, ComponentMeshOBJ::StaticOnDropOBJ );
}

void ComponentMeshOBJ::OnDropOBJ()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );
        assert( m_pMesh );

        int len = strlen( pFile->m_FullPath );
        const char* filenameext = &pFile->m_FullPath[len-4];

        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            SetOBJFile( pFile );
            m_pMesh->m_NumIndicesToDraw = 0;
        }

        // update the panel so new OBJ name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pOBJFile->m_FullPath;
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMeshOBJ::ExportAsJSONObject()
{
    cJSON* component = ComponentMesh::ExportAsJSONObject();

    if( m_pOBJFile )
        cJSON_AddStringToObject( component, "OBJ", m_pOBJFile->m_FullPath );

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
            SetOBJFile( pFile );
        }
    }
}

ComponentMeshOBJ& ComponentMeshOBJ::operator=(const ComponentMeshOBJ& other)
{
    assert( &other != this );

    ComponentMesh::operator=( other );

    this->m_pMesh->SetShaderGroup( other.m_pMesh->GetShaderGroup() );
    SetOBJFile( other.m_pOBJFile );

    return *this;
}

void ComponentMeshOBJ::SetOBJFile(MyFileObject* objfile)
{
    if( objfile )
        objfile->AddRef();

    SAFE_RELEASE( m_pOBJFile );
    m_pOBJFile = objfile;
}

void ComponentMeshOBJ::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    if( m_pOBJFile == 0 || m_pOBJFile->m_FileReady == false )
        return;

    // TODO: find a better way to handle the creation of a mesh if we pass in an obj file that isn't ready.
    if( m_pMesh->m_NumIndicesToDraw == 0 )
    {
        m_pMesh->CreateFromOBJBuffer( m_pOBJFile->m_pBuffer );
    }

    ComponentMesh::Draw( pMatViewProj, pShaderOverride, drawcount );
}
