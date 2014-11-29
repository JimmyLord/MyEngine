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

ComponentMesh::ComponentMesh()
: ComponentRenderable()
{
    m_BaseType = BaseComponentType_Renderable;

    m_pMesh = 0;
    m_pOBJFile = 0;
}

ComponentMesh::ComponentMesh(GameObject* owner)
: ComponentRenderable( owner )
{
    m_BaseType = BaseComponentType_Renderable;

    m_pMesh = 0;
    m_pOBJFile = 0;
}

ComponentMesh::~ComponentMesh()
{
    SAFE_DELETE( m_pMesh );
}

#if MYFW_USING_WX
void ComponentMesh::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentMesh::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Mesh" );
}

void ComponentMesh::FillPropertiesWindow()
{
    g_pPanelWatch->ClearAllVariables();

    assert( m_pMesh );

    const char* desc = "no shader";
    if( m_pMesh->GetShaderGroup() )
        desc = m_pMesh->GetShaderGroup()->GetShader( ShaderPass_Main )->m_pFilename;
    g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, ComponentMesh::StaticOnDropShaderGroup );

    desc = "no mesh";
    if( m_pOBJFile )
        desc = m_pOBJFile->m_Filename;
    g_pPanelWatch->AddPointerWithDescription( "OBJ", 0, desc, this, ComponentMesh::StaticOnDropFile );
}

void ComponentMesh::OnDropShaderGroup()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        assert( pShaderGroup );
        assert( m_pMesh );

        m_pMesh->SetShaderGroup( pShaderGroup );
    }
}

void ComponentMesh::OnDropFile()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );
        assert( m_pMesh );

        int len = strlen( pFile->m_Filename );
        const char* filenameext = &pFile->m_Filename[len-4];
        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            m_pOBJFile = pFile;
            m_pMesh->m_NumIndicesToDraw = 0;
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMesh::ExportAsJSONObject()
{
    cJSON* component = ComponentBase::ExportAsJSONObject();

    if( m_pMesh->GetShaderGroup() )
        cJSON_AddStringToObject( component, "Shader", m_pMesh->GetShaderGroup()->GetName() );
    if( m_pOBJFile )
        cJSON_AddStringToObject( component, "OBJ", m_pOBJFile->m_Filename );

    return component;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jsonobj)
{
    cJSON* shaderstringobj = cJSON_GetObjectItem( jsonobj, "Shader" );
    if( shaderstringobj )
    {
        ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
        m_pMesh->SetShaderGroup( pShaderGroup );
    }

    cJSON* objstringobj = cJSON_GetObjectItem( jsonobj, "OBJ" );
    if( objstringobj )
    {
        m_pOBJFile = g_pFileManager->FindFileByName( objstringobj->valuestring );
        assert( m_pOBJFile );
    }
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    if( m_pMesh == 0 )
        m_pMesh = MyNew MyMesh();
}

void ComponentMesh::SetShader(ShaderGroup* pShader)
{
    ComponentRenderable::SetShader( pShader );

    m_pMesh->SetShaderGroup( pShader );
}

void ComponentMesh::Draw(MyMatrix* pMatViewProj)
{
    ComponentRenderable::Draw(pMatViewProj);

    // TODO: find a better way to handle the creation of a mesh if we pass in an obj file that isn't ready.
    if( m_pMesh->m_NumIndicesToDraw == 0 && m_pOBJFile && m_pOBJFile->m_FileReady )
    {
        m_pMesh->CreateFromOBJBuffer( m_pOBJFile->m_pBuffer );
    }

    m_pMesh->m_Position = this->m_pComponentTransform->m_Transform;
    m_pMesh->Draw( pMatViewProj, 0, 0, 0, 0, 0, 0 );
}
