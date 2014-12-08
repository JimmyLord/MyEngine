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
}

ComponentMesh::ComponentMesh(GameObject* owner)
: ComponentRenderable( owner )
{
    m_BaseType = BaseComponentType_Renderable;

    m_pMesh = 0;
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

void ComponentMesh::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    assert( m_pMesh );

    const char* desc = "no shader";
    if( m_pMesh->GetShaderGroup() )
        desc = m_pMesh->GetShaderGroup()->GetShader( ShaderPass_Main )->m_pFilename;
    g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, ComponentMesh::StaticOnDrop );

    desc = "no texture";
    if( m_pMesh->m_pTexture )
        desc = m_pMesh->m_pTexture->m_Filename;
    g_pPanelWatch->AddPointerWithDescription( "Texture", 0, desc, this, ComponentMesh::StaticOnDrop );    
}

void ComponentMesh::OnDrop()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        assert( pShaderGroup );
        assert( m_pMesh );

        m_pMesh->SetShaderGroup( pShaderGroup );
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );
        assert( m_pMesh );

        int len = strlen( pFile->m_Filename );
        const char* filenameext = &pFile->m_Filename[len-4];

        if( strcmp( filenameext, ".png" ) == 0 )
        {
            m_pMesh->m_pTexture = g_pTextureManager->FindTexture( pFile->m_Filename );
        }
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        m_pMesh->m_pTexture = (TextureDefinition*)g_DragAndDropStruct.m_Value;
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMesh::ExportAsJSONObject()
{
    cJSON* component = ComponentRenderable::ExportAsJSONObject();

    if( m_pMesh->GetShaderGroup() )
        cJSON_AddStringToObject( component, "Shader", m_pMesh->GetShaderGroup()->GetName() );
    if( m_pMesh->m_pTexture )
        cJSON_AddStringToObject( component, "Texture", m_pMesh->m_pTexture->m_Filename );

    return component;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jsonobj)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj );

    cJSON* shaderstringobj = cJSON_GetObjectItem( jsonobj, "Shader" );
    if( shaderstringobj )
    {
        ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
        m_pMesh->SetShaderGroup( pShaderGroup );
    }

    cJSON* texturestringobj = cJSON_GetObjectItem( jsonobj, "Texture" );
    if( texturestringobj )
    {
        m_pMesh->m_pTexture = g_pTextureManager->FindTexture( texturestringobj->valuestring );
    }
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    if( m_pMesh == 0 )
        m_pMesh = MyNew MyMesh();
}

ComponentMesh& ComponentMesh::operator=(const ComponentMesh& other)
{
    assert( &other != this );

    ComponentRenderable::operator=( other );

    this->m_pMesh->SetShaderGroup( other.m_pMesh->GetShaderGroup() );

    return *this;
}

void ComponentMesh::SetShader(ShaderGroup* pShader)
{
    ComponentRenderable::SetShader( pShader );

    m_pMesh->SetShaderGroup( pShader );
}

void ComponentMesh::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, drawcount );

    // TODO: find a better way to handle the creation of a mesh.
    if( m_pMesh->m_NumIndicesToDraw == 0 )
    {
        return;
        //m_pMesh->CreateCylinder( 1, 40, 0.9, 1, 0, 1, 0, 1, 0, 1, 0, 1 );
        //assert( false );
    }

    m_pMesh->m_Position = this->m_pComponentTransform->m_Transform;
    m_pMesh->Draw( pMatViewProj, 0, 0, 0, 0, 0, 0, pShaderOverride );
}
