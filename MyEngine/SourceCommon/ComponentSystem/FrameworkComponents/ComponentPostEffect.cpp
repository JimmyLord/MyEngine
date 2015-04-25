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
    m_BaseType = BaseComponentType_Data;

    m_pFullScreenQuad = 0;
    m_pShaderGroup = 0;
}

ComponentPostEffect::~ComponentPostEffect()
{
    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pShaderGroup );
}

#if MYFW_USING_WX
void ComponentPostEffect::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    g_pPanelObjectList->AddObject( this, ComponentPostEffect::StaticOnLeftClick, ComponentData::StaticOnRightClick, gameobjectid, "Post Effect" );
}

void ComponentPostEffect::OnLeftClick(bool clear)
{
    ComponentData::OnLeftClick( clear );
}

void ComponentPostEffect::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Post Effect", this, ComponentData::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentData::FillPropertiesWindow( clear );

        const char* desc = "no shader";
        if( m_pShaderGroup && m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
            desc = m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
        g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, ComponentPostEffect::StaticOnDropShader );
    }
}

void ComponentPostEffect::OnDropShader()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        assert( pShaderGroup );

        SetShader( pShaderGroup );

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
    }
}

void ComponentPostEffect::OnValueChanged(int controlid, bool finishedchanging)
{
    //ComponentData::OnValueChanged( controlid, finishedchanging );
}
#endif //MYFW_USING_WX

cJSON* ComponentPostEffect::ExportAsJSONObject()
{
    cJSON* component = ComponentData::ExportAsJSONObject();

    if( m_pShaderGroup )
        cJSON_AddStringToObject( component, "Shader", m_pShaderGroup->GetName() );

    return component;
}

void ComponentPostEffect::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentData::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* shaderstringobj = cJSON_GetObjectItem( jsonobj, "Shader" );
    if( shaderstringobj )
    {
        ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
        if( pShaderGroup )
        {
            SetShader( pShaderGroup );
        }
    }
}

void ComponentPostEffect::Reset()
{
    ComponentData::Reset();

    // free old quad and shader if needed.
    SAFE_RELEASE( m_pFullScreenQuad );
    SAFE_RELEASE( m_pShaderGroup );

    m_pFullScreenQuad = MyNew MySprite();
    m_pFullScreenQuad->Create( 2, 2, 0, 1, 1, 0, Justify_Center, false );
    m_pShaderGroup = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

ComponentPostEffect& ComponentPostEffect::operator=(const ComponentPostEffect& other)
{
    assert( &other != this );

    ComponentData::operator=( other );

    m_pFullScreenQuad = other.m_pFullScreenQuad;
    if( m_pFullScreenQuad )
        m_pFullScreenQuad->AddRef();

    m_pShaderGroup = other.m_pShaderGroup;
    if( m_pShaderGroup )
        m_pShaderGroup->AddRef();

    return *this;
}

void ComponentPostEffect::SetShader(ShaderGroup* pShader)
{
    pShader->AddRef();
    SAFE_RELEASE( m_pShaderGroup );
    m_pShaderGroup = pShader;
}

void ComponentPostEffect::Render(FBODefinition* pFBO)
{
    assert( m_pFullScreenQuad );
    assert( m_pShaderGroup );

    if( m_pFullScreenQuad == 0 || m_pShaderGroup == 0 )
        return;

    m_pFullScreenQuad->SetShaderAndTexture( m_pShaderGroup, pFBO->m_pColorTexture );
    m_pFullScreenQuad->Create( 2, 2, 0, (float)pFBO->m_Width/pFBO->m_TextureWidth, (float)pFBO->m_Height/pFBO->m_TextureHeight, 0, Justify_Center, false );

    if( m_pFullScreenQuad->Setup( 0 ) )
    {
        Shader_Base* pShader = (Shader_Base*)m_pShaderGroup->GlobalPass();
        pShader->ProgramDepthmap( pFBO->m_pDepthTexture );

        m_pFullScreenQuad->DrawNoSetup();
        m_pFullScreenQuad->DeactivateShader();
    }
}