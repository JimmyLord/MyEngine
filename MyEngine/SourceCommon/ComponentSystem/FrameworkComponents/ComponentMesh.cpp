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
bool ComponentMesh::m_PanelWatchBlockVisible = true;
#endif

const char* OpenGLPrimitiveTypeStrings[7] =
{
    "Points",
    "Lines",
    "LineLoop",
    "LineStrip",
    "Triangles",
    "TriangleStrip",
    "TriangleFan",
};

ComponentMesh::ComponentMesh()
: ComponentRenderable()
{
    m_BaseType = BaseComponentType_Renderable;

    m_pMesh = 0;
    m_pMaterial = 0;

    m_GLPrimitiveType = GL_TRIANGLES;
    m_PointSize = 1;
}

ComponentMesh::~ComponentMesh()
{
    SAFE_RELEASE( m_pMesh );
    SAFE_RELEASE( m_pMaterial );
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    SAFE_RELEASE( m_pMesh );
    SAFE_RELEASE( m_pMaterial );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentMesh::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    assert( gameobjectid.IsOk() );
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMesh::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Mesh" );
}

void ComponentMesh::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentMesh::FillPropertiesWindow(bool clear)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Mesh", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    //assert( m_pMesh );

    if( m_PanelWatchBlockVisible )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        const char* desc = "no material";
        if( m_pMaterial )
            desc = m_pMaterial->GetName();
        g_pPanelWatch->AddPointerWithDescription( "Material", 0, desc, this, ComponentMesh::StaticOnDropMaterial );

        g_pPanelWatch->AddEnum( "Primitive Type", &m_GLPrimitiveType, 7, OpenGLPrimitiveTypeStrings );
        g_pPanelWatch->AddInt( "Point Size", &m_PointSize, 1, 100 );
    }
}

void ComponentMesh::OnDropMaterial(wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
        assert( pMaterial );
        //assert( m_pMesh );

        SetMaterial( pMaterial );

        // update the panel so new Material name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pMaterial->GetName();
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMesh::ExportAsJSONObject()
{
    cJSON* component = ComponentRenderable::ExportAsJSONObject();

    assert( m_pMaterial == 0 || m_pMaterial->m_pFile ); // new materials should be saved as files before the state is saved.
    if( m_pMaterial && m_pMaterial->m_pFile )
        cJSON_AddStringToObject( component, "Material", m_pMaterial->m_pFile->m_FullPath );

    cJSON_AddNumberToObject( component, "PrimitiveType", m_GLPrimitiveType );
    cJSON_AddNumberToObject( component, "PointSize", m_PointSize );
    
    return component;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* materialstringobj = cJSON_GetObjectItem( jsonobj, "Material" );
    if( materialstringobj )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->FindMaterialByFilename( materialstringobj->valuestring );
        if( pMaterial )
        {
            pMaterial->AddRef();
            SAFE_RELEASE( m_pMaterial );
            m_pMaterial = pMaterial;
        }
    }

    cJSONExt_GetInt( jsonobj, "PrimitiveType", &m_GLPrimitiveType );
    cJSONExt_GetInt( jsonobj, "PointSize", &m_PointSize );
}

ComponentMesh& ComponentMesh::operator=(const ComponentMesh& other)
{
    assert( &other != this );

    ComponentRenderable::operator=( other );

    m_pMesh = other.m_pMesh;
    if( m_pMesh )
        m_pMesh->AddRef();

    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    m_GLPrimitiveType = other.m_GLPrimitiveType;
    m_PointSize = other.m_PointSize;

    return *this;
}

void ComponentMesh::SetMaterial(MaterialDefinition* pMaterial)
{
    ComponentRenderable::SetMaterial( pMaterial );

    pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
}

void ComponentMesh::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, drawcount );

    if( m_pMesh )
    {
        if( m_pMesh->m_SubmeshList.Count() > 0 )
        {
            if( m_pMaterial )
                m_pMesh->SetMaterial( m_pMaterial );
            m_pMesh->m_SubmeshList[0]->m_PrimitiveType = m_GLPrimitiveType;
            m_pMesh->m_SubmeshList[0]->m_PointSize = m_PointSize;
        }

        m_pMesh->SetTransform( m_pComponentTransform->m_Transform );

        // Find nearest lights.
        MyLight* lights;
        int numlights = g_pLightManager->FindNearestLights( 4, m_pComponentTransform->m_Transform.GetTranslation(), &lights );

        // Find nearest shadow casting light.
        MyMatrix* pShadowVP = 0;
        TextureDefinition* pShadowTex = 0;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentCameraShadow* pShadowCam = dynamic_cast<ComponentCameraShadow*>( pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera ) );
                if( pShadowCam )
                {
                    pShadowVP = &pShadowCam->m_matViewProj;
#if 1
                    pShadowTex = pShadowCam->m_pDepthFBO->m_pDepthTexture;
#else
                    pShadowTex = pShadowCam->m_pDepthFBO->m_pColorTexture;
#endif
                }
            }
        }

        Vector3 campos;
#if MYFW_USING_WX
        if( g_pEngineCore->m_EditorMode )
        {
            campos = g_pEngineCore->m_pEditorState->GetEditorCamera()->m_pComponentTransform->GetPosition();
        }
        else
#endif
        {
            campos = g_pComponentSystemManager->GetFirstCamera()->m_pComponentTransform->GetPosition();
        }

        m_pMesh->Draw( pMatViewProj, &campos, lights, numlights, pShadowVP, pShadowTex, 0, pShaderOverride );
    }
}
