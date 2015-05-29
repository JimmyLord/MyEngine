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
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_pMesh = 0;
    m_MaterialList.AllocateObjects( MAX_SUBMESHES );
    for( int i=0; i<MAX_SUBMESHES; i++ )
        m_MaterialList.Add( 0 );

    m_GLPrimitiveType = GL_TRIANGLES;
    m_PointSize = 1;
}

ComponentMesh::~ComponentMesh()
{
    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<m_MaterialList.Count(); i++ )
        SAFE_RELEASE( m_MaterialList[i] );
    m_MaterialList.FreeAllInList();
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<m_MaterialList.Count(); i++ )
        SAFE_RELEASE( m_MaterialList[i] );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentMesh::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    MyAssert( gameobjectid.IsOk() );
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMesh::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Mesh" );
}

void ComponentMesh::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentMesh::FillPropertiesWindow(bool clear)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Mesh", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    //MyAssert( m_pMesh );

    if( m_PanelWatchBlockVisible )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        for( unsigned int i=0; i<m_pMesh->m_SubmeshList.Count(); i++ )
        {
            const char* desc = "no material";

            char label[20];
            sprintf_s( label, 20, "Material %d", i );

            if( m_MaterialList[i] != 0 )
                desc = m_MaterialList[i]->GetName();

            m_ControlID_Material[i] = g_pPanelWatch->AddPointerWithDescription( label, 0, desc, this, ComponentMesh::StaticOnDropMaterial );
        }

        g_pPanelWatch->AddEnum( "Primitive Type", &m_GLPrimitiveType, 7, OpenGLPrimitiveTypeStrings );
        g_pPanelWatch->AddInt( "Point Size", &m_PointSize, 1, 100 );
    }
}

void ComponentMesh::OnDropMaterial(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        int materialthatchanged = -1;
        for( int i=0; i<MAX_SUBMESHES; i++ )
        {
            if( controlid == m_ControlID_Material[i] )
                materialthatchanged = i;                
        }

        if( materialthatchanged != -1 )
        {
            MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
            MyAssert( pMaterial );

            SetMaterial( pMaterial, materialthatchanged );

            // update the panel so new Material name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pMaterial->GetName();
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMesh::ExportAsJSONObject()
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject();

    cJSON* jMaterialArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jComponent, "Materials", jMaterialArray );

    for( unsigned int i=0; i<m_MaterialList.Count(); i++ )
    {
        MyAssert( m_MaterialList[i] == 0 || m_MaterialList[i]->m_pFile ); // new materials should be saved as files before the state is saved.

        cJSON* jMaterial = 0;
        if( m_MaterialList[i] && m_MaterialList[i]->m_pFile )
            jMaterial = cJSON_CreateString( m_MaterialList[i]->m_pFile->m_FullPath );

        if( jMaterial )
            cJSON_AddItemToArray( jMaterialArray, jMaterial );
    }

    cJSON_AddNumberToObject( jComponent, "PrimitiveType", m_GLPrimitiveType );
    cJSON_AddNumberToObject( jComponent, "PointSize", m_PointSize );
    
    return jComponent;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jComponentMesh, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jComponentMesh, sceneid );

    // TODO: remove this "Material" block, it's for old scenes before I changed to multiple materials.
    cJSON* jMaterial = cJSON_GetObjectItem( jComponentMesh, "Material" );
    if( jMaterial )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
        if( pMaterial )
        {
            SetMaterial( pMaterial, 0 );
            pMaterial->Release();
        }
    }

    cJSON* jMaterialArray = cJSON_GetObjectItem( jComponentMesh, "Materials" );
    if( jMaterialArray )
    {
        int nummaterials = cJSON_GetArraySize( jMaterialArray );

        //for( int i=0; i<MAX_SUBMESHES; i++ ) { MyAssert( m_MaterialList[i] == 0 ); }

        for( int i=0; i<nummaterials; i++ )
        {
            cJSON* jMaterial = cJSON_GetArrayItem( jMaterialArray, i );
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
            if( pMaterial )
            {
                SetMaterial( pMaterial, i );
                pMaterial->Release();
            }
        }
    }

    cJSONExt_GetInt( jComponentMesh, "PrimitiveType", &m_GLPrimitiveType );
    cJSONExt_GetInt( jComponentMesh, "PointSize", &m_PointSize );
}

ComponentMesh& ComponentMesh::operator=(const ComponentMesh& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    if( other.m_pMesh )
        other.m_pMesh->AddRef();
    SAFE_RELEASE( m_pMesh );
    m_pMesh = other.m_pMesh;

    //const ComponentMesh* pOther = &other;
    MyAssert( other.m_MaterialList.Count() == m_MaterialList.Count() );
    for( unsigned int i=0; i<other.m_MaterialList.Count(); i++ )
    {
        SetMaterial( other.m_MaterialList[i], i );
    }

    m_GLPrimitiveType = other.m_GLPrimitiveType;
    m_PointSize = other.m_PointSize;

    return *this;
}

void ComponentMesh::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    ComponentRenderable::SetMaterial( pMaterial, submeshindex );

    MyAssert( submeshindex >= 0 && submeshindex < (int)m_MaterialList.Count() );

    pMaterial->AddRef();
    SAFE_RELEASE( m_MaterialList[submeshindex] );
    m_MaterialList[submeshindex] = pMaterial;
}

void ComponentMesh::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, drawcount );

    if( m_pMesh )
    {
        for( unsigned int i=0; i<m_pMesh->m_SubmeshList.Count(); i++ )
        {
            m_pMesh->SetMaterial( m_MaterialList[i], i );
            m_pMesh->m_SubmeshList[i]->m_PrimitiveType = m_GLPrimitiveType;
            m_pMesh->m_SubmeshList[i]->m_PointSize = m_PointSize;
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
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : 0;
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
