//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentPostEffect::m_PanelWatchBlockVisible = true;
#endif

ComponentPostEffect::ComponentPostEffect()
: ComponentData()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pFullScreenQuad = 0;
    m_pMaterial = 0;
}

ComponentPostEffect::~ComponentPostEffect()
{
    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pMaterial );
}

#if MYFW_USING_WX
void ComponentPostEffect::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    g_pPanelObjectList->AddObject( this, ComponentPostEffect::StaticOnLeftClick, ComponentData::StaticOnRightClick, gameobjectid, "Post Effect" );
}

void ComponentPostEffect::OnLeftClick(unsigned int count, bool clear)
{
    ComponentData::OnLeftClick( count, clear );
}

void ComponentPostEffect::FillPropertiesWindow(bool clear, bool addcomponentvariables)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Post Effect", this, ComponentData::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentData::FillPropertiesWindow( clear );

        const char* desc = "no material";
        if( m_pMaterial && m_pMaterial->m_pFile )
            desc = m_pMaterial->m_pFile->m_FilenameWithoutExtension;
        g_pPanelWatch->AddPointerWithDescription( "Material", 0, desc, this, ComponentPostEffect::StaticOnDropMaterial );
    }
}

void ComponentPostEffect::OnDropMaterial(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
        MyAssert( pMaterial );

        SetMaterial( pMaterial );

        // update the panel so new Material name shows up.
        if( pMaterial->m_pFile )
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pMaterial->m_pFile->m_FilenameWithoutExtension;
    }
}

void ComponentPostEffect::OnValueChanged(int controlid, bool finishedchanging)
{
    //ComponentData::OnValueChanged( controlid, finishedchanging );
}
#endif //MYFW_USING_WX

cJSON* ComponentPostEffect::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentData::ExportAsJSONObject( savesceneid );

    if( m_pMaterial )
        cJSON_AddStringToObject( component, "Material", m_pMaterial->m_pFile->m_FullPath );

    return component;
}

void ComponentPostEffect::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentData::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* materialstringobj = cJSON_GetObjectItem( jsonobj, "Material" );
    if( materialstringobj )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( materialstringobj->valuestring );
        if( pMaterial )
        {
            SetMaterial( pMaterial );
            pMaterial->Release();
        }
    }
}

void ComponentPostEffect::Reset()
{
    ComponentData::Reset();

    // free old quad and material if needed.
    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pMaterial );

    m_pFullScreenQuad = MyNew MySprite( false );
    m_pFullScreenQuad->Create( 2, 2, 0, 1, 1, 0, Justify_Center, false );
    m_pMaterial = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

ComponentPostEffect& ComponentPostEffect::operator=(const ComponentPostEffect& other)
{
    MyAssert( &other != this );

    ComponentData::operator=( other );

    m_pFullScreenQuad = other.m_pFullScreenQuad;
    if( m_pFullScreenQuad )
        m_pFullScreenQuad->AddRef();

    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    return *this;
}

void ComponentPostEffect::SetMaterial(MaterialDefinition* pMaterial)
{
    pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
}

void ComponentPostEffect::Render(FBODefinition* pFBO)
{
    MyAssert( m_pFullScreenQuad );
    MyAssert( m_pMaterial );

    if( m_pFullScreenQuad == 0 || m_pMaterial == 0 )
        return;

    m_pMaterial->SetTextureColor( pFBO->m_pColorTexture );

    m_pFullScreenQuad->SetMaterial( m_pMaterial );
    m_pFullScreenQuad->Create( 2, 2, 0, (float)pFBO->m_Width/pFBO->m_TextureWidth, (float)pFBO->m_Height/pFBO->m_TextureHeight, 0, Justify_Center, false );

    if( m_pFullScreenQuad->Setup( 0 ) )
    {
        Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
        pShader->ProgramDepthmap( pFBO->m_pDepthTexture );

        m_pFullScreenQuad->DrawNoSetup();
        m_pFullScreenQuad->DeactivateShader();
    }
}